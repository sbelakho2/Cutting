#!/usr/bin/env python3
"""Reject incomplete, contradictory, or overclaimed S1-B production records."""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parents[1]
CONTROLS_BOM = ROOT / "bom" / "selected-controls.csv"
SOURCING = ROOT / "bom" / "sourcing-evidence.csv"
PLAN = ROOT / "docs" / "24-production-and-release-plan.md"
MECHANISM_CUSTOM_BOM = ROOT / "bom" / "mechanism-custom-parts.csv"

SOURCE_HEADERS = [
    "evidence_id", "bom_record", "item_id", "manufacturer", "exact_order", "quantity",
    "source_url", "source_kind", "evidence_status", "observed_unit_price", "currency",
    "price_basis", "reference_price", "reference_basis", "availability_observed",
    "observation_date", "comparison_status", "procurement_status", "next_required_evidence",
]

REQUIRED_DIRECT = {
    "CT-MCU01": {
        "exact_order": "Raspberry Pi Pico 2 SC1631",
        "source_url": "https://www.raspberrypi.com/products/raspberry-pi-pico-2/",
        "observed_unit_price": "5.00",
        "currency": "USD",
        "comparison_status": "not_established_single_observation",
    },
    "CT-SR01": {
        "exact_order": "Harmony XPSUAF13AP",
        "source_url": "https://www.automation24.com/safety-relay-schneider-electric-harmony-xpsuaf13ap",
        "observed_unit_price": "133.50",
        "currency": "EUR",
        "comparison_status": "below_displayed_rrp_not_delivered_comparison",
    },
    "CT-ES01": {
        "exact_order": "SIRIUS ACT 3SU1851-0NB00-2AC2",
        "source_url": "https://www.automation24.com/emergency-stop-off-button-complete-unit-siemens-sirius-act-3su1851-0nb00-2ac2",
        "observed_unit_price": "110.70",
        "currency": "EUR",
        "comparison_status": "below_displayed_rrp_not_delivered_comparison",
    },
    "CT-GL01": {
        "exact_order": "SLC-F-024-20/20-R4 article 6018200007",
        "source_url": "https://www.automation24.com/mechanical-switch-with-guard-locking-bernstein-slc-f-024-20-20-r4-6018200007",
        "observed_unit_price": "143.50",
        "currency": "EUR",
        "comparison_status": "not_established_single_observation",
    },
    "CT-GLA01": {
        "exact_order": "ACF-1 article 3911742391",
        "source_url": "https://www.automation24.com/actuator-bernstein-acf-1-3911742391",
        "observed_unit_price": "17.90",
        "currency": "EUR",
        "comparison_status": "not_established_single_observation",
    },
}

CSV_SCHEMAS = {
    ROOT / "bom" / "selected-controls.csv": {
        "id", "quantity", "manufacturer", "exact_order", "source_url", "observed_unit",
        "price_basis", "checked_date", "release_status", "interface_or_hold",
    },
    ROOT / "bom" / "selected-cabinet-wiring.csv": {
        "id", "quantity", "manufacturer", "exact_order", "source_record", "release_status", "function_or_hold",
    },
    ROOT / "bom" / "selected-cutting-head.csv": {
        "id", "quantity", "manufacturer", "exact_order", "source", "status", "observed_unit_usd", "price_basis", "interface",
    },
    ROOT / "bom" / "selected-transport-and-cabinet.csv": {
        "id", "quantity", "manufacturer", "exact_order", "source_url", "observed_unit_usd", "price_basis", "interface",
    },
    ROOT / "bom" / "selected-mechanism.csv": {
        "id", "quantity", "manufacturer", "exact_order", "source_url", "observed_unit_eur", "price_basis", "checked_date", "interface", "geometry_ids",
    },
    MECHANISM_CUSTOM_BOM: {
        "id", "quantity", "description", "material", "nominal_stock", "manufacturing_record", "release_state",
    },
}

TEMPLATES = {
    ROOT / "docs" / "inspection" / "production-build-record-template.csv": [
        "build_id", "configuration_revision", "work_package", "record_id", "operator", "reviewer",
        "started_utc", "completed_utc", "input_identity", "acceptance_criterion", "actual_result",
        "disposition", "nonconformance_id",
    ],
    ROOT / "docs" / "inspection" / "incoming-inspection-template.csv": [
        "receipt_id", "build_id", "bom_item_id", "manufacturer", "exact_order", "quantity_received",
        "lot_or_serial", "certificate_or_cofc", "inspection_method", "acceptance_criterion", "actual_result",
        "inspector", "date_utc", "disposition", "nonconformance_id",
    ],
    ROOT / "docs" / "inspection" / "first-article-and-energisation-gate-template.csv": [
        "build_id", "gate_id", "required_evidence_record", "requirement", "actual_result", "reviewer",
        "date_utc", "disposition", "nonconformance_id",
    ],
    ROOT / "docs" / "inspection" / "guard-payoff-release-template.csv": [
        "build_id", "check_id", "source_or_instrument", "required_actual_evidence", "actual_result",
        "reviewer", "date_utc", "disposition", "nonconformance_id",
    ],
}


class RecordError(RuntimeError):
    """A controlled source record is absent, malformed, or inconsistent."""


def read_csv(path: Path, expected_headers: set[str] | list[str]) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        actual = reader.fieldnames
        if actual is None:
            raise RecordError(f"{path.relative_to(ROOT)} has no header")
        if isinstance(expected_headers, list):
            if actual != expected_headers:
                raise RecordError(f"{path.relative_to(ROOT)} header is not the controlled template header")
        elif set(actual) != expected_headers:
            raise RecordError(f"{path.relative_to(ROOT)} headers do not match the controlled schema")
        rows = list(reader)
    if not rows:
        raise RecordError(f"{path.relative_to(ROOT)} has no data/template row")
    return rows


def require_http_url(value: str, label: str) -> None:
    parsed = urlparse(value)
    if parsed.scheme not in {"http", "https"} or not parsed.netloc:
        raise RecordError(f"{label} is not an absolute HTTP(S) URL: {value!r}")


def verify_bom_schemas() -> tuple[dict[str, dict[str, str]], int]:
    controls: dict[str, dict[str, str]] = {}
    total = 0
    for path, headers in CSV_SCHEMAS.items():
        rows = read_csv(path, headers)
        ids: set[str] = set()
        for row in rows:
            item_id = row["id"].strip()
            if not item_id or item_id in ids:
                raise RecordError(f"{path.relative_to(ROOT)} has a blank or duplicate id {item_id!r}")
            ids.add(item_id)
            source_url = row.get("source_url", "").strip()
            if source_url and source_url != "in-house":
                require_http_url(source_url, f"{path.relative_to(ROOT)} {item_id} source_url")
            if item_id.startswith("HOLD-") or row.get("manufacturer") == "UNSELECTED":
                if row.get("release_status") != "release_hold_not_for_procurement":
                    raise RecordError(f"{path.relative_to(ROOT)} {item_id} lost its non-procurement release hold")
            if path == CONTROLS_BOM:
                controls[item_id] = row
        total += len(rows)
    return controls, total


def number(value: str, label: str) -> float:
    try:
        return float(value)
    except ValueError as error:
        raise RecordError(f"{label} is not numeric: {value!r}") from error


def verify_sourcing(controls: dict[str, dict[str, str]]) -> int:
    rows = read_csv(SOURCING, SOURCE_HEADERS)
    seen_ids: set[str] = set()
    by_item: dict[str, dict[str, str]] = {}
    allowed_statuses = {
        "not_established_single_observation",
        "not_established_price_not_observed",
        "below_displayed_rrp_not_delivered_comparison",
    }
    for row in rows:
        evidence_id = row["evidence_id"].strip()
        item_id = row["item_id"].strip()
        if not evidence_id or evidence_id in seen_ids:
            raise RecordError(f"sourcing evidence has a blank or duplicate evidence_id {evidence_id!r}")
        if item_id not in controls:
            raise RecordError(f"sourcing evidence references unknown controls item {item_id!r}")
        if item_id in by_item:
            raise RecordError(f"sourcing evidence has duplicate controls item {item_id!r}")
        seen_ids.add(evidence_id)
        by_item[item_id] = row
        controls_row = controls[item_id]
        if row["manufacturer"] != controls_row["manufacturer"] or row["exact_order"] != controls_row["exact_order"]:
            raise RecordError(f"sourcing evidence identity drift for {item_id}")
        if row["observed_unit_price"].strip():
            expected_observed_unit = f"{row['currency']}{row['observed_unit_price']}"
            if controls_row["observed_unit"] != expected_observed_unit:
                raise RecordError(f"sourcing evidence price drift for {item_id}")
        require_http_url(row["source_url"], f"sourcing evidence {item_id}")
        if not row["next_required_evidence"].strip():
            raise RecordError(f"sourcing evidence {item_id} lacks a next-evidence action")
        if row["comparison_status"] not in allowed_statuses:
            raise RecordError(f"sourcing evidence {item_id} has unsupported comparison status")
        if row["comparison_status"] == "below_displayed_rrp_not_delivered_comparison":
            observed = number(row["observed_unit_price"], f"{item_id} observed unit price")
            reference = number(row["reference_price"], f"{item_id} displayed RRP")
            if observed >= reference:
                raise RecordError(f"sourcing evidence {item_id} claims below-RRP without a lower observed price")
        elif row["reference_price"].strip():
            raise RecordError(f"sourcing evidence {item_id} has a reference price without a controlled comparison status")

    for item_id, expected in REQUIRED_DIRECT.items():
        row = by_item.get(item_id)
        if row is None:
            raise RecordError(f"missing direct-page sourcing evidence for {item_id}")
        if row["evidence_status"] != "direct_page_checked":
            raise RecordError(f"{item_id} is not marked as direct-page evidence")
        for key, value in expected.items():
            if row[key] != value:
                raise RecordError(f"{item_id} expected {key}={value!r}, got {row[key]!r}")
    return len(rows)


def verify_templates() -> int:
    for path, header in TEMPLATES.items():
        rows = read_csv(path, header)
        if any(any((value or "").strip() for value in row.values()) for row in rows):
            raise RecordError(f"{path.relative_to(ROOT)} must remain blank as a template")
    return len(TEMPLATES)


def verify_guard_payoff_custom_parts() -> None:
    rows = read_csv(MECHANISM_CUSTOM_BOM, CSV_SCHEMAS[MECHANISM_CUSTOM_BOM])
    required_descriptions = {
        "G101": "preliminary_left_guard_panel5mm",
        "G102": "preliminary_right_guard_panel5mm",
        "G103": "preliminary_fixed_rear_guard_panel5mm",
        "G104": "preliminary_guard_roof5mm",
        "G105": "preliminary_closed_front_door5mm",
        "G106": "preliminary_guard_lock_mounting_plate",
        "P001": "provisional_payoff_spool_envelope_OD400x160",
        "P002": "provisional_payoff_spindle_envelope39mm",
        "P003": "provisional_payoff_base_envelope430x560x8",
        "P004": "provisional_payoff_left_support_post_envelope60x70",
        "P005": "provisional_payoff_right_support_post_envelope60x70",
        "P006": "payoff_brake_disc_180x6",
        "P007": "payoff_brake_caliper",
    }
    by_id = {row["id"]: row for row in rows}
    concept_ids = {item_id for item_id in by_id if item_id.startswith("G") or item_id.startswith("P")}
    if concept_ids != set(required_descriptions):
        raise RecordError("mechanism custom BOM guard/payoff IDs are incomplete or include uncontrolled additions")
    for item_id, description in required_descriptions.items():
        row = by_id[item_id]
        if row["quantity"] != "1" or description not in row["description"]:
            raise RecordError(f"mechanism custom BOM {item_id} identity or quantity drift")
        if row["material"] == "UNSELECTED" or row["nominal_stock"] == "concept_only":
            raise RecordError(f"mechanism custom BOM {item_id} lost its R2 selected design state")
        if row["manufacturing_record"] != "docs/26-guard-and-payoff-selection.md":
            raise RecordError(f"mechanism custom BOM {item_id} lost its guard/payoff governing record")
        if row["release_state"] != "selected_design_not_fabricated":
            raise RecordError(f"mechanism custom BOM {item_id} lost its non-fabrication release state")


def verify_plan_links() -> None:
    text = PLAN.read_text(encoding="utf-8")
    normalized = re.sub(r"\s+", " ", text)
    required_phrases = (
        "source-controlled build plan only",
        "A `HOLD-` or `UNSELECTED` row is not an order line",
        "No such landed-cost comparison exists",
        "not safety functions",
        "No row in this table may be closed by relaxing",
    )
    for phrase in required_phrases:
        if phrase not in normalized:
            raise RecordError(f"production plan is missing required boundary: {phrase!r}")
    for target in re.findall(r"\]\(([^)]+)\)", text):
        target = target.split("#", 1)[0]
        if not target or "://" in target or target.startswith("mailto:"):
            continue
        if not (PLAN.parent / target).exists():
            raise RecordError(f"production plan has a broken local link: {target}")


def main() -> int:
    try:
        controls, bom_rows = verify_bom_schemas()
        sourcing_rows = verify_sourcing(controls)
        template_count = verify_templates()
        verify_guard_payoff_custom_parts()
        verify_plan_links()
        print(
            "production record integrity passed: "
            f"{bom_rows} BOM rows, {sourcing_rows} sourcing records, {template_count} blank controlled templates"
        )
    except (OSError, csv.Error, RecordError) as error:
        print(f"production record integrity failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())