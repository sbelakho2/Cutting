#!/usr/bin/env python3
"""Reject stale or overclaimed published mechanism CAD artifacts."""

from __future__ import annotations

import csv
import json
import math
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = ROOT / "shared" / "mechanism_spec.hpp"
OUTPUT = ROOT / "engineering" / "generated" / "mechanism"

MANIFEST_HEADERS = [
    "id", "description", "purchased", "xmin_mm", "ymin_mm", "zmin_mm",
    "xmax_mm", "ymax_mm", "zmax_mm", "volume_mm3",
]
BOM_HEADERS = ["description", "quantity", "purchased"]

STATIC_PARTS = {
    "GD_G101": ("R2 selected DC01 2mm powder-coated left guard panel;30mm wire pass-through;not a safety-gap design", "0"),
    "GD_G102": ("R2 selected DC01 2mm powder-coated right guard panel;30mm wire pass-through;not a safety-gap design", "0"),
    "GD_G103": ("R2 selected DC01 2mm powder-coated fixed rear guard panel", "0"),
    "GD_G104": ("R2 selected DC01 2mm powder-coated guard roof", "0"),
    "GD_G105": ("R2 selected 5mm polycarbonate closed front door;actuator aperture only;no kinematic or safety validation", "0"),
    "GD_G106": ("R2 selected S235JR 5mm guard-lock mounting plate;body pocket only;SLC drilling pattern requires manufacturer drawing", "0"),
    "GD_G107": ("Bernstein SLC-F-024-20/20-R4 30x192x44 catalogue envelope;static fit only", "1"),
    "GD_G108": ("Bernstein ACF-1 60x40x52.6 catalogue envelope;static fit only", "1"),
    "PO_P001": ("R2 selected welded payoff spool OD400x160;3mm S235JR flanges;left flange carries the welded P008 drive boss;right flange 20.5 pilot bore;60x2.5 ERW hub", "0"),
    "PO_P002": ("R2 selected C45 20h6 payoff spindle L660;M20x1.5 ends not modeled;bearing pockets in P004/P005", "0"),
    "PO_P003": ("R2 selected S235JR payoff base 430x560x8;4x13 anchor holes;selected not load-qualified", "0"),
    "PO_P004": ("R2 selected S235JR 18mm left payoff post;47H7 6204 pocket and 20.5 spindle bore;weld and load path not validated", "0"),
    "PO_P005": ("R2 selected S235JR 18mm right payoff post;47H7 6204 pocket and 20.5 spindle bore;weld and load path not validated", "0"),
    "PO_P006": ("R2 selected S235JR brake disc 180x6;20H7 bore;keyway and setscrew are fabrication details;friction brake not safety brake", "0"),
    "PO_P007": ("R2 selected brake caliper bracket envelope;pads and SWU-class adjuster spring not modeled;tension brake not safety brake", "0"),
    "PO_P008": ("R2 selected keyed spool drive boss 54.6x12;20H7 bore + 6JS9 keyseat;welded to the left flange inside the hub;transmits brake torque;not load-qualified", "0"),
}

EXPECTED_TRUE = {
    "run_complete",
    "cross_subsystem_static_intersections_checked",
    "external_wire_route_checked",
    "step_readback",
    "frame_integrated",
}
EXPECTED_FALSE = {
    "guard_door_kinematics_validated",
    "guard_safety_function_validated",
    "payoff_selection_complete",
    "payoff_rotation_validated",
    "full_machine_integrated",
    "physical_results",
    "fabrication_released",
}
EXPECTED_GROUP_COUNTS = {
    "head": 57,
    "RIGHT_": 181,
    "LEFT_": 181,
    "MM_": 43,
    "MM2_": 43,
    "FR_": 29,
    "GD_": 8,
    "PO_": 8,
}


class OutputError(RuntimeError):
    """A published CAD artifact is missing, stale, or contradicts source scope."""


def read_csv(path: Path, headers: list[str]) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames != headers:
            raise OutputError(f"{path.relative_to(ROOT)} has an unexpected controlled header")
        rows = list(reader)
    for row_number, row in enumerate(rows, start=2):
        if None in row or any(row.get(header) is None for header in headers):
            raise OutputError(f"{path.relative_to(ROOT)} has a missing or extra field in row {row_number}")
    return rows


def finite_number(value: str, label: str) -> float:
    try:
        parsed = float(value)
    except ValueError as error:
        raise OutputError(f"{label} is not numeric: {value!r}") from error
    if not math.isfinite(parsed):
        raise OutputError(f"{label} is not finite: {value!r}")
    return parsed


def reject_duplicate_json_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise OutputError(f"status.json repeats the key {key!r}")
        result[key] = value
    return result


def source_revision() -> str:
    match = re.search(r'inline constexpr std::string_view revision="([^"]+)"', SPEC.read_text(encoding="utf-8"))
    if match is None:
        raise OutputError("shared/mechanism_spec.hpp lacks a parseable mechanism revision")
    return match.group(1)


def verify_status() -> None:
    status_path = OUTPUT / "status.json"
    try:
        status = json.loads(status_path.read_text(encoding="utf-8"), object_pairs_hook=reject_duplicate_json_keys)
    except json.JSONDecodeError as error:
        raise OutputError(f"{status_path.relative_to(ROOT)} is invalid JSON: {error}") from error
    if status.get("mechanism_revision") != source_revision():
        raise OutputError("published mechanism status revision does not match shared/mechanism_spec.hpp")
    if status.get("occurrences") != 550:
        raise OutputError(f"published mechanism occurrence count is {status.get('occurrences')!r}, expected 550")
    for key in EXPECTED_TRUE:
        if status.get(key) is not True:
            raise OutputError(f"published mechanism status must retain {key}:true")
    for key in EXPECTED_FALSE:
        if status.get(key) is not False:
            raise OutputError(f"published mechanism status must retain {key}:false")
    scope = status.get("scope")
    if not isinstance(scope, str) or "R2 guard/payoff selections" not in scope:
        raise OutputError("published mechanism status lacks the R2 guard/payoff scope boundary")


def verify_manifest_and_bom() -> None:
    manifest_path = OUTPUT / "manifest.csv"
    manifest = read_csv(manifest_path, MANIFEST_HEADERS)
    if len(manifest) != 550:
        raise OutputError(f"published manifest has {len(manifest)} rows, expected 550")
    seen_ids: set[str] = set()
    group_counts = dict.fromkeys(EXPECTED_GROUP_COUNTS, 0)
    for row_number, row in enumerate(manifest, start=2):
        item_id = row["id"]
        if not item_id or item_id in seen_ids:
            raise OutputError(f"published manifest has a blank or duplicate ID in row {row_number}: {item_id!r}")
        seen_ids.add(item_id)
        group = "head"
        for prefix in EXPECTED_GROUP_COUNTS:
            if prefix != "head" and item_id.startswith(prefix):
                group = prefix
                break
        group_counts[group] += 1
        if row["purchased"] not in {"0", "1"}:
            raise OutputError(f"published manifest has an invalid purchase state for {item_id}")
        xmin = finite_number(row["xmin_mm"], f"published manifest {item_id} xmin_mm")
        ymin = finite_number(row["ymin_mm"], f"published manifest {item_id} ymin_mm")
        zmin = finite_number(row["zmin_mm"], f"published manifest {item_id} zmin_mm")
        xmax = finite_number(row["xmax_mm"], f"published manifest {item_id} xmax_mm")
        ymax = finite_number(row["ymax_mm"], f"published manifest {item_id} ymax_mm")
        zmax = finite_number(row["zmax_mm"], f"published manifest {item_id} zmax_mm")
        volume = finite_number(row["volume_mm3"], f"published manifest {item_id} volume_mm3")
        if xmin > xmax or ymin > ymax or zmin > zmax or volume <= 0:
            raise OutputError(f"published manifest has invalid bounds or volume for {item_id}")
    if group_counts != EXPECTED_GROUP_COUNTS:
        raise OutputError(f"published manifest group counts drifted: {group_counts}")
    static_rows = {row["id"]: row for row in manifest if row["id"].startswith(("GD_", "PO_"))}
    if set(static_rows) != set(STATIC_PARTS):
        raise OutputError("published manifest guard/payoff IDs are incomplete or contain uncontrolled additions")
    for item_id, (description, purchased) in STATIC_PARTS.items():
        row = static_rows[item_id]
        if row["description"] != description or row["purchased"] != purchased:
            raise OutputError(f"published manifest identity or purchase state drift for {item_id}")

    bom_path = OUTPUT / "mechanism-bom.csv"
    bom = read_csv(bom_path, BOM_HEADERS)
    by_description = {row["description"]: row for row in bom}
    if len(by_description) != len(bom):
        raise OutputError("published mechanism BOM has a duplicate description")
    for row_number, row in enumerate(bom, start=2):
        if not row["description"] or row["purchased"] not in {"0", "1"}:
            raise OutputError(f"published mechanism BOM has an invalid identity or purchase state in row {row_number}")
        try:
            quantity = int(row["quantity"])
        except ValueError as error:
            raise OutputError(f"published mechanism BOM quantity is invalid in row {row_number}") from error
        if quantity <= 0:
            raise OutputError(f"published mechanism BOM quantity is not positive in row {row_number}")
    for item_id, (description, purchased) in STATIC_PARTS.items():
        row = by_description.get(description)
        if row is None or row["quantity"] != "1" or row["purchased"] != purchased:
            raise OutputError(f"published mechanism BOM identity, quantity, or purchase state drift for {item_id}")


def verify_step() -> None:
    step_path = OUTPUT / "full-machine.step"
    if not step_path.is_file() or step_path.stat().st_size == 0:
        raise OutputError("published full-machine STEP is absent or empty")


def verify_stock_containment() -> None:
    manifest = read_csv(OUTPUT / "manifest.csv", ["id", "description", "purchased", "xmin_mm", "ymin_mm", "zmin_mm", "xmax_mm", "ymax_mm", "zmax_mm", "volume_mm3"])
    bounds: dict[str, tuple[float, float, float]] = {}
    for row in manifest:
        part_id = row["id"].split("_")[-1] if "_" in row["id"] else row["id"]
        extents = (
            float(row["xmax_mm"]) - float(row["xmin_mm"]),
            float(row["ymax_mm"]) - float(row["ymin_mm"]),
            float(row["zmax_mm"]) - float(row["zmin_mm"]),
        )
        current = bounds.get(part_id)
        merged = tuple(max(a, b) for a, b in zip(current, extents)) if current else extents
        bounds[part_id] = merged
    custom = read_csv(ROOT / "bom" / "mechanism-custom-parts.csv", ["id", "quantity", "description", "material", "nominal_stock", "manufacturing_record", "release_state"])
    for row in custom:
        stock = row["nominal_stock"]
        match = re.fullmatch(r"(\d+(?:\.\d+)?)x(\d+(?:\.\d+)?)x(\d+(?:\.\d+)?)", stock)
        if not match:
            continue
        stock_dims = sorted(float(g) for g in match.groups())
        part_id = row["id"]
        if part_id not in bounds:
            continue
        need = sorted(bounds[part_id])
        if any(s < n for s, n in zip(stock_dims, need)):
            raise OutputError(
                f"stock containment failure: {part_id} stock {stock} cannot contain CAD extent "
                f"{bounds[part_id][0]:.2f}x{bounds[part_id][1]:.2f}x{bounds[part_id][2]:.2f}"
            )


def main() -> int:
    try:
        verify_status()
        verify_manifest_and_bom()
        verify_stock_containment()
        verify_step()
        print("mechanism generated-output integrity passed: R4, 550 occurrences, 15 guard/payoff parts, stock containment, two measuring stations, unreleased")
    except (OSError, csv.Error, OutputError) as error:
        print(f"mechanism generated-output integrity failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())