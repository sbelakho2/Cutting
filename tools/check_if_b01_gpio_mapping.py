#!/usr/bin/env python3
"""Verify IF-B01 connector allocation against controller, cabinet, and RP2350 sources."""

from __future__ import annotations

import csv
import json
import re
import sys
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ALLOCATION = ROOT / "hardware" / "if-b01" / "if-b01-gpio-allocation.csv"
NETLIST = ROOT / "hardware" / "if-b01" / "if-b01-netlist.csv"
INTENT = ROOT / "hardware" / "if-b01" / "if-b01-native-intent.json"
CONTROLS = ROOT / "shared" / "controls_spec.hpp"
CABINET = ROOT / "shared" / "cabinet_controls_spec.hpp"
FIRMWARE = ROOT / "software" / "machine" / "rp2350" / "board_rp2350.cpp"

HEADERS = [
    "allocation_id", "category", "pin_symbol", "gpio", "connector", "pin", "net",
    "firmware_signal", "receiver_ref", "receiver_input_pin", "receiver_output_pin",
    "cabinet_relay", "asserted_state", "safety_boundary",
]


class MappingError(RuntimeError):
    """The controller-to-board allocation has drifted."""


def read_csv(path: Path, headers: list[str]) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames != headers:
            raise MappingError(f"{path.relative_to(ROOT)} does not have the controlled header")
        rows = list(reader)
    if not rows:
        raise MappingError(f"{path.relative_to(ROOT)} has no allocation rows")
    return rows


def parse_controls_pins(text: str) -> dict[str, int]:
    pins: dict[str, int] = {}
    for declaration in re.findall(r"inline\s+constexpr\s+std::uint8_t\s+([^;]+);", text):
        for assignment in declaration.split(","):
            match = re.fullmatch(r"\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(\d+)\s*", assignment)
            if match is None:
                raise MappingError(f"unparseable controller pin declaration: {assignment!r}")
            symbol, value = match.groups()
            if symbol in pins:
                raise MappingError(f"duplicate controller pin symbol {symbol}")
            pins[symbol] = int(value)
    if not pins:
        raise MappingError("no std::uint8_t controller pin declarations found")
    return pins


def initializer(text: str, name: str) -> list[str]:
    match = re.search(rf"{re.escape(name)}\s*\[[^\]]*\]\s*=\s*\{{(.*?)\}};", text, re.DOTALL)
    if match is None:
        raise MappingError(f"firmware initializer {name} not found")
    return re.findall(r"ctrl::pin::([A-Za-z_][A-Za-z0-9_]*)", match.group(1))


def verify_unique_rows(rows: list[dict[str, str]]) -> None:
    ids: set[str] = set()
    connector_pins: set[tuple[str, str]] = set()
    by_symbol: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        allocation_id = row["allocation_id"]
        key = row["connector"], row["pin"]
        if not allocation_id or allocation_id in ids:
            raise MappingError(f"blank or duplicate allocation id {allocation_id!r}")
        if key in connector_pins:
            raise MappingError(f"duplicate board connector assignment {key[0]}-{key[1]}")
        if row["safety_boundary"] == "safety_function":
            raise MappingError(f"{allocation_id} falsely assigns IF-B01 a safety function")
        try:
            gpio = int(row["gpio"])
        except ValueError as error:
            raise MappingError(f"{allocation_id} GPIO is not numeric") from error
        if gpio < 0 or gpio > 28 or row["firmware_signal"] != f"GP{gpio}":
            raise MappingError(f"{allocation_id} has an invalid GPIO/signal pair")
        ids.add(allocation_id)
        connector_pins.add(key)
        by_symbol[row["pin_symbol"]].append(row)
    for symbol, entries in by_symbol.items():
        shared = {(entry["gpio"], entry["net"]) for entry in entries}
        if len(shared) != 1:
            raise MappingError(f"pin symbol {symbol} has incompatible shared assignments")
        if len(entries) > 1 and symbol != "out_drv_enable":
            raise MappingError(f"pin symbol {symbol} is duplicated without a declared shared channel")
    enables = by_symbol.get("out_drv_enable", [])
    if len(enables) != 4 or {entry["net"] for entry in enables} != {"DRV_ENA_NEG"}:
        raise MappingError("shared drive-enable channel must cover four DRV_ENA_NEG pins")


def verify_source_rows(rows: list[dict[str, str]]) -> None:
    pins = parse_controls_pins(CONTROLS.read_text(encoding="utf-8"))
    netlist_rows = read_csv(
        NETLIST,
        ["connector", "pin", "net", "external_connection", "firmware_signal", "notes"],
    )
    netlist = {(row["connector"], row["pin"]): row for row in netlist_rows}
    intent = json.loads(INTENT.read_text(encoding="utf-8"))
    connector_intent = intent["connector_net_expectations"]
    pin_intent = intent["component_pin_net_expectations"]
    cabinet = CABINET.read_text(encoding="utf-8")

    for row in rows:
        allocation_id = row["allocation_id"]
        symbol = row["pin_symbol"]
        gpio = int(row["gpio"])
        if pins.get(symbol) != gpio:
            raise MappingError(f"{allocation_id}: controls_spec maps {symbol} to {pins.get(symbol)!r}, not GP{gpio}")
        key = row["connector"], row["pin"]
        source_row = netlist.get(key)
        if source_row is None or source_row["net"] != row["net"]:
            raise MappingError(f"{allocation_id}: connector netlist drift at {key[0]}-{key[1]}")
        expected_intent = connector_intent.get(key[0], {}).get(key[1])
        if expected_intent != row["net"]:
            raise MappingError(f"{allocation_id}: native intent drift at {key[0]}-{key[1]}")
        if row["category"] not in {"encoder_a", "encoder_b"}:
            if source_row["firmware_signal"] != row["firmware_signal"]:
                raise MappingError(f"{allocation_id}: CSV firmware-signal drift")
        else:
            receiver = pin_intent.get(row["receiver_ref"], {})
            if receiver.get(row["receiver_input_pin"]) != row["net"]:
                raise MappingError(f"{allocation_id}: receiver input net drift")
            if receiver.get(row["receiver_output_pin"]) != row["firmware_signal"]:
                raise MappingError(f"{allocation_id}: receiver output GPIO drift")
        relay = row["cabinet_relay"]
        if relay:
            return_pin = str(int(row["pin"]) + 1)
            expected = f'{{"{relay}", "{row["connector"]}-{row["pin"]}", "{row["connector"]}-{return_pin}", "{row["firmware_signal"]}"'
            if expected not in cabinet:
                raise MappingError(f"{allocation_id}: cabinet relay allocation drift")


def verify_firmware_use(rows: list[dict[str, str]]) -> None:
    firmware = FIRMWARE.read_text(encoding="utf-8")
    by_category: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        by_category[row["category"]].append(row)
        token = f"ctrl::pin::{row['pin_symbol']}"
        if row["category"] != "encoder_b" and token not in firmware:
            raise MappingError(f"firmware does not use {token}")

    expected_pulses = [row["pin_symbol"] for row in by_category["motion_pulse"]]
    expected_directions = [row["pin_symbol"] for row in by_category["motion_direction"]]
    if initializer(firmware, "step_pul_pin") != expected_pulses:
        raise MappingError("firmware pulse-PIO order does not match allocation")
    if initializer(firmware, "step_dir_pin") != expected_directions:
        raise MappingError("firmware direction-PIO order does not match allocation")
    expected_encoder_a = [row["pin_symbol"] for row in by_category["encoder_a"]]
    if initializer(firmware, "encoder_a_pin") != expected_encoder_a:
        raise MappingError("firmware encoder-A PIO order does not match allocation")
    for row in by_category["encoder_b"]:
        a_symbol = row["pin_symbol"].replace("_b", "_a")
        a_row = next(item for item in rows if item["pin_symbol"] == a_symbol)
        if int(row["gpio"]) != int(a_row["gpio"]) + 1:
            raise MappingError(f"{row['allocation_id']}: encoder B must be adjacent to encoder A")
    contact_symbols = {row["pin_symbol"] for row in by_category["field_input"]}
    alarm_symbols = {row["pin_symbol"] for row in by_category["alarm_input"]}
    if not contact_symbols.issubset(set(initializer(firmware, "contact_inputs"))):
        raise MappingError("firmware contact-input initialization is incomplete")
    if set(initializer(firmware, "alarm_inputs")) != alarm_symbols:
        raise MappingError("firmware alarm-input initialization does not match allocation")
    indicator_symbols = {row["pin_symbol"] for row in by_category["indicator_output"]}
    if set(initializer(firmware, "outputs")) != indicator_symbols:
        raise MappingError("firmware indicator-output initialization does not match allocation")
    if "gpio_set_irq_enabled_with_callback(ctrl::pin::in_safety_ok" not in firmware:
        raise MappingError("firmware does not retain the safety diagnostic IRQ source")
    if "gpio_put(ctrl::pin::out_drv_enable, 0);" not in firmware:
        raise MappingError("firmware does not retain the drive-enable low assertion")


def main() -> int:
    try:
        rows = read_csv(ALLOCATION, HEADERS)
        verify_unique_rows(rows)
        verify_source_rows(rows)
        verify_firmware_use(rows)
        print(f"IF-B01/RP2350 GPIO allocation passed: {len(rows)} connector assignments")
    except (OSError, csv.Error, json.JSONDecodeError, MappingError, StopIteration) as error:
        print(f"IF-B01/RP2350 GPIO allocation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())