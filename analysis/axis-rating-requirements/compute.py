#!/usr/bin/env python3
"""M04/M06 axis rating requirements (review F06).

`python3 analysis/axis-rating-requirements/compute.py` -> manifest.json + report.md.

Computes the MINIMUM acceptable catalogue ratings for every load-bearing
element of the S1-B custom cut axis from the unchanged acceptance loads
(docs/39 / handoff goal 12/37). It does not select parts: it produces the
exact selection criteria a candidate must be verified against, and the
contradictions a candidate must not repeat (the 4900 N support vs 5000 N
proof class, and the 340/400 mm rail identity mismatch in the register).
"""
from __future__ import annotations

import hashlib
import json
import math
import pathlib

HERE = pathlib.Path(__file__).resolve().parent

# Unchanged acceptance loads (handoff goal 12; must never be relaxed):
AXIAL_N = 2500.0          # working axial
TRANSVERSE_N = 750.0
VERTICAL_N = 375.0
PROOF_FACTOR = 2.0
PROOF_AXIAL_N = AXIAL_N * PROOF_FACTOR      # 5000 N
# Fault/stall thrust bound (handoff 3.7 / docs/39): 8.5 N.m motor torque at
# 10 mm lead, efficiency eta = 1 upper bound; real eta < 1 raises the force.
LEAD_MM = 10.0
MOTOR_TORQUE_NM = 8.5
TORQUE_TOLERANCE_FACTOR = 1.2   # motor tolerance + impact allowance (declared)
ETA_MIN = 0.85                  # declared lower efficiency bound (verify per screw)

stall_thrust_ideal_n = MOTOR_TORQUE_NM * 2.0 * math.pi / (LEAD_MM / 1000.0) / 1.0
stall_thrust_bound_n = MOTOR_TORQUE_NM * TORQUE_TOLERANCE_FACTOR * 2.0 * math.pi / (LEAD_MM / 1000.0) / ETA_MIN

# Minimum catalogue ratings (selection acceptance criteria):
criteria = {
    "linear_guide_block_set": {
        "static_load_C0_N_min": PROOF_AXIAL_N * 1.5,      # proof x margin
        "dynamic_load_C_N_min": stall_thrust_bound_n * 2.0, # life basis (L10 target G15)
        "note": "per axis, 2 sets; HGR20-class candidates must show BOTH C0 and C "
                "against these numbers - the old generic '4900 N class' figure is "
                "below the 5000 N proof and may never be reused",
    },
    "ball_screw_nut": {
        "dynamic_rating_N_min": stall_thrust_bound_n * 1.5,
        "static_rating_N_min": PROOF_AXIAL_N * 1.5,
        "note": "SFU1610-class C7 10 mm lead; end machining 8 h7x18; the published "
                "nut rating must be checked at the declared preload class",
    },
    "fixed_support_BK_class": {
        "axial_rating_N_min": max(PROOF_AXIAL_N, stall_thrust_bound_n) * 1.25,
        "note": "the support carrying the fault-thrust direction; its axial rating "
                "must clear BOTH the 5000 N proof and the stall bound with margin - "
                "the 4900 N contradiction is resolved ONLY by a rated part",
    },
    "floating_support_BF_class": {
        "axial_rating_N_min": PROOF_AXIAL_N * 1.0,
        "note": "floating side; radial capacity per guide reactions",
    },
    "coupling": {
        "rated_torque_nm_min": MOTOR_TORQUE_NM * TORQUE_TOLERANCE_FACTOR,
        "note": "reversal duty (1 N.m x 2.5 service factor for feed; head axes see "
                "the full stall bound); 16 mm fit length on the exposed shaft",
    },
    "motor_adapter_joint": {
        "note": "the R6 adapter FEM still FAILS its screen (285.987 MPa vs 216.667 "
                "MPa; refinement 18.46%/4.96% vs 5%/3% gates) - no rating number "
                "releases it; the joint redesign + converged re-solve is the gate "
                "(review F07)",
    },
}

# Register consistency contradictions to eliminate:
contradictions = [
    "AX-01 interface text says 'rail 440 total' while AX-R01 is a 400 mm rail: the "
    "register must state ONE rail geometry per axis (KK8610 lineage is 340 mm rail / "
    "440 overall; the custom HGR20 lineage is 400 mm rail / 440 base) - mixing the "
    "two lineages in one row is a source-quality defect",
    "support rating class 4900 N vs 5000 N proof: never rounded, never reused",
    "340/400 mm mismatch across engineering/README (KK8610 purchased-axis line) and "
    "bom/selected-cutting-head.csv (custom axis): the cost goal selects the custom "
    "axis; the purchased-axis line remains the comparison baseline, not the build",
]

result = {
    "model": "M04/M06 axis rating requirements (review F06)",
    "unchanged_loads": {
        "working_axial_N": AXIAL_N, "transverse_N": TRANSVERSE_N, "vertical_N": VERTICAL_N,
        "proof_axial_N": PROOF_AXIAL_N, "proof_factor": PROOF_FACTOR,
    },
    "stall_thrust": {
        "ideal_eta1_N": stall_thrust_ideal_n,
        "declared_bound_N": stall_thrust_bound_n,
        "assumptions": {"motor_torque_nm": MOTOR_TORQUE_NM,
                        "torque_tolerance_factor": TORQUE_TOLERANCE_FACTOR,
                        "eta_min": ETA_MIN, "lead_mm": LEAD_MM},
        "note": "the 5340.708 N eta=1 figure is a DECLARED BOUND, not an advantage; "
                "the eta_min case is the selection basis",
    },
    "selection_criteria": criteria,
    "register_contradictions_to_eliminate": contradictions,
    "open_gates": [
        "select exact rated parts against these criteria and re-verify with catalogue "
        "documents (G38/G39); catalogue family text is not a quote",
        "complete native axis CAD with the selected parts and manufacturing drawings",
        "re-run all load signs/proof/fault cases and the loaded-gap/stiffness chain "
        "on the selected assembly (M04/M06/M08)",
    ],
}
payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
(HERE / "manifest.json").write_text(payload, encoding="utf-8")

lines = ["# M04/M06 axis rating requirements (review F06)", "",
         "Deterministic output of `analysis/axis-rating-requirements/compute.py`.", "",
         "## Unchanged acceptance loads", "",
         f"- working: axial {AXIAL_N:.0f} N, transverse {TRANSVERSE_N:.0f} N, vertical {VERTICAL_N:.0f} N",
         f"- proof: {PROOF_AXIAL_N:.0f} N (double) - never relaxed", "",
         "## Stall thrust", "",
         f"- ideal (eta=1): {stall_thrust_ideal_n:.1f} N (the stored 5340.708 N bound)",
         f"- declared selection basis (torque x{TORQUE_TOLERANCE_FACTOR}, eta={ETA_MIN}): "
         f"{stall_thrust_bound_n:.1f} N", "",
         "## Minimum catalogue ratings (selection criteria)", ""]
for k, v in criteria.items():
    nums = ", ".join(f"{ik}={iv}" for ik, iv in v.items() if ik != "note")
    lines.append(f"- **{k}**: {nums}")
    lines.append(f"  {v['note']}")
lines += ["", "## Register contradictions to eliminate", ""]
for c in contradictions:
    lines.append(f"- {c}")
lines += ["", "## Open gates", ""]
for g in result["open_gates"]:
    lines.append(f"- {g}")
(HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"stall ideal={stall_thrust_ideal_n:.1f} N bound={stall_thrust_bound_n:.1f} N "
      f"proof={PROOF_AXIAL_N:.0f} N")
print("manifest sha256[:16] =", hashlib.sha256(payload.encode()).hexdigest()[:16])
