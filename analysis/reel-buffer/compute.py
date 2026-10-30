#!/usr/bin/env python3
"""M28 reel inertia/drive/brake + M29 dancer/buffer storage (review F09/F12).

Real physics for the <=30 kg dual-mode reel envelope (docs/26 geometry:
flange 400 mm, hub 60 mm, width 160 mm). `python3 analysis/reel-buffer/compute.py`
-> manifest.json + report.md.

What this computes: spool+rail inertia bounds, unwinding/brake torque demand,
dancer storage volume for the commanded demand profile, and the tension-control
requirements. What it does NOT do: select the final motor/brake identities or
validate dynamics - those are the selection/qualification gates named below.
"""
from __future__ import annotations

import hashlib
import json
import math
import pathlib

HERE = pathlib.Path(__file__).resolve().parent

# Envelope (docs/26 P001/PY-W01 + user requirement U07): <=30 kg loaded.
M_SPOOL_KG = 6.0            # welded flange+hub assembly estimate (S235JR + ERW)
M_WIRE_MAX_KG = 24.0        # 30 kg total cap minus spool
FLANGE_D_MM = 400.0
HUB_D_MM = 60.0
WIDTH_MM = 160.0
WIRE_SPEED_MM_S = 500.0     # commanded feed
WIRE_SPEED_MAX_MM_S = 750.0 # transient/regeneration case (1.5x)
WIRE_D_MM_MIN = 1.2
WIRE_D_MM_MAX = 4.5

# ---- inertia: hollow cylinder about its axis, wire pack between hub and flange
def inertia_kgm2(mass_kg: float, r_in_m: float, r_out_m: float) -> float:
    return 0.5 * mass_kg * (r_in_m * r_in_m + r_out_m * r_out_m)

r_hub = HUB_D_MM / 2000.0
r_full = FLANGE_D_MM / 2000.0
# worst-case inertia: full pack to flange radius
J_full = inertia_kgm2(M_SPOOL_KG, r_hub, r_full) + inertia_kgm2(M_WIRE_MAX_KG, r_hub, r_full)
# empty-reel inertia: spool only
J_empty = inertia_kgm2(M_SPOOL_KG, r_hub, r_full)
# average radius of a partially unwound pack (wire volume conservation)
def pack_radius_m(wire_mass_kg: float) -> float:
    # annulus area = mass / (density * width); copper ~8900 kg/m3 + PVC ~1400 mean
    rho = 5000.0  # conservative blended stranded-Cu/PVC density
    area = (wire_mass_kg / rho) / (WIDTH_MM / 1000.0)
    return math.sqrt(r_hub * r_hub + area / math.pi)

# ---- unwinding torque at commanded tension
TENSION_N_MIN = 2.0   # keep the wire taut through the buffer
TENSION_N_MAX = 8.0   # below the payoff-side damage threshold (open: confirm per lot)
def unwind_torque_nm(tension_n: float, radius_m: float) -> float:
    return tension_n * radius_m

torque_cases = []
for mass in (M_WIRE_MAX_KG, M_WIRE_MAX_KG / 2, 1.0):
    r = pack_radius_m(mass)
    omega = (WIRE_SPEED_MM_S / 1000.0) / r  # rad/s
    torque_cases.append({
        "wire_mass_kg": mass,
        "pack_radius_m": r,
        "unwind_torque_at_8N_nm": unwind_torque_nm(TENSION_N_MAX, r),
        "unwind_torque_at_2N_nm": unwind_torque_nm(TENSION_N_MIN, r),
        "pack_rpm_at_500mm_s": omega * 60.0 / (2.0 * math.pi),
        "inertia_kgm2": inertia_kgm2(M_SPOOL_KG, r_hub, r_full) + inertia_kgm2(mass, r_hub, r),
    })

# ---- dancer/buffer storage: absorb the demand-vs-supply mismatch
# worst case: feed accelerates from 0 to 750 mm/s in 50 ms while the payoff
# supply slews at its own limit; storage = integral of the speed difference.
ACCEL_MM_S2 = 15000.0   # strip accel class (300 mm/s2 blade; feed 500 mm/s in 350 ms -> use 1.5 m/s2*10)
SUPPLY_SLEW_MM_S2 = 2500.0  # spool+brake can follow at this rate (to be validated M29)
t_catchup = (WIRE_SPEED_MAX_MM_S - 0.0) / SUPPLY_SLEW_MM_S2
storage_mm = 0.5 * (ACCEL_MM_S2 - SUPPLY_SLEW_MM_S2) * min(0.05, t_catchup) ** 2 + \
             max(0.0, (WIRE_SPEED_MAX_MM_S - SUPPLY_SLEW_MM_S2 * 0.05)) * max(0.0, t_catchup - 0.05)
storage_mm = max(storage_mm, 100.0)  # floor: minimum dancer travel provision

result = {
    "model": "M28 reel inertia/drive/brake + M29 dancer storage (review F09)",
    "envelope": {"total_mass_cap_kg": 30.0, "flange_mm": FLANGE_D_MM, "hub_mm": HUB_D_MM,
                 "width_mm": WIDTH_MM, "modes": ["small-reel cassette/adapter", "large-reel cradle"],
                 "note": "envelope from docs/26 + U07; final reel package freeze is owner decision D3"},
    "inertia": {"J_full_kgm2": J_full, "J_empty_kgm2": J_empty,
                "worst_case_note": "full pack to flange radius is the bounding inertia"},
    "torque_demand": {
        "cases": torque_cases,
        "brake_requirement_nm": max(c["unwind_torque_at_8N_nm"] for c in torque_cases),
        "drive_requirement_nm": max(c["unwind_torque_at_8N_nm"] for c in torque_cases),
        "note": "brake must hold tension at full pack radius; drive must follow the demand "
                "slew; exact motor/brake identities are the selection gate (G38/G39)",
    },
    "dancer_storage": {
        "required_travel_mm": storage_mm,
        "demand_profile": "0 -> 750 mm/s in 50 ms vs supply slew 2500 mm/s2 (to be validated)",
        "note": "storage floor 100 mm; full validated dancer/control design is M29",
    },
    "open_gates": [
        "reel envelope freeze (D3): exact package OD/core/width/bore per family",
        "drive/brake selection with identified parts and hot ratings (G38/G39)",
        "dancer mechanism, control loop and fault behaviour (M29, P08/P14)",
        "full/mid/empty tension and storage validation on hardware (P06/P15)",
        "30 kg loading method and retention (P15-T4)",
    ],
}
payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
(HERE / "manifest.json").write_text(payload, encoding="utf-8")

lines = ["# M28/M29 reel and buffer physics (desk closure)", "",
         "Deterministic output of `analysis/reel-buffer/compute.py` - replaces the "
         "mislabelled tooth-pass script note (review F12: tooth-pass is M28-M16 "
         "dynamics input, NOT a reel model).", "",
         f"- Envelope: {result['envelope']['total_mass_cap_kg']} kg total, flange "
         f"{FLANGE_D_MM:.0f} mm, hub {HUB_D_MM:.0f} mm, width {WIDTH_MM:.0f} mm "
         "(dual-mode small cassette + large cradle; package freeze = owner D3)",
         f"- Inertia: J_full = {J_full:.4f} kg.m2, J_empty = {J_empty:.4f} kg.m2",
         "- Unwind torque demand:", ""]
for c in torque_cases:
    lines.append(f"  - wire {c['wire_mass_kg']:.1f} kg at r={c['pack_radius_m']*1000:.0f} mm: "
                 f"{c['unwind_torque_at_8N_nm']:.2f} N.m at 8 N tension, "
                 f"{c['unwind_torque_at_2N_nm']:.2f} N.m at 2 N; "
                 f"{c['pack_rpm_at_500mm_s']:.1f} rpm at 0.50 m/s")
lines += ["",
          f"- Brake hold requirement: {result['torque_demand']['brake_requirement_nm']:.2f} N.m at full-pack radius",
          f"- Dancer storage floor: {result['dancer_storage']['required_travel_mm']:.0f} mm "
          "(0 -> 750 mm/s demand vs supply slew)", "",
          "Open gates:", ""]
for g in result["open_gates"]:
    lines.append(f"- {g}")
(HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"J_full={J_full:.4f} kg.m2 brake={result['torque_demand']['brake_requirement_nm']:.2f} N.m "
      f"storage={storage_mm:.0f} mm")
print("manifest sha256[:16] =", hashlib.sha256(payload.encode()).hexdigest()[:16])
