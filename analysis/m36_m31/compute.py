#!/usr/bin/env python3
"""M36 feed pulse/timing envelope + M31 metrology error budget (desk closure).

Reproducible: `python3 analysis/m36_m31/compute.py` writes report.md and
manifest.json beside itself from the constants below; rerunning from the same
inputs reproduces byte-identical outputs (except the report date line is not
used - all values are deterministic).

Input provenance is explicit: every constant cites its single active source.
The legacy 3200-pulses/rev / 40 kHz pair (shared/machine_spec.hpp) is the
quarantined reference-software scale and is never used here.
"""
from __future__ import annotations

import hashlib
import json
import math
import pathlib

HERE = pathlib.Path(__file__).resolve().parent

# ---- single active source of truth: shared/controls_spec.hpp --------------
FEED_PULSES_PER_REV = 2000.0          # ctrl::axis::feed_pulses_per_rev
ROLLER_D_MM = 40.0                    # feed::catalogue::roller_d (UMCL40)
ROLLER_C_MM = math.pi * ROLLER_D_MM   # 125.6637 mm nominal circumference
FEED_STEPS_PER_MM = FEED_PULSES_PER_REV / ROLLER_C_MM
FEED_SPEED_MM_S = 500.0               # ctrl::axis::feed_speed_mm_s (0.50 m/s)
SHAFT_LIMIT_RPM = 260.0               # ctrl::axis::feed_shaft_limit_rpm
STEP_RATE_CEILING_HZ = 8000.0         # ctrl::axis::max_step_hz (production)
COMMISSION_SPEED_MM_S = 50.0          # ctrl::axis::strip_speed (0.05 m/s class)
NOMINAL_COUNTS_PER_M = 40960          # ctrl::enc::nominal_counts_per_metre
UM_PER_COUNT = 24.4140625             # ctrl::enc::nominal_um_per_count
WHEEL_OD_MM = 63.66                   # measurement_spec wheel_od_mm
WHEEL_OD_TOL_MM = 0.1
LENGTH_TOL_BASE_UM = 200.0            # G01/G02: 0.20 mm
LENGTH_TOL_SLOPE = 0.0005             # 0.05 % of L
STRIP_TOL_UM = 200.0                  # G02 per enabled end

# ---- M36: pulse/timing envelope ------------------------------------------
demand_hz = FEED_SPEED_MM_S * FEED_STEPS_PER_MM
rpm_at_speed = FEED_SPEED_MM_S / ROLLER_C_MM * 60.0
rate_margin_pct = (STEP_RATE_CEILING_HZ - demand_hz) / STEP_RATE_CEILING_HZ * 100.0
rpm_margin_pct = (SHAFT_LIMIT_RPM - rpm_at_speed) / SHAFT_LIMIT_RPM * 100.0
# Loaded radius: urethane tread deflection and wear shrink the effective
# circumference. The step rate rises as 1/r while the commanded linear speed
# stays 500 mm/s, so the ceiling sets a minimum loaded radius.
min_loaded_radius_mm = (FEED_SPEED_MM_S * FEED_PULSES_PER_REV) / (2.0 * math.pi * STEP_RATE_CEILING_HZ)
nominal_radius_mm = ROLLER_D_MM / 2.0
allowed_radius_loss_mm = nominal_radius_mm - min_loaded_radius_mm

# ---- M31: length error budget --------------------------------------------
# GUM-style half-width budget for the finished relaxed length at L = 1000 mm
# (worst-case external half-width T_L = 700 um). Terms are engineering
# allocations; physical terms carry their qualification gate (open until the
# corresponding P-test).
def tolerance_um(L_um: int) -> float:
    return LENGTH_TOL_BASE_UM + LENGTH_TOL_SLOPE * L_um

budget = [
    # (term, half-width um, evidence class, gate)
    ("command_quantisation", 2 * UM_PER_COUNT, "host: 2 independent count quantisations + one accumulated fraction (test_cycle)", "closed_host"),
    ("wheel_scale_calibration", 25.0, "calibration procedure + gauge R&R pending (P13)", "open_P13"),
    ("loaded_radius_wear", 15.0, "wear/drift envelope pending (P06/P13)", "open_P13"),
    ("slip_within_grip", 50.0, "planned movement verified against the calibrated wheel with tested error allocation (M26/M31)", "open_P13"),
    ("tension_relaxation", 60.0, "wire relaxation model validated with held-out physical data (M23)", "open_P13"),
    ("thermal_expansion_length", 25.0, "thermal drift (M23) + environment record (D1)", "open_P10"),
    ("blade_datum_and_incision", 20.0, "blade datum stability (P13)", "open_P13"),
    ("handling_deflection", 30.0, "output/handling deflection (M30/P08)", "open_P08"),
    ("measurement_decision", 25.0, "predeclared conformity rule |e|+U<=T with U<=0.05 mm gauge (M31)", "open_P13"),
]
budget_total = math.sqrt(sum(u * u for _, u, _, _ in budget))   # independent terms
budget_rss_excl_gates = math.sqrt(sum(u * u for _, u, _, g in budget if g == "closed_host"))

# ---- report/manifest ------------------------------------------------------
def compute() -> dict:
    return {
        "model": "M36 feed-timing envelope + M31 length error budget",
        "inputs": {
            "feed_pulses_per_rev": FEED_PULSES_PER_REV,
            "roller_d_mm": ROLLER_D_MM,
            "feed_speed_mm_s": FEED_SPEED_MM_S,
            "step_rate_ceiling_hz": STEP_RATE_CEILING_HZ,
            "shaft_limit_rpm": SHAFT_LIMIT_RPM,
            "nominal_counts_per_m": NOMINAL_COUNTS_PER_M,
            "length_tol_base_um": LENGTH_TOL_BASE_UM,
            "length_tol_slope": LENGTH_TOL_SLOPE,
            "source_of_truth": "shared/controls_spec.hpp (legacy 3200/40000 pair quarantined)",
        },
        "M36": {
            "roller_circumference_mm": ROLLER_C_MM,
            "feed_steps_per_mm": FEED_STEPS_PER_MM,
            "demand_hz_at_500mm_s": demand_hz,
            "ceiling_hz": STEP_RATE_CEILING_HZ,
            "rate_margin_pct": rate_margin_pct,
            "shaft_rpm_at_500mm_s": rpm_at_speed,
            "shaft_rpm_margin_pct": rpm_margin_pct,
            "min_loaded_radius_mm": min_loaded_radius_mm,
            "nominal_radius_mm": nominal_radius_mm,
            "allowed_radius_loss_mm": allowed_radius_loss_mm,
            "decision": "production ceiling stays 8000 Hz; loaded-radius loss must stay <= "
                        f"{allowed_radius_loss_mm:.5f} mm (min loaded radius {min_loaded_radius_mm:.5f} mm). "
                        "A higher qualified ceiling or recalibrated loaded radius is a recorded alternative (M36 gate).",
        },
        "M31": {
            "case_L_um": 1000000,
            "external_half_width_um": tolerance_um(1000000),
            "strip_half_width_um": STRIP_TOL_UM,
            "budget_terms": [
                {"term": t, "half_width_um": u, "evidence": e, "gate": g} for t, u, e, g in budget
            ],
            "budget_rss_um": budget_total,
            "host_closed_terms_rss_um": budget_rss_excl_gates,
            "margin_um_at_1000mm": tolerance_um(1000000) - budget_total,
            "decision": "external tolerance holds only if the RSS of the declared half-widths stays below T_L with "
                        "the predeclared |e|+U<=T conformity rule; all open terms must close at their P-gates "
                        "before capability claims (G05).",
        },
    }


def write_outputs() -> None:
    result = compute()
    payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
    (HERE / "manifest.json").write_text(payload, encoding="utf-8")
    m36 = result["M36"]
    m31 = result["M31"]
    lines = [
        "# M36 feed timing / M31 length error budget (desk closure)",
        "",
        "Deterministic output of `analysis/m36_m31/compute.py`; values cite the single",
        "active source `shared/controls_spec.hpp` (legacy 3200/40000 constants quarantined).",
        "",
        "## M36 pulse/timing envelope",
        "",
        f"- Roller circumference (nominal): {m36['roller_circumference_mm']:.4f} mm; feed scale {m36['feed_steps_per_mm']:.4f} steps/mm",
        f"- Demand at 0.50 m/s: {m36['demand_hz_at_500mm_s']:.2f} Hz vs ceiling {m36['ceiling_hz']:.0f} Hz -> margin {m36['rate_margin_pct']:.2f} %",
        f"- Shaft speed {m36['shaft_rpm_at_500mm_s']:.2f} rpm vs limit {m36['shaft_rpm_margin_pct']:.2f} % margin (ceiling 260 rpm)",
        f"- Minimum loaded radius at the 8 kHz ceiling: {m36['min_loaded_radius_mm']:.5f} mm "
        f"(allowed tread/wear loss {m36['allowed_radius_loss_mm']:.5f} mm from nominal {m36['nominal_radius_mm']:.1f} mm)",
        f"- Decision: {m36['decision']}",
        "",
        "## M31 length error budget (half-widths, um)",
        "",
        "| term | half-width um | evidence | gate |",
        "| --- | --- | --- | --- |",
    ]
    for t in m31["budget_terms"]:
        lines.append(f"| {t['term']} | {t['half_width_um']:.1f} | {t['evidence']} | {t['gate']} |")
    lines += [
        "",
        f"- RSS of declared half-widths: {m31['budget_rss_um']:.1f} um vs external T_L(1000 mm) = "
        f"{m31['external_half_width_um']:.0f} um -> margin {m31['margin_um_at_1000mm']:.1f} um",
        f"- Host-closed terms only: {m31['host_closed_terms_rss_um']:.1f} um (quantisation class)",
        f"- Decision: {m31['decision']}",
        "",
        "Open terms are physical gates (P06/P08/P10/P13), not fabrication: they are",
        "scheduled with their test procedures and must not be closed by model output.",
    ]
    (HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    digest = hashlib.sha256(payload.encode()).hexdigest()[:16]
    print(f"manifest.json sha256[:16]={digest}")
    print(f"rate_margin_pct={m36['rate_margin_pct']:.3f} min_loaded_radius_mm={m36['min_loaded_radius_mm']:.5f}")
    print(f"budget_rss_um={m31['budget_rss_um']:.1f} margin_um={m31['margin_um_at_1000mm']:.1f}")


if __name__ == "__main__":
    write_outputs()
