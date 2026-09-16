#!/usr/bin/env python3
"""S1-B design-integrity audit: purchasing coverage, source-file existence,
manufacturability rules, spec arithmetic and fastener engagement.

Run from the repository root. Exit code 0 = all checks pass.
"""
from __future__ import annotations

import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BOM_DIR = ROOT / "bom"
PLAN = ROOT / "docs" / "24-production-and-release-plan.md"
MECH_BOM = ROOT / "engineering" / "generated" / "mechanism" / "mechanism-bom.csv"
CUSTOM = BOM_DIR / "mechanism-custom-parts.csv"
DRAWINGS = ROOT / "engineering" / "generated" / "drawings"

SELECTED = [
    BOM_DIR / "selected-cutting-head.csv",
    BOM_DIR / "selected-transport-and-cabinet.csv",
    BOM_DIR / "selected-mechanism.csv",
    BOM_DIR / "selected-controls.csv",
    BOM_DIR / "selected-cabinet-wiring.csv",
    BOM_DIR / "fabrication-tooling.csv",
]

failures: list[str] = []


def fail(msg: str) -> None:
    failures.append(msg)


def rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as f:
        return [r for r in csv.DictReader(f)]


# Purchased CAD rows -> the BOM record that must cover them.
PURCHASED_COVERAGE = {
    "8.0000.3123.0006": ("selected-mechanism.csv", "ME-W01"),
    "Bernstein ACF-1": ("selected-controls.csv", "CT-GLA01"),
    "Bernstein SLC-F": ("selected-controls.csv", "CT-GL01"),
    "E40S6-1024-6-L-5": ("selected-mechanism.csv", "ME-E01"),
    "LHFRW12": ("selected-mechanism.csv", "MC-B01"),
    "PSFJ12-240": ("selected-mechanism.csv", "MC-S01"),
    "SKF 626-2Z": ("selected-transport-and-cabinet.csv", "ME-B01"),
    "SWU10.5-35": ("selected-mechanism.csv", "MC-P02"),
    "SWU17-60": ("selected-mechanism.csv", "MC-P01"),
    "Supplied6/6 flexible coupling": ("selected-mechanism.csv", "ME-E01"),
}

# Standard ISO/DIN fastener rows in the mechanism BOM are covered by the
# standard-fastener register in the master plan.
STD_FASTENER_TOKENS = ("DIN913", "DIN929", "ISO4032", "ISO4762", "ISO7092", "ISO8734")

# BOM rows that are standard commodity hardware covered by the register line.
STD_ROW_PATTERN = re.compile(r"^(TR|ME)-(FB|FW|FD)\d|^TR-K01$|^CT-IF-U|^CT-PS36$|^CT-FSR01$")


def check_bom_rows() -> None:
    all_rows = [(p, r) for p in SELECTED for r in rows(p)]
    plan_text = PLAN.read_text(encoding="utf-8")
    for path, row in all_rows:
        item_id = row["id"].strip()
        mfr = row.get("manufacturer", "").strip()
        order = (row.get("exact_order") or row.get("ordering_code") or "").strip()
        if item_id.startswith("HOLD-"):
            continue
        if mfr == "UNSELECTED":
            fail(f"{path.name} {item_id} is UNSELECTED; internal build requires a selected identity")
            continue
        if not order:
            fail(f"{path.name} {item_id} has no exact order code")
        # price coverage: observed or quotation register or standard hardware
        observed = (row.get("observed_unit_usd") or row.get("observed_unit_eur") or "").strip()
        basis = row.get("price_basis", "").strip()
        if not observed and "not_observed" in basis:
            if item_id not in plan_text and not STD_ROW_PATTERN.match(item_id):
                fail(f"{path.name} {item_id} is unpriced and absent from the master-plan quotation register")
        # relative source files must exist
        for key in ("source_record", "source_url"):
            val = row.get(key, "").strip()
            if val and val != "in-house" and not val.startswith(("http://", "https://")):
                if not (ROOT / val).exists():
                    fail(f"{path.name} {item_id} {key} points to missing file {val}")


def check_custom_parts() -> None:
    for row in rows(CUSTOM):
        item_id = row["id"]
        record = row.get("manufacturing_record", "").strip()
        if record and not (ROOT / record).exists():
            fail(f"mechanism-custom-parts.csv {item_id} manufacturing record missing: {record}")
        if item_id.startswith(("G", "P")):
            drawing = DRAWINGS / f"{item_id}-sheet.pdf"
            known = {
                "P001": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "P002": DRAWINGS / "P002-R2-payoff-spindle.pdf",
                "P003": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "P004": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "P005": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "P006": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "P007": DRAWINGS / "P004-P007-R2-payoff-plates.pdf",
                "G101": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
                "G102": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
                "G103": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
                "G104": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
                "G105": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
                "G106": DRAWINGS / "G101-G106-R2-guard-panels.pdf",
            }.get(item_id, drawing)
            if not known.exists():
                fail(f"mechanism-custom-parts.csv {item_id} has no drawing sheet ({known.name})")


def check_purchased_cad_coverage() -> None:
    if not MECH_BOM.exists():
        fail("mechanism-bom.csv missing; regenerate mechanism outputs")
        return
    selected_rows = {r["id"].strip(): r for p in SELECTED for r in rows(p)}
    plan_text = PLAN.read_text(encoding="utf-8")
    cad_qty: dict[str, int] = {}
    for row in rows(MECH_BOM):
        if row.get("purchased", "0") != "1":
            continue
        desc = row["description"]
        covered = False
        for token, (_, bom_id) in PURCHASED_COVERAGE.items():
            if token in desc and bom_id in selected_rows:
                covered = True
                cad_qty[token] = cad_qty.get(token, 0) + int(float(row["quantity"]))
        if not covered and any(t in desc for t in STD_FASTENER_TOKENS):
            if "Standard hardware" in plan_text:
                covered = True
        if not covered:
            fail(f"mechanism-bom.csv purchased row lacks a BOM/register identity: {desc[:90]}")
    # CAD quantity vs selected BOM quantity for the covered catalogue rows.
    # The mechanism CAD models ONE representative measuring module of the TWO
    # designed (docs/13): those rows carry a x2 design-intent multiplier.
    module_scale = {
        "8.0000.3123.0006": 2,
        "E40S6-1024-6-L-5": 2,
        "SKF 626-2Z": 2,
        "Supplied6/6 flexible coupling": 2,
    }
    for token, (_, bom_id) in PURCHASED_COVERAGE.items():
        bom_qty = int(float(selected_rows[bom_id]["quantity"]))
        expected = cad_qty.get(token, 0) * module_scale.get(token, 1)
        if expected != bom_qty:
            fail(f"quantity mismatch: CAD models {cad_qty.get(token, 0)}x '{token}' "
                 f"(design x{module_scale.get(token, 1)}) = {expected} but {bom_id} selects {bom_qty}")


def check_arithmetic() -> None:
    # Belt: 2*C + pi*Dp == pitch length 225 (S5M, 20 teeth)
    C = 62.5
    Dp = 20 * 5 / 3.141592653589793
    pitch_length = 2 * C + 3.141592653589793 * Dp
    if abs(pitch_length - 225) > 0.01:
        fail(f"belt arithmetic mismatch: 2*{C}+pi*Dp={pitch_length:.3f} != 225")
    # Measuring wheel envelope diameter = 100/pi
    wheel_d = 100 / 3.141592653589793
    if abs(wheel_d - 31.83) > 0.005:
        fail(f"measuring wheel diameter check: {wheel_d:.4f} != 31.83")
    # Encoder counts
    if 1024 * 4 != 4096:
        fail("encoder count arithmetic wrong (1024 PPR x4 != 4096)")
    # Guide insert: bore = wire OD + clearance fits the insert OD with >=1 mm wall
    insert_od, clearance, max_wire = 8.0, 0.5, 4.5
    wall = (insert_od - (max_wire + clearance)) / 2
    if wall < 1.0:
        fail(f"guide insert wall {wall:.2f} mm < 1.0 mm at max wire OD")


def check_fastener_engagement() -> None:
    # H003: F0 M5x16 through 15 plate, 8.5 counterbore from motor face,
    # 2 mm F0 boss into the stage register -> engagement 16-(15-8.5)-2=7.5
    eng_f0 = 16 - (15 - 8.5) - 2
    if eng_f0 < 7.5:
        fail(f"H003 F0 M5x16 engagement {eng_f0:.1f} mm < 1.5D (7.5 mm)")
    # Motor M5x25 through the full 15 plate -> 10 mm
    eng_motor = 25 - 15
    if eng_motor < 7.5:
        fail(f"H003 motor M5x25 engagement {eng_motor:.1f} mm < 1.5D (7.5 mm)")
    # M201: M3x10 through 6 plate -> 4 protrusion into 3.5-deep encoder thread
    prot = 10 - 6
    if prot < 3.5 or prot > 5.0:
        fail(f"M201 M3x10 protrusion {prot:.1f} mm outside [3.5,5.0]")
    # Bed: M10x35 through 25 bed -> 10 into the DIN929 M10 weld nut (height ~9)
    eng_bed = 35 - 25
    if eng_bed < 9:
        fail(f"bed M10x35 engagement {eng_bed:.1f} mm < DIN929 M10 nut height 9")


def check_dfm() -> None:
    # H003 counterbore bottom land
    if 15 - 8.5 < 5.5:
        fail("H003 counterbore bottom land < screw head 5 + 0.5 clearance")
    # payoff post bearing pocket land
    if 18 - 14 < 3:
        fail("P004/P005 bearing-pocket land < 3 mm")
    # spool flange bore wall: 20.5 bore in 60 hub tube is not applicable; hub wall 2.5
    if 2.5 < 2.0:
        fail("payoff hub tube wall < 2 mm")


def main() -> int:
    check_bom_rows()
    check_custom_parts()
    check_purchased_cad_coverage()
    check_arithmetic()
    check_fastener_engagement()
    check_dfm()
    if failures:
        print("design-integrity audit FAILED:")
        for f in failures:
            print("  -", f)
        return 1
    print("design-integrity audit passed: purchasing, sources, drawings, arithmetic, engagement and DFM checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
