#!/usr/bin/env python3
"""Transport-geometry envelope proof (M27/M30/M28 rigorous application).

`python3 analysis/transport-geometry/compute.py` -> manifest.json + report.md.
Sweeps the FULL validated envelope (L 200-3000 mm; per-end strips 0 or 3-25 mm)
and proves numerically, for every cell:

  I1  tail moves (backfeed s2, pull s2+3) are FULLY observable at station D
      (the piece spans D throughout each move),
  I2  the eject's observable window is D - tail0 (unobserved remainder > 0 for
      every cell -> discharge evidence is mandatory in every case),
  I3  the right cassette grips the piece through its tail moves (far nip 140 mm)
      and loses grip only during the eject coast,
  I4  severed bodies shorter than the grip boundary are never propel-lable ->
      the waste path must accept them (chute capacity requirement),
  I5  D is occupied by the piece immediately at sever for every valid L.

Also derives the belt tooth-pass frequency independently (M28) from the actual
pulley geometry (20 teeth x 5 mm pitch on the roller shaft, 1:1).
"""
from __future__ import annotations

import hashlib
import json
import math
import pathlib

HERE = pathlib.Path(__file__).resolve().parent

D_MM = 106.75            # ctrl::sensing::piece_station_x_mm
GRIP_MM = 140.0          # ctrl::sensing::piece_body_grip_x_mm (far right nip)
NEAR_NIP_MM = 75.5       # feed::design::near_nip_x
DISCHARGE_LO_MM = 230.0  # ctrl::sensing discharge zone
DISCHARGE_HI_MM = 270.0
EJECT_MM = 230.0         # ctrl::job::ejection_um
STRIP_CLEAR_MM = 3.0
MIN_L_MM = 200.0
MAX_L_MM = 3000.0
STRIPS = [0.0] + [float(s) for s in range(3, 26)]

# M28 derivation (feed_drive_spec: HTPA20S5M250-A-N14 pulley, 20T, 5 mm pitch,
# 1:1 on the roller shaft; UMCL40 roller 40 mm OD).
ROLLER_D_MM = 40.0
PULLEY_TEETH = 20
BELT_PITCH_MM = 5.0
WIRE_SPEED_MM_S = 500.0
shaft_rps = WIRE_SPEED_MM_S / (math.pi * ROLLER_D_MM)
tooth_pass_hz = PULLEY_TEETH * shaft_rps
belt_speed_mm_s = shaft_rps * PULLEY_TEETH * BELT_PITCH_MM


def observed_D(tail0: float, length: float, delta: float) -> float:
    """Distance of a piece move that station D observes (controller formula)."""
    abs_d = abs(delta)
    sign = 1.0 if delta >= 0 else -1.0
    t0 = tail0
    bound_a = (D_MM - length - t0) / sign
    bound_b = (D_MM - t0) / sign
    lo, hi = 0.0, abs_d
    if sign > 0:
        lo, hi = max(lo, bound_a), min(hi, bound_b)
    else:
        lo, hi = max(lo, bound_b), min(hi, bound_a)
    return max(0.0, hi - lo)


def main() -> None:
    cells = 0
    violations: list[str] = []
    min_pull_observed_frac = 1.0
    min_backfeed_observed_frac = 1.0
    eject_windows = []
    eject_unobserved = []
    driven_eject = []
    min_grip_margin = 1e9

    for L in (200.0, 250.0, 300.0, 500.0, 1000.0, 1500.0, 2000.0, 2500.0, 3000.0):
        for s1 in STRIPS:
            for s2 in STRIPS:
                cells += 1
                # post-sever geometry per docs/01 step sequence, derived from the
                # implemented phase states (review F11 correction): after tail
                # withdraw the tail sits at -s2; the pull moves it +(s2+3), so
                # the tail at ejection is ALWAYS +3 mm with an enabled tail
                # strip (never s2+3), and 0 without one.
                tail_at_score = -s2 if s2 else 0.0
                tail_at_eject = STRIP_CLEAR_MM if s2 else 0.0
                # I3 grip through tail moves: piece [tail, tail+L] must cover far nip
                grip_margin = (tail_at_score + L) - GRIP_MM
                if grip_margin < min_grip_margin:
                    min_grip_margin = grip_margin
                if grip_margin < 0:
                    violations.append(f"I3 L={L} s2={s2}: no grip during tail score")
                # I1 tail moves fully observed
                backfeed_obs = observed_D(0.0, L, -s2) if s2 else 0.0
                pull_obs = observed_D(tail_at_score, L, s2 + STRIP_CLEAR_MM) if s2 else 0.0
                if s2:
                    if backfeed_obs < s2 - 1e-9:
                        violations.append(f"I1 L={L} s2={s2}: backfeed {backfeed_obs}<{s2}")
                        min_backfeed_observed_frac = min(min_backfeed_observed_frac, backfeed_obs / s2)
                    if pull_obs < s2 + STRIP_CLEAR_MM - 1e-9:
                        violations.append(f"I1 L={L} s2={s2}: pull {pull_obs}<{s2 + STRIP_CLEAR_MM}")
                        min_pull_observed_frac = min(min_pull_observed_frac, pull_obs / (s2 + STRIP_CLEAR_MM))
                # I2 eject window and unobserved remainder
                win = observed_D(tail_at_eject, L, EJECT_MM)
                eject_windows.append(win)
                eject_unobserved.append(EJECT_MM - win)
                if win >= EJECT_MM - 1e-9:
                    violations.append(f"I2 L={L} s1={s1} s2={s2}: fully observed eject (unexpected)")
                # I5 D occupied at sever
                if L < D_MM:
                    violations.append(f"I5 L={L}: piece does not span D at sever")
                # gripped eject distance and required takeaway slide
                driven = GRIP_MM - tail_at_eject
                driven_eject.append(driven)
                # piece tail must pass discharge zone end: slide >= HI - GRIP
                slide_required = DISCHARGE_HI_MM - GRIP_MM
                if slide_required < 0:
                    violations.append("discharge zone lies inside gripped region")

    # I4 waste-path capacity: any severed body shorter than the grip boundary is
    # loose (prime stub, or interrupted-stock tips below 140 mm) -> the chute
    # must accept bodies up to just below the grip boundary, not only 10-25 mm.
    chute_min_capacity_mm = GRIP_MM

    result = {
        "model": "transport-geometry envelope proof (M27/M30/M28)",
        "envelope_cells": cells,
        "method_note": "interval arithmetic over a 9 x 24 x 24 sampled grid of the "
                       "validated envelope (L x lead x tail), evaluated from the "
                       "implemented phase states; it is NOT 5184 end-to-end runs and "
                       "not a validated contact/control model. The same ejection "
                       "identity is checked against runtime in the host "
                       "origin_transfer test; a wrong tail-origin variant must be "
                       "rejected there (review F11).",
        "runtime_identity": "signed D material over one piece = L exactly (feed part "
                            "(L - X_D) + backfeed -s2 + pull +(s2+3) + eject window "
                            "(X_D - 3)); validated by tests/P00-P15 and the "
                            "origin_transfer host group",
        "invariants": {
            "I1_tail_moves_fully_observable": not any(v.startswith("I1") for v in violations),
            "I2_eject_always_partially_unobserved": not any(v.startswith("I2") for v in violations),
            "I3_grip_held_through_tail_moves": not any(v.startswith("I3") for v in violations),
            "I5_piece_spans_D_at_sever": not any(v.startswith("I5") for v in violations),
        },
        "violations": violations,
        "derived": {
            "eject_window_mm": {"min": min(eject_windows), "max": max(eject_windows)},
            "eject_unobserved_mm": {"min": min(eject_unobserved), "max": max(eject_unobserved)},
            "discharge_evidence_required_in_every_cell": all(u > 0 for u in eject_unobserved),
            "driven_eject_mm": {"min": min(driven_eject), "max": max(driven_eject)},
            "takeaway_min_slide_mm": DISCHARGE_HI_MM - GRIP_MM,
            "waste_chute_min_body_capacity_mm": chute_min_capacity_mm,
            "waste_chute_note": "bodies below the grip boundary are never propel-lable: the "
                                "waste path must accept severed bodies up to just under "
                                "140 mm (interrupted-stock tips), not only 10 mm stubs "
                                "and 3-25 mm slugs",
            "min_grip_margin_at_far_nip_mm": min_grip_margin,
        },
        "M28_tooth_pass": {
            "pulley_teeth": PULLEY_TEETH,
            "pitch_mm": BELT_PITCH_MM,
            "shaft_rps_at_500mm_s": shaft_rps,
            "tooth_pass_hz": tooth_pass_hz,
            "belt_speed_mm_s": belt_speed_mm_s,
            "cross_check": "matches the handoff's corrected 79.5775 Hz nominal: the pulley "
                           "runs 1:1 on the roller shaft, so tooth pass = teeth x wire "
                           "speed / (pi x roller D), NOT teeth x wire speed / belt pitch "
                           "circumference (belt speed is 397.9 mm/s, below wire speed)",
        },
    }
    payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
    (HERE / "manifest.json").write_text(payload, encoding="utf-8")

    lines = ["# Transport-geometry envelope proof (M27/M30/M28)", "",
             f"Interval arithmetic over {cells} sampled envelope cells (9 lengths x 24 lead "
             "x 24 tail settings) evaluated from the implemented phase states - not "
             "end-to-end runs, not a contact/control model (see manifest method_note). "
             "Deterministic output of `analysis/transport-geometry/compute.py`. The lead "
             "setting is swept for coverage but does not enter the ejection equations: "
             "it changes the feed start position, which cancels in the identity.", "",
             "## Invariants", ""]
    for k, v in result["invariants"].items():
        lines.append(f"- {'PASS' if v else 'FAIL'} — {k}")
    lines += ["", f"Violations: {len(violations)}", ""]
    for v in violations[:20]:
        lines.append(f"  - {v}")
    d = result["derived"]
    lines += ["", "## Derived design requirements", "",
              f"- Eject observation window at D: {d['eject_window_mm']['min']:.2f}-"
              f"{d['eject_window_mm']['max']:.2f} mm of the {EJECT_MM:.0f} mm move; "
              f"unobserved remainder {d['eject_unobserved_mm']['min']:.2f}-"
              f"{d['eject_unobserved_mm']['max']:.2f} mm in every cell -> "
              f"discharge evidence mandatory: {d['discharge_evidence_required_in_every_cell']}",
              f"- Gripped eject distance: {d['driven_eject_mm']['min']:.1f}-"
              f"{d['driven_eject_mm']['max']:.1f} mm; the takeaway must at minimum slide a "
              f"further {d['takeaway_min_slide_mm']:.0f} mm to clear the discharge zone end",
              f"- Waste chute minimum body capacity: {d['waste_chute_min_body_capacity_mm']:.0f} mm "
              f"({d['waste_chute_note']})",
              f"- Minimum grip margin at the far nip during tail score: "
              f"{d['min_grip_margin_at_far_nip_mm']:.1f} mm", "",
              "## M28 tooth-pass derivation", "",
              f"- Pulley {PULLEY_TEETH} teeth x {BELT_PITCH_MM} mm pitch, 1:1 on the roller shaft; "
              f"shaft {shaft_rps:.5f} rps at 0.50 m/s",
              f"- Tooth-pass frequency: {tooth_pass_hz:.4f} Hz; belt speed "
              f"{belt_speed_mm_s:.1f} mm/s (below wire speed because the pulley pitch "
              f"circumference {PULLEY_TEETH * BELT_PITCH_MM:.0f} mm < roller circumference "
              f"{math.pi * ROLLER_D_MM:.1f} mm)",
              f"- {result['M28_tooth_pass']['cross_check']}"]
    (HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"cells={cells} violations={len(violations)} tooth_pass={tooth_pass_hz:.4f} Hz")
    print("manifest sha256[:16] =", hashlib.sha256(payload.encode()).hexdigest()[:16])


if __name__ == "__main__":
    main()
