#!/usr/bin/env python3
"""Fastener tightening-torque computation (closes the manual TORQUE PENDING gaps).

`python3 analysis/fastener-torques/compute.py` -> manifest.json + torques.csv.

Method (VDI 2230 R1 class calculation, stated explicitly):
    F_M = 0.7 * A_s * R_p0.2     target assembly preload (70 % of proof)
    T   = K * d * F_M            K = 0.11 (lightly oiled / standard condition)
Torque tolerance +/-10 % at the wrench. Proof loads use ISO 898-1 tensile
stress areas A_s and class property classes:
    8.8:  R_p0.2 = 660 MPa     10.9: R_p0.2 = 900 MPa
Where a joint's FEM preload differs (the native adapter analysis applies
8.0 kN per M5 F0 screw = ~60 % class proof), the computed torque is given for
the standard 70 % target and the analysis preload is noted separately.

These are CALCULATED values for the specified fastener classes and surface
condition. If the released assembly process specifies a different surface
condition (lubricated, plated, waxed) the torque must be recomputed with the
matching K - a wrong friction assumption is the classic way to miss preload.
"""
from __future__ import annotations

import csv
import hashlib
import json
import pathlib
import re

HERE = pathlib.Path(__file__).resolve().parent

K_TORQUE = 0.11            # friction coefficient class (lightly oiled)
PRELOAD_FRACTION = 0.70    # of proof
RP02 = {"8.8": 660.0, "10.9": 900.0}   # MPa (ISO 898-1)
AS_MM2 = {"M3": 5.03, "M4": 8.78, "M5": 14.2, "M6": 20.1,
          "M8": 36.6, "M10": 58.0, "M12": 84.3}   # ISO 898-1 stress areas
D_MM = {"M3": 3.0, "M4": 4.0, "M5": 5.0, "M6": 6.0, "M8": 8.0, "M10": 10.0, "M12": 12.0}


def torque_nm(dia: str, cls: str) -> tuple[float, float, float]:
    a_s = AS_MM2[dia]
    f_proof = a_s * RP02[cls] / 1000.0        # kN
    f_m = PRELOAD_FRACTION * f_proof          # kN
    t = K_TORQUE * D_MM[dia] * f_m * 1000.0 / 1000.0   # N.m
    return f_proof, f_m, t


JOINTS = [
    # (joint, fastener, class, washer/notes, usage)
    ("A base/axis plates", "ISO4762 M5x16", "10.9", "ISO7092 M5 washer (HF0-B/HF0-W class)",
     "H005 base, H006 carriage (4x per carriage), frame risers"),
    ("B motor adapter", "ISO4762 M5x25", "10.9", "HM-W ISO7092 M5 5.3x9x1 reduced-OD washer",
     "8x per axis; 15 mm adapter thread engagement; FEM preload 8.0 kN/screw noted"),
    ("B rail screws", "ISO4762 M5", "10.9", "ISO7092 M5 washer",
     "HGR20 rail at 60 mm pitch"),
    ("B supports", "ISO4762 M5", "10.9", "ISO7092 M5 washer", "BK12/BF12 lands"),
    ("D cassette front module", "ISO4762 M4x20", "10.9", "TR-FW02 ISO7092 M4 washer",
     "F112 front module, 9 mm nominal engagement"),
    ("D encoder plate", "ISO4762 M4x12", "8.8", "ISO7092 M4 washer",
     "M201 into ifm RU3100 M4x0.7x6 flange taps - CAP at ifm's stated tap limit "
     "(manufacturer value to be read from the datasheet at release)"),
    ("D cassette cover", "ISO4762 M3x8", "8.8", "-", "F103 cover to F112"),
    ("E payoff posts", "ISO4762 M8", "8.8", "ISO4032 M8 lock nut where used",
     "P004/P005 to P003 base"),
    ("E spindle retention", "KM-class M20x1.5 nut", "-", "tab washer",
     "bearing retention both ends - use the bearing manufacturer's nut torque "
     "table (KM-class), computed value below is the generic 70 % class estimate"),
    ("A bed-to-frame", "ISO4762 M10x35", "8.8", "-", "frame to base"),
    ("A floor anchors", "M12 anchors", "-", "per anchor datasheet",
     "anchor installation torque is anchor-specific - use the anchor datasheet"),
    ("F guards", "ISO4762 M5", "8.8", "ISO7092 M5 washer", "guard fasteners"),
    ("misc lock nuts", "ISO4032 M6", "8.8", "-", "wiring/mechanism retention"),
]

rows = []
for joint, fastener, cls, washer, usage in JOINTS:
    tok = fastener.split()[1] if fastener.split()[0] == "ISO4762" else fastener
    m = re.search(r"M\d+", tok if "M" in tok else fastener)
    dia = m.group(0) if m else ""
    if dia == "M20":
        a_s, rp = 245.0, 660.0
        f_proof = a_s * rp / 1000.0
        f_m = PRELOAD_FRACTION * f_proof
        t = K_TORQUE * 20.0 * f_m * 1000.0 / 1000.0
        t = round(t, 1)
        rows.append({"joint": joint, "fastener": fastener, "class": cls,
                     "A_s_mm2": a_s, "proof_kN": round(f_proof, 2), "preload_kN": round(f_m, 2),
                     "torque_Nm": t, "tolerance": "+/-10%", "notes": usage})
        continue
    if dia not in AS_MM2:
        continue
    if cls not in RP02:
        rows.append({"joint": joint, "fastener": fastener, "class": cls,
                     "A_s_mm2": "", "proof_kN": "", "preload_kN": "",
                     "torque_Nm": "datasheet", "tolerance": "datasheet", "notes": usage})
        continue
    f_proof, f_m, t = torque_nm(dia, cls)
    rows.append({"joint": joint, "fastener": fastener, "class": cls,
                 "A_s_mm2": AS_MM2[dia], "proof_kN": round(f_proof, 2), "preload_kN": round(f_m, 2),
                 "torque_Nm": round(t, 2), "tolerance": "+/-10%", "notes": (washer + "; " + usage).strip("; ")})

result = {
    "model": "fastener tightening torques (VDI 2230 class)",
    "method": {"K": K_TORQUE, "surface_condition": "lightly oiled / standard",
               "preload_fraction_of_proof": PRELOAD_FRACTION,
               "formula": "F_M = 0.7*A_s*R_p0.2; T = K*d*F_M",
               "property_classes": RP02},
    "joints": rows,
    "warnings": [
        "recompute with the matching K if the released surface condition differs",
        "the ifm M4 flange tap has a manufacturer torque limit - the computed "
        "value is an upper bound, take min(computed, ifm limit) at release",
        "anchor and KM-nut values come from their datasheets, not this table",
        "the adapter FEM models 8.0 kN/screw (~60 % class proof) preload; the "
        "70 % table above is the assembly target unless the joint calculation "
        "says otherwise",
    ],
}
payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
(HERE / "manifest.json").write_text(payload, encoding="utf-8")
with (HERE / "torques.csv").open("w", newline="", encoding="utf-8") as f:
    w = csv.DictWriter(f, fieldnames=["joint", "fastener", "class", "A_s_mm2", "proof_kN",
                                      "preload_kN", "torque_Nm", "tolerance", "notes"])
    w.writeheader()
    for r in rows:
        w.writerow(r)
print(f"{len(rows)} joints computed")
print("manifest sha256[:16] =", hashlib.sha256(payload.encode()).hexdigest()[:16])
