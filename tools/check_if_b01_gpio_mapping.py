#!/usr/bin/env python3
"""Verify IF-B01 connector allocation against controller, cabinet, and RP2350 sources."""

from __future__ import annotations

import csv
import json
import math
import re
import sys
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_if_b01_kicad import child, children, parse_sexp  # noqa: E402

ALLOCATION = ROOT / "hardware" / "if-b01" / "if-b01-gpio-allocation.csv"
NETLIST = ROOT / "hardware" / "if-b01" / "if-b01-netlist.csv"
INTENT = ROOT / "hardware" / "if-b01" / "if-b01-native-intent.json"
CONTROLS = ROOT / "shared" / "controls_spec.hpp"
CABINET = ROOT / "shared" / "cabinet_controls_spec.hpp"
FIRMWARE = ROOT / "software" / "machine" / "rp2350" / "board_rp2350.cpp"
PCB = ROOT / "hardware" / "if-b01" / "if-b01.kicad_pcb"
SCHEMATIC = ROOT / "hardware" / "if-b01" / "if-b01.kicad_sch"

HEADERS = [
    "allocation_id", "category", "pin_symbol", "gpio", "connector", "pin", "net",
    "firmware_signal", "receiver_ref", "receiver_input_pin", "receiver_output_pin",
    "cabinet_relay", "asserted_state", "safety_boundary",
]

# Official Raspberry Pi Pico 2 (SC1631) physical pinout.  Left row (physical pins
# 1-20) runs top to bottom; right row (physical pins 21-40) runs GP16 at the
# bottom to VBUS at the top.  Every physical pin carries exactly one sheet net or
# is deliberately unused on IF-B01.
PICO2_PHYSICAL_PIN_FUNCTION: dict[int, tuple[str, str | None]] = {
    1: ("GP0", "GP0"), 2: ("GP1", "GP1"), 3: ("GND", "0V_PELV"), 4: ("GP2", "GP2"),
    5: ("GP3", "GP3"), 6: ("GP4", "GP4"), 7: ("GP5", "GP5"), 8: ("GND", "0V_PELV"),
    9: ("GP6", "GP6"), 10: ("GP7", "GP7"), 11: ("GP8", "GP8"), 12: ("GP9", "GP9"),
    13: ("GND", "0V_PELV"), 14: ("GP10", "GP10"), 15: ("GP11", "GP11"), 16: ("GP12", "GP12"),
    17: ("GP13", "GP13"), 18: ("GND", "0V_PELV"), 19: ("GP14", "GP14"), 20: ("GP15", "GP15"),
    21: ("GP16", "GP16"), 22: ("GP17", "GP17"), 23: ("GND", "0V_PELV"), 24: ("GP18", "GP18"),
    25: ("GP19", "GP19"), 26: ("GP20", "GP20"), 27: ("GP21", "GP21"), 28: ("GND", "0V_PELV"),
    29: ("GP22", "GP22"), 30: ("RUN", None), 31: ("GP26", "GP26"), 32: ("GP27", "GP27"),
    33: ("AGND", "0V_PELV"), 34: ("GP28", "GP28"), 35: ("ADC_VREF", None), 36: ("3V3", "3V3_MCU"),
    37: ("3V3_EN", None), 38: ("GND", "0V_PELV"), 39: ("VSYS", "+5V_LOGIC"), 40: ("VBUS", None),
}


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


def _pad_world(local_x: float, local_y: float, rotation_deg: float, origin_x: float, origin_y: float) -> tuple[float, float]:
    radians = math.radians(rotation_deg)
    cosine, sine = math.cos(radians), math.sin(radians)
    return (
        origin_x + local_x * cosine - local_y * sine,
        origin_y + local_x * sine + local_y * cosine,
    )


def _footprint_reference(footprint: list[object]) -> str | None:
    for prop in children(footprint, "property"):
        if len(prop) >= 3 and prop[1] == "Reference" and isinstance(prop[2], str):
            return prop[2]
    return None


def verify_pico_socket_orientation() -> int:
    """Derive every J1/J2 physical Pico pin from PCB pad coordinates (F01)."""
    root = parse_sexp(PCB.read_text(encoding="utf-8"))
    sockets: dict[str, tuple[float, float, float, dict[str, tuple[float, float, str | None]]]] = {}
    for footprint in children(root, "footprint"):
        reference = _footprint_reference(footprint)
        if reference not in {"J1", "J2"}:
            continue
        at = child(footprint, "at")
        if at is None or len(at) != 4:
            raise MappingError(f"{reference}: socket footprint has no placement")
        origin_x, origin_y, rotation = float(at[1]), float(at[2]), float(at[3])
        pads: dict[str, tuple[float, float, str | None]] = {}
        for pad in children(footprint, "pad"):
            number = pad[1] if isinstance(pad[1], str) else str(pad[1])
            pad_at = child(pad, "at")
            if pad_at is None or len(pad_at) < 3:
                raise MappingError(f"{reference}-{number}: pad has no position")
            net_node = child(pad, "net")
            net = net_node[2] if net_node is not None and len(net_node) == 3 else None
            if not isinstance(net, str):
                net = None
            pads[number] = (float(pad_at[1]), float(pad_at[2]), net)
        sockets[reference] = (origin_x, origin_y, rotation, pads)
    if set(sockets) != {"J1", "J2"}:
        raise MappingError(f"PCB is missing J1/J2 socket footprints: found {sorted(sockets)}")

    left_x = min(origin_x for origin_x, _, _, _ in sockets.values())
    right_x = max(origin_x for origin_x, _, _, _ in sockets.values())
    if abs((right_x - left_x) - 17.78) > 1e-6:
        raise MappingError(f"Pico socket columns are not 17.78 mm apart: {left_x} vs {right_x}")
    top_y = min(
        origin_y + local_y
        for _, origin_y, _, pads in sockets.values()
        for local_x, local_y, _ in pads.values()
        if local_x == 0.0
    )

    physical_owner: dict[int, tuple[str, str]] = {}
    for reference, (origin_x, origin_y, rotation, pads) in sockets.items():
        if len(pads) != 20:
            raise MappingError(f"{reference}: expected 20 socket pads, found {len(pads)}")
        if abs(origin_x - left_x) <= 1e-9:
            column = "left"
        elif abs(origin_x - right_x) <= 1e-9:
            column = "right"
        else:
            raise MappingError(f"{reference}: socket column x={origin_x} is not a Pico row")
        for number, (local_x, local_y, net) in pads.items():
            world_x, world_y = _pad_world(local_x, local_y, rotation, origin_x, origin_y)
            slot_exact = (world_y - top_y) / 2.54
            slot = round(slot_exact)
            if not 0 <= slot <= 19 or abs(slot_exact - slot) > 1e-6:
                raise MappingError(f"{reference}-{number}: pad y={world_y} is off the 2.54 mm Pico row grid")
            physical = 1 + slot if column == "left" else 40 - slot
            function, expected_net = PICO2_PHYSICAL_PIN_FUNCTION[physical]
            actual_net = net[1:] if net is not None and net.startswith("/") else net
            if expected_net is None:
                if actual_net is not None and not actual_net.startswith("unconnected-"):
                    raise MappingError(
                        f"{reference}-{number} is physical Pico pin {physical} ({function}) and must be unused "
                        f"on IF-B01, but carries net {actual_net!r}"
                    )
            elif actual_net != expected_net:
                raise MappingError(
                    f"{reference}-{number} at ({world_x:.3f}, {world_y:.3f}) is physical Pico pin {physical} ({function}) "
                    f"but carries net {actual_net!r}; expected {expected_net!r} — the socket row is reversed/mirrored"
                )
            if physical in physical_owner:
                raise MappingError(
                    f"physical Pico pin {physical} is mapped by both {physical_owner[physical]} and {(reference, number)}"
                )
            physical_owner[physical] = (reference, number)

    if sorted(physical_owner) != list(range(1, 41)):
        raise MappingError(f"unmapped physical Pico pins: {sorted(set(range(1, 41)) - set(physical_owner))}")
    for reference, offset in (("J1", 0), ("J2", 20)):
        for number in range(1, 21):
            physical = offset + number
            if physical_owner[physical] != (reference, str(number)):
                raise MappingError(
                    f"{reference}-{number} must sit at physical Pico pin {physical}, found {physical_owner[physical]}"
                )

    intent = json.loads(INTENT.read_text(encoding="utf-8"))["component_pin_net_expectations"]
    for reference in ("J1", "J2"):
        expected_pins = intent[reference]
        for number, expected_net in expected_pins.items():
            actual = sockets[reference][3][number][2]
            actual = actual[1:] if actual is not None and actual.startswith("/") else actual
            if expected_net is None:
                if actual is not None and not actual.startswith("unconnected-"):
                    raise MappingError(f"{reference}-{number}: expected an intentional no-connect, found net {actual!r}")
                continue
            if actual != expected_net:
                raise MappingError(f"{reference}-{number}: pad net {actual!r} differs from the native intent {expected_net!r}")
    return len(physical_owner)


def verify_pull_configuration_contract(rows: list[dict[str, str]]) -> None:
    """Verify the F04 contract list and its numeric pull-down/LOW recomputation."""
    intent = json.loads(INTENT.read_text(encoding="utf-8"))
    contract = intent.get("gpio_pull_configuration_contract")
    if not isinstance(contract, dict):
        raise MappingError("native intent lacks the gpio_pull_configuration_contract block")
    expected_gpios = sorted(
        f"GP{row['gpio']}" for row in rows if row["category"] in {"field_input", "alarm_input"}
    )
    if sorted(contract.get("gpios") or []) != expected_gpios:
        raise MappingError(
            f"pull contract GPIO list drift: {contract.get('gpios')!r} != {expected_gpios!r}"
        )
    gpio_nets = contract.get("gpio_nets", {})
    for row in rows:
        if row["category"] in {"field_input", "alarm_input"}:
            gpio = f"GP{row['gpio']}"
            if gpio_nets.get(gpio) != row["net"]:
                raise MappingError(f"pull contract net drift for {gpio}: {gpio_nets.get(gpio)!r} != {row['net']!r}")
    series = float(contract["series_ohm"])
    pull_up = float(contract["external_pull_up_ohm"])
    rail = float(contract["rail_v_nominal"])
    computed_low = rail * series / (series + pull_up)
    if abs(computed_low - float(contract["nominal_low_v"])) > 5e-4:
        raise MappingError(
            f"pull contract nominal LOW {contract['nominal_low_v']} V does not match the recomputed {computed_low:.4f} V"
        )
    if float(contract["vil_max_v"]) != 0.8:
        raise MappingError("pull contract must compare against the RP2350 VIL(max) of 0.8 V")
    if float(contract["worst_case_low_v"]) >= float(contract["vil_max_v"]):
        raise MappingError("pull contract worst-case LOW does not clear the 0.8 V VIL(max)")
    schematic = SCHEMATIC.read_text(encoding="utf-8")
    marker = contract.get("marker")
    if not marker or marker not in schematic:
        raise MappingError("schematic lacks the pull-configuration contract text block")
    block = schematic.split(marker, 1)[1][:512]
    missing = [gpio for gpio in expected_gpios if gpio not in block]
    if missing:
        raise MappingError(f"schematic pull-configuration contract omits {missing}")
    if "disabled in BOTH firmware images" not in block:
        raise MappingError("schematic pull-configuration contract must require disabling internal pulls in both firmware images")


def main() -> int:
    try:
        rows = read_csv(ALLOCATION, HEADERS)
        verify_unique_rows(rows)
        verify_source_rows(rows)
        verify_firmware_use(rows)
        verify_pull_configuration_contract(rows)
        socket_pins = verify_pico_socket_orientation()
        print(
            f"IF-B01/RP2350 GPIO allocation passed: {len(rows)} connector assignments, "
            f"{socket_pins} coordinate-derived Pico 2 socket pins, pull contract verified"
        )
    except (OSError, csv.Error, json.JSONDecodeError, MappingError, StopIteration) as error:
        print(f"IF-B01/RP2350 GPIO allocation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())