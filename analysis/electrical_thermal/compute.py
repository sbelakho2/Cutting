#!/usr/bin/env python3
"""M34 regeneration/energy, M21 bleed, M20 ventilation balance (desk closure).

Model-map correction (review F12): branch absorption/energy is M34; M33 is
fault/protection coordination and is covered by the P02/P11 procedures, not by
this script.

`python3 analysis/electrical_thermal/compute.py` -> manifest.json + report.md.
Formulas and counterexamples per ENGINEERING_HANDOFF 3.7; every component
identity is a candidate until its physical record exists. Nothing here is a
completed electrical/thermal qualification.
"""
from __future__ import annotations

import hashlib
import json
import math
import pathlib

HERE = pathlib.Path(__file__).resolve().parent


def branch_absorption(v_i: float, energy_j: float, c_farad: float) -> float:
    return math.sqrt(v_i * v_i + 2.0 * energy_j / c_farad)


# M34: counterexample from the handoff - a 36 V branch with 100 uF and a
# 0.1 J regeneration increment lands at 57.411 V: NOT a safe branch.
cases = []
for name, v_i, e_j, c_f in [
    ("stored_counterexample_36V_100uF_0.1J", 36.0, 0.1, 100e-6),
    ("candidate_36V_2200uF_0.1J", 36.0, 0.1, 2200e-6),
    ("candidate_60V_2200uF_0.5J", 60.0, 0.5, 2200e-6),
    ("accepted_48V_2200uF_0.2J_V20_branch", 48.0, 0.2, 2200e-6),
    ("accepted_48V_4700uF_0.5J_V20_branch", 48.0, 0.5, 4700e-6),
    ("candidate_60V_4700uF_0.5J", 60.0, 0.5, 4700e-6),
]:
    v_f = branch_absorption(v_i, e_j, c_f)
    cases.append({
        "case": name,
        "V_initial": v_i,
        "E_increment_J": e_j,
        "C_F": c_f,
        "V_final": v_f,
        "safe_within_10pct_of_nominal": v_f <= v_i * 1.10,
        "inside_V20_driver_limit_50V": (v_f <= 50.0) if abs(v_i - 48.0) < 1e-9 else None,
        "note": "safety of a DC branch is judged by the protected rating and the "
                "component voltage classes, not by this bound alone (G33)",
    })

# M21: RC bleed envelope. Corrected arithmetic (review F12): resistor power
# falls as P(t)/P0 = exp(-2t/RC). The historical "below 25 % of initial power in
# 2.6 s" claim belongs to a different (25 W-envelope) configuration and is NOT
# reproduced by these examples - at 2.6 s they retain ~88-94 % of initial power,
# and significant voltage remains well past 60 s. Dissipation is not a discharge
# guarantee: verified resistor/relay per branch and measured bus decay are
# required (G33/G36).
def rc_voltage(v0: float, r_ohm: float, c_f: float, t_s: float) -> float:
    return v0 * math.exp(-t_s / (r_ohm * c_f))


def rc_power_fraction(r_ohm: float, c_f: float, t_s: float) -> float:
    return math.exp(-2.0 * t_s / (r_ohm * c_f))


bleed = {
    "candidate": "18 kohm 25 W class bleed envelope (historical audit; identity unverified)",
    "examples": [
        {"branch": "36V_2200uF", "V_after_2.6s": rc_voltage(36.0, 18000.0, 2200e-6, 2.6),
         "V_after_60s": rc_voltage(36.0, 18000.0, 2200e-6, 60.0),
         "power_fraction_after_2.6s": rc_power_fraction(18000.0, 2200e-6, 2.6)},
        {"branch": "60V_4700uF", "V_after_2.6s": rc_voltage(60.0, 18000.0, 4700e-6, 2.6),
         "V_after_60s": rc_voltage(60.0, 18000.0, 4700e-6, 60.0),
         "power_fraction_after_2.6s": rc_power_fraction(18000.0, 4700e-6, 2.6)},
    ],
    "decision": "power taper is NOT a discharge guarantee: measured deterministic bus decay "
                "with the exact retained resistor/relay per branch is required before guard "
                "release (G33/G36, P10/P11).",
}

# M20: ventilation balance candidate from the handoff 3.7 figures.
ventilation = {
    "declared_loss_budget_W": 650.0,
    "required_service_flow_m3_h": 800.0,
    "candidate_paths": {
        "per_fan_at_50Hz_m3_h": 445.0,
        "two_path_catalogue_at_50Hz_m3_h": 890.0,
        "alternative_60Hz_fans": "four-fan Rittal SK 3240.100 class; 60 Hz is a separate case",
    },
    "service_target_filter_cases": {"final_filters": 890.0, "uniform_service_limit": 830.0},
    "balance_gates": {
        "energy_imbalance_pct": "< 1 % (computed from measurements, never input as target)",
        "mass_imbalance_pct": "< 0.1 %",
        "critical_temperature_refinement_K": "< 0.2 on final refinement",
        "normal_design_air_C": 45.0,
        "selected_driver_inlet_C": 50.0,
        "trip_bound_candidate_C": "45+2+1.5+0.5 = 49 with each term separately justified",
    },
    "decision": "candidate only: installed P-Q intersection with the real system resistance "
                "under final filters/doors/ducts must be measured (TS01-TS18).",
}

result = {
    "model": "M34/M21/M20 electrical-thermal desk closure",
    "M34_branch_absorption": cases,
    "M21_bleed_envelope": bleed,
    "M20_ventilation": ventilation,
}
payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
(HERE / "manifest.json").write_text(payload, encoding="utf-8")

lines = ["# M34 / M21 / M20 electrical-thermal desk closure", "",
         "Deterministic output of `analysis/electrical_thermal/compute.py`.", "",
         "## M34 branch absorption  `V_final = sqrt(V_i^2 + 2E/C)`", "",
         "| case | V_i | E (J) | C (F) | V_final | within +10 % of nominal |",
         "| --- | --- | --- | --- | --- | --- |"]
for c in cases:
    lines.append(f"| {c['case']} | {c['V_initial']} | {c['E_increment_J']} | {c['C_F']:.4g} "
                 f"| {c['V_final']:.3f} | {'yes' if c['safe_within_10pct_of_nominal'] else 'NO'} |")
lines += ["",
          "The stored counterexample (36 V, 100 uF, +0.1 J -> 57.411 V) demonstrates why a "
          "terminal nominal voltage is never a branch safety argument. Absorption/bleed parts "
          "per branch with measured bus decay are the G33 gate.", "",
          "## M21 bleed envelope", "",
          f"- Candidate: {bleed['candidate']}",
          f"- 36 V/2200 uF: {bleed['examples'][0]['V_after_2.6s']:.1f} V after 2.6 s ({bleed['examples'][0]['power_fraction_after_2.6s']*100:.1f} % of initial power), "
          f"{bleed['examples'][0]['V_after_60s']:.1f} V after 60 s",
          f"- 60 V/4700 uF: {bleed['examples'][1]['V_after_2.6s']:.1f} V after 2.6 s ({bleed['examples'][1]['power_fraction_after_2.6s']*100:.1f} % of initial power), "
          f"{bleed['examples'][1]['V_after_60s']:.1f} V after 60 s",
          f"- Decision: {bleed['decision']}", "",
          "## M20 ventilation balance (candidate)", "",
          f"- Declared loss budget {ventilation['declared_loss_budget_W']} W; required service "
          f"flow {ventilation['required_service_flow_m3_h']} m3/h",
          f"- Two-path catalogue {ventilation['candidate_paths']['two_path_catalogue_at_50Hz_m3_h']} m3/h "
          f"at 50 Hz (445 per fan); 60 Hz alternative is a separate case",
          f"- Decision: {ventilation['decision']}"]
(HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print("M34 counterexample V_final = %.3f V (stored figure 57.411 V)" % cases[0]["V_final"])
print("manifest sha256[:16] =", hashlib.sha256(payload.encode()).hexdigest()[:16])
