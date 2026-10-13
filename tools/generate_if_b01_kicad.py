#!/usr/bin/env python3
"""Create the native-but-unreleased IF-B01 KiCad 10 design source.

The generator owns the logical capture, project-local footprint definitions and
a fully placed/routed four-layer board source.  It deliberately does not make a
release decision: manufacturing files, physical inspection, fixture results and
cabinet/safety qualification remain separately controlled evidence.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import re
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parents[1]
DESIGN_DIR = ROOT / "hardware" / "if-b01"
REVISION = "S1-B-IF-B01-NATIVE-DRAFT-R2-2026-09-13"
PROJECT = "if-b01"
ROOT_UUID = "a12aaf12-6e48-48d2-98f7-22be8f739e69"


@dataclass(frozen=True)
class Pin:
    number: str
    name: str
    electrical_type: str
    x: float
    y: float
    orientation: int


@dataclass(frozen=True)
class SymbolSpec:
    name: str
    reference_prefix: str
    default_value: str
    description: str
    pins: tuple[Pin, ...]
    half_width: float
    half_height: float


@dataclass
class Component:
    reference: str
    symbol: str
    footprint: str
    nets: dict[str, str | None]
    x: float
    y: float
    value: str
    in_bom: bool = True
    on_board: bool = True


@dataclass(frozen=True)
class BoardPlacement:
    """Placed footprint origin in the generated board coordinate system."""

    x: float
    y: float
    rotation: int = 0


@dataclass(frozen=True)
class BoardPad:
    """Physical pad metadata used by the deterministic breakout router."""

    reference: str
    pin: str
    net: str
    x: float
    y: float
    kind: str
    half_x: float
    escape_direction: int
    base_escape: float


@dataclass(frozen=True)
class EscapeVia:
    """A through-via assigned to an individual schematic pin."""

    pad: BoardPad
    x: float
    y: float
    diameter: float
    drill: float


def stable_uuid(key: str) -> str:
    return str(uuid.uuid5(uuid.NAMESPACE_URL, f"https://starz.invalid/if-b01/{key}"))


def fmt(number: float) -> str:
    return f"{number:.3f}".rstrip("0").rstrip(".")


def quoted(value: str) -> str:
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def effects(size: float = 1.27, hide: bool = False) -> str:
    hidden = " (hide yes)" if hide else ""
    return f"(effects (font (size {fmt(size)} {fmt(size)})){hidden})"


def property_line(name: str, value: str, x: float, y: float, hide: bool = False) -> str:
    return f"(property {quoted(name)} {quoted(value)} (at {fmt(x)} {fmt(y)} 0) {effects(hide=hide)})"


def expand_reference(text: str) -> list[str]:
    match = re.fullmatch(r"([A-Za-z]+)(\d+)-([A-Za-z]+)(\d+)", text)
    if not match:
        return [text]
    prefix_a, first, prefix_b, last = match.groups()
    if prefix_a != prefix_b:
        raise ValueError(f"mixed prefixes in reference range {text!r}")
    return [f"{prefix_a}{number}" for number in range(int(first), int(last) + 1)]


def load_bom() -> dict[str, dict[str, str]]:
    path = DESIGN_DIR / "if-b01-bom.csv"
    with path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    parts: dict[str, dict[str, str]] = {}
    for row in rows:
        for reference in expand_reference(row["reference"]):
            if reference in parts:
                raise ValueError(f"duplicate BOM reference {reference}")
            parts[reference] = row
    return parts


def load_connector_input() -> dict[tuple[str, str], str]:
    path = DESIGN_DIR / "if-b01-netlist.csv"
    with path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    return {(row["connector"], row["pin"]): row["net"] for row in rows}


def validate_controls_spec() -> None:
    """Assert the IF-B01-facing GPIO allocation against the source authority."""
    text = (ROOT / "shared" / "controls_spec.hpp").read_text(encoding="utf-8")
    required_patterns = (
        r"gpio_5v_tolerant\s*=\s*false",
        r"blade_front_pul\s*=\s*0\s*,\s*blade_front_dir\s*=\s*1",
        r"blade_rear_pul\s*=\s*2\s*,\s*blade_rear_dir\s*=\s*3",
        r"feed_left_pul\s*=\s*4\s*,\s*feed_left_dir\s*=\s*5",
        r"feed_right_pul\s*=\s*6\s*,\s*feed_right_dir\s*=\s*7",
        r"enc_left_a\s*=\s*8\s*,\s*enc_left_b\s*=\s*9",
        r"enc_right_a\s*=\s*10\s*,\s*enc_right_b\s*=\s*11",
        r"in_start\s*=\s*12",
        r"in_stop_ok\s*=\s*13",
        r"in_reset\s*=\s*14",
        r"in_safety_ok\s*=\s*15",
        r"in_home_rear\s*=\s*16",
        r"out_drv_enable\s*=\s*17",
        r"in_alm_blade_front\s*=\s*18\s*,\s*in_alm_blade_rear\s*=\s*19",
        r"in_alm_feed_left\s*=\s*20\s*,\s*in_alm_feed_right\s*=\s*21",
        r"out_led_green\s*=\s*22",
        r"out_led_red\s*=\s*26",
        r"in_home_front\s*=\s*27",
        r"out_buzzer\s*=\s*28",
    )
    missing = [pattern for pattern in required_patterns if re.search(pattern, text) is None]
    if missing:
        raise ValueError(f"controls_spec.hpp no longer matches IF-B01 GPIO allocation: {missing}")


def header_pins(
    count: int, names: list[str], kinds: list[str], x: float = -10.16, reverse_layout: bool = False
) -> tuple[Pin, ...]:
    if len(names) != count or len(kinds) != count:
        raise ValueError("header pin metadata count mismatch")
    return tuple(
        Pin(
            str(index + 1),
            names[index],
            kinds[index],
            x,
            ((index - (count - 1) / 2) if reverse_layout else ((count - 1) / 2 - index)) * 2.54,
            0,
        )
        for index in range(count)
    )


def generic_two_pin(name: str, reference: str, description: str) -> SymbolSpec:
    return SymbolSpec(
        name,
        reference,
        name,
        description,
        (
            Pin("1", "1", "passive", -5.08, 0, 0),
            Pin("2", "2", "passive", 5.08, 0, 180),
        ),
        2.54,
        2.54,
    )


PICO_LEFT_NAMES = [
    "01 GP0", "02 GP1", "03 GND", "04 GP2", "05 GP3", "06 GP4", "07 GP5", "08 GND",
    "09 GP6", "10 GP7", "11 GP8", "12 GP9", "13 GND", "14 GP10", "15 GP11", "16 GP12",
    "17 GP13", "18 GND", "19 GP14", "20 GP15",
]
PICO_LEFT_KINDS = [
    "output", "output", "power_in", "output", "output", "output", "output", "power_in",
    "output", "output", "bidirectional", "bidirectional", "power_in", "bidirectional", "bidirectional",
    "bidirectional", "bidirectional", "power_in", "bidirectional", "bidirectional",
]
PICO_RIGHT_NAMES = [
    "21 GP16", "22 GP17", "23 GND", "24 GP18", "25 GP19", "26 GP20", "27 GP21", "28 GND",
    "29 GP22", "30 RUN", "31 GP26", "32 GP27", "33 AGND", "34 GP28", "35 ADC_VREF", "36 3V3",
    "37 3V3_EN", "38 GND", "39 VSYS", "40 VBUS",
]
PICO_RIGHT_KINDS = [
    "bidirectional", "output", "power_in", "bidirectional", "bidirectional", "bidirectional", "bidirectional",
    "power_in", "output", "input", "output", "bidirectional", "power_in", "output", "input", "power_out",
    "input", "power_in", "power_in", "power_in",
]

# Physical Pico 2 (SC1631) pinout reference used to prove socket orientation.
# Physical pin 1 (GP0) is the top of the left row; physical pin 21 (GP16) is the
# bottom of the right row; physical pin 40 (VBUS) is the top of the right row.
PICO2_PHYSICAL_ROW_FUNCTION = (
    "GP0", "GP1", "GND", "GP2", "GP3", "GP4", "GP5", "GND", "GP6", "GP7",
    "GP8", "GP9", "GND", "GP10", "GP11", "GP12", "GP13", "GND", "GP14", "GP15",
    "GP16", "GP17", "GND", "GP18", "GP19", "GP20", "GP21", "GND", "GP22", "RUN",
    "GP26", "GP27", "AGND", "GP28", "ADC_VREF", "3V3", "3V3_EN", "GND", "VSYS", "VBUS",
)

# F04 pull-configuration contract: all ten dry-contact inputs rely solely on the
# board's external 47 kOhm 1 % pull-up.  RP2350 internal pulls MUST be disabled in
# BOTH firmware images on exactly these GPIOs, otherwise the guaranteed LOW rises
# to 0.82-1.14 V and exceeds the RP2350 VIL(max) of 0.8 V.
CONTACT_INPUTS = (
    ("IN_START", "GP12"), ("IN_STOP_OK", "GP13"), ("IN_RESET", "GP14"),
    ("IN_SAFETY_OK", "GP15"), ("IN_HOME_REAR", "GP16"), ("IN_HOME_FRONT", "GP27"),
    ("IN_ALM_BF", "GP18"), ("IN_ALM_BR", "GP19"), ("IN_ALM_FL", "GP20"), ("IN_ALM_FR", "GP21"),
)
PULL_DISABLED_GPIO_ORDER = tuple(gpio for _, gpio in CONTACT_INPUTS)
PULL_CONTRACT_MARKER = "PULL-CONFIG CONTRACT"

# F03 RU3100 single-ended TTL interface: one 2.5 V mid-bias divider and filter
# capacitor per receiver minus input (node, encoder rail, top R, bottom R, filter C).
RECEIVER_BIAS_CHANNELS = (
    ("L_ENC_A_N", "+5V_ENC_L", "R38", "R39", "C14"),
    ("L_ENC_B_N", "+5V_ENC_L", "R40", "R41", "C15"),
    ("R_ENC_A_N", "+5V_ENC_R", "R42", "R43", "C16"),
    ("R_ENC_B_N", "+5V_ENC_R", "R44", "R45", "C17"),
)
BIAS_RESISTOR_VALUE = "10k"
BIAS_CAPACITOR_VALUE = "100nF 16V"


SYMBOLS: dict[str, SymbolSpec] = {
    "PICO2_LEFT_SOCKET": SymbolSpec(
        "PICO2_LEFT_SOCKET", "J", "Pico 2 left socket", "Pico 2 SC1631 physical pins 1-20 socket row",
        header_pins(20, PICO_LEFT_NAMES, PICO_LEFT_KINDS), 5.08, 26.67,
    ),
    "PICO2_RIGHT_SOCKET": SymbolSpec(
        "PICO2_RIGHT_SOCKET", "J", "Pico 2 right socket",
        "Pico 2 SC1631 physical pins 21-40 right socket row; pin 1 (GP16) is drawn at the row bottom exactly where physical pin 21 sits",
        header_pins(20, PICO_RIGHT_NAMES, PICO_RIGHT_KINDS, reverse_layout=True), 5.08, 26.67,
    ),
    "HDR4_PELV": SymbolSpec(
        "HDR4_PELV", "J", "MSTB 2.5 4", "Four-position PELV supply header",
        header_pins(4, ["1 +24V", "2 0V", "3 +5V", "4 0V"], ["passive"] * 4), 5.08, 7.62,
    ),
    "HDR4_IO": SymbolSpec(
        "HDR4_IO", "J", "MSTB 2.5 4", "Four-position external I/O header",
        header_pins(4, ["1", "2", "3", "4"], ["passive"] * 4), 5.08, 7.62,
    ),
    "ENC_HDR6": SymbolSpec(
        "ENC_HDR6", "J", "MSTB 2.5 6", "Six-position encoder header",
        header_pins(6, ["1 +5V", "2 0V", "3 A+", "4 A-", "5 B+", "6 B-"], ["passive", "passive", "output", "output", "output", "output"]), 5.08, 10.16,
    ),
    "CONTACT_HDR8": SymbolSpec(
        "CONTACT_HDR8", "J", "MSTB 2.5 8", "Eight-position isolated dry-contact header",
        header_pins(8, [str(number) for number in range(1, 9)], ["passive"] * 8), 5.08, 12.7,
    ),
    "CONTACT_HDR12": SymbolSpec(
        "CONTACT_HDR12", "J", "MSTB 2.5 12", "Twelve-position isolated dry-contact header",
        header_pins(12, [str(number) for number in range(1, 13)], ["passive"] * 12), 5.08, 17.78,
    ),
    "R": generic_two_pin("R", "R", "Two-terminal resistor"),
    "C": generic_two_pin("C", "C", "Two-terminal capacitor"),
    "FUSE": generic_two_pin("FUSE", "F", "Two-terminal resettable fuse"),
    "ZENER": SymbolSpec(
        "ZENER", "D", "Zener", "Two-terminal clamp diode; pin 1 anode, pin 2 cathode",
        (Pin("1", "A", "passive", -5.08, 0, 0), Pin("2", "K", "passive", 5.08, 0, 180)), 2.54, 2.54,
    ),
    "TP": SymbolSpec(
        "TP", "TP", "TestPoint", "Single-pad loop test point",
        (Pin("1", "TP", "passive", -2.54, 0, 0),), 2.54, 2.54,
    ),
    "MH": SymbolSpec("MH", "MH", "MountingHole_3.2mm_M3", "Non-electrical M3 mounting hole", (), 2.54, 2.54),
    "MATING_PLUG": SymbolSpec("MATING_PLUG", "P", "Mating plug", "Supplied off-board mating connector", (), 5.08, 2.54),
    "PWR_FLAG": SymbolSpec(
        "PWR_FLAG", "#FLG", "PWR_FLAG", "ERC power-source declaration for the protected COM rail",
        (Pin("1", "PWR", "power_out", 0, 0, 0),), 2.54, 2.54,
    ),
    "AM26LV32EIDR": SymbolSpec(
        "AM26LV32EIDR", "U", "AM26LV32EIDR", "3.3 V quad RS-422 differential receiver",
        (
            Pin("1", "1B", "input", -15.24, 12.7, 0), Pin("2", "1A", "input", -15.24, 15.24, 0),
            Pin("3", "1Y", "output", 15.24, 13.97, 180), Pin("4", "G", "input", -15.24, 7.62, 0),
            Pin("5", "2Y", "output", 15.24, 5.08, 180), Pin("6", "2A", "input", -15.24, 7.62 - 5.08, 0),
            Pin("7", "2B", "input", -15.24, 0, 0), Pin("8", "GND", "power_in", 0, -17.78, 90),
            Pin("9", "3B", "input", -15.24, -5.08, 0), Pin("10", "3A", "input", -15.24, -7.62, 0),
            Pin("11", "3Y", "output", 15.24, -6.35, 180), Pin("12", "~G", "input", -15.24, -2.54, 0),
            Pin("13", "4Y", "output", 15.24, -13.97, 180), Pin("14", "4A", "input", -15.24, -12.7, 0),
            Pin("15", "4B", "input", -15.24, -15.24, 0), Pin("16", "VDD", "power_in", 0, 17.78, 270),
        ),
        10.16,
        15.24,
    ),
    "TPL7407LDR": SymbolSpec(
        "TPL7407LDR", "U", "TPL7407LDR", "40 V seven-channel low-side NMOS driver",
        (
            Pin("1", "IN1", "input", -12.7, 7.62, 0), Pin("2", "IN2", "input", -12.7, 5.08, 0),
            Pin("3", "IN3", "input", -12.7, 2.54, 0), Pin("4", "IN4", "input", -12.7, 0, 0),
            Pin("5", "IN5", "input", -12.7, -2.54, 0), Pin("6", "IN6", "input", -12.7, -5.08, 0),
            Pin("7", "IN7", "input", -12.7, -7.62, 0), Pin("8", "GND", "power_in", 0, -15.24, 90),
            Pin("9", "COM", "power_in", 0, 15.24, 270), Pin("10", "OUT7", "open_collector", 12.7, -7.62, 180),
            Pin("11", "OUT6", "open_collector", 12.7, -5.08, 180), Pin("12", "OUT5", "open_collector", 12.7, -2.54, 180),
            Pin("13", "OUT4", "open_collector", 12.7, 0, 180), Pin("14", "OUT3", "open_collector", 12.7, 2.54, 180),
            Pin("15", "OUT2", "open_collector", 12.7, 5.08, 180), Pin("16", "OUT1", "open_collector", 12.7, 7.62, 180),
        ),
        7.62,
        12.7,
    ),
}


FOOTPRINT_BY_SYMBOL = {
    "PICO2_LEFT_SOCKET": "PICO_SOCKET_1X20",
    "PICO2_RIGHT_SOCKET": "PICO_SOCKET_1X20_RIGHT",
    "HDR4_PELV": "MSTB_2_5_4_G_5_08",
    "HDR4_IO": "MSTB_2_5_4_G_5_08",
    "ENC_HDR6": "MSTB_2_5_6_G_5_08",
    "CONTACT_HDR8": "MSTB_2_5_8_G_5_08",
    "CONTACT_HDR12": "MSTB_2_5_12_G_5_08",
    "R": "R_0603_1608",
    "C": "C_0603_1608",
    "FUSE": "PTC_1812",
    "ZENER": "SOD_323",
    "TP": "TestPoint_1_0mm",
    "MH": "MountingHole_3_2mm_M3",
    "AM26LV32EIDR": "SOIC_16_3_9x9_9_P1_27",
    "TPL7407LDR": "SOIC_16_3_9x9_9_P1_27",
}


def make_components(parts: dict[str, dict[str, str]]) -> list[Component]:
    components: list[Component] = []

    def add(
        reference: str,
        symbol: str,
        nets: dict[str, str | None],
        x: float,
        y: float,
        value: str | None = None,
        *,
        in_bom: bool = True,
        on_board: bool = True,
        footprint: str | None = None,
    ) -> None:
        if reference in {component.reference for component in components}:
            raise ValueError(f"duplicate generated reference {reference}")
        if in_bom and reference not in parts:
            raise ValueError(f"missing BOM identity for {reference}")
        footprint = (FOOTPRINT_BY_SYMBOL.get(symbol, "") if footprint is None else footprint) if on_board else ""
        components.append(
            Component(
                reference, symbol, footprint, nets, x, y,
                value if value is not None else parts.get(reference, {}).get("exact_order", symbol), in_bom, on_board,
            )
        )

    # Pico 2 rows: J1 is physical Pico pins 1-20, J2 is physical pins 21-40.
    add("J1", "PICO2_LEFT_SOCKET", {
        "1": "GP0", "2": "GP1", "3": "0V_PELV", "4": "GP2", "5": "GP3", "6": "GP4", "7": "GP5",
        "8": "0V_PELV", "9": "GP6", "10": "GP7", "11": "GP8", "12": "GP9", "13": "0V_PELV",
        "14": "GP10", "15": "GP11", "16": "GP12", "17": "GP13", "18": "0V_PELV", "19": "GP14", "20": "GP15",
    }, 25.4, 57.15, "Pico 2 SC1631 left socket")
    add("J2", "PICO2_RIGHT_SOCKET", {
        "1": "GP16", "2": "GP17", "3": "0V_PELV", "4": "GP18", "5": "GP19", "6": "GP20", "7": "GP21",
        "8": "0V_PELV", "9": "GP22", "10": None, "11": "GP26", "12": "GP27", "13": "0V_PELV", "14": "GP28",
        "15": None, "16": "3V3_MCU", "17": None, "18": "0V_PELV", "19": "+5V_LOGIC", "20": None,
    }, 71.12, 57.15, "Pico 2 SC1631 right socket")

    add("J3", "HDR4_PELV", {"1": "+24V_PELV", "2": "0V_PELV", "3": "+5V_LOGIC", "4": "0V_PELV"}, 325.12, 26.67, "1759033")
    add("J4", "HDR4_IO", {"1": "BF_PUL_NEG", "2": "BF_DIR_NEG", "3": "DRV_ENA_NEG", "4": "0V_PELV"}, 355.6, 50.8 + 1.27, "1759033")
    add("J5", "HDR4_IO", {"1": "BR_PUL_NEG", "2": "BR_DIR_NEG", "3": "DRV_ENA_NEG", "4": "0V_PELV"}, 355.6, 76.2 + 1.27, "1759033")
    add("J6", "HDR4_IO", {"1": "FL_PUL_NEG", "2": "FL_DIR_NEG", "3": "DRV_ENA_NEG", "4": "0V_PELV"}, 355.6, 101.6 + 1.27, "1759033")
    add("J7", "HDR4_IO", {"1": "FR_PUL_NEG", "2": "FR_DIR_NEG", "3": "DRV_ENA_NEG", "4": "0V_PELV"}, 355.6, 127 + 1.27, "1759033")
    add("J8", "ENC_HDR6", {"1": "+5V_ENC_L", "2": "0V_PELV", "3": "L_ENC_A_P", "4": "L_ENC_A_N", "5": "L_ENC_B_P", "6": "L_ENC_B_N"}, 292.1, 50.8 + 1.27, "1759059")
    add("J9", "ENC_HDR6", {"1": "+5V_ENC_R", "2": "0V_PELV", "3": "R_ENC_A_P", "4": "R_ENC_A_N", "5": "R_ENC_B_P", "6": "R_ENC_B_N"}, 292.1, 91.44 + 1.27, "1759059")
    add("J10", "CONTACT_HDR12", {
        "1": "IN_START", "2": "0V_PELV", "3": "IN_STOP_OK", "4": "0V_PELV", "5": "IN_RESET", "6": "0V_PELV",
        "7": "IN_SAFETY_OK", "8": "0V_PELV", "9": "IN_HOME_REAR", "10": "0V_PELV", "11": "IN_HOME_FRONT", "12": "0V_PELV",
    }, 373.38, 190.5 + 1.27, "1759114")
    add("J11", "CONTACT_HDR8", {
        "1": "IN_ALM_BF", "2": "0V_PELV", "3": "IN_ALM_BR", "4": "0V_PELV", "5": "IN_ALM_FL", "6": "0V_PELV",
        "7": "IN_ALM_FR", "8": "0V_PELV",
    }, 373.38, 241.3 + 1.27, "1759091")
    add("J12", "HDR4_IO", {"1": "GREEN_NEG", "2": "RED_NEG", "3": "BUZZER_NEG", "4": "0V_PELV"}, 325.12, 251.46 + 1.27, "1759033")

    # Supplied mating plugs are a kit item, not PCB-mounted parts.
    for index, y in enumerate((15.24, 22.86, 30.48, 38.1, 45.72, 53.34, 60.96, 68.58, 76.2, 83.82), start=3):
        add(f"P{index}", "MATING_PLUG", {}, 398.78, y, parts[f"P{index}"]["exact_order"], on_board=False)

    add("U1", "AM26LV32EIDR", {
        "1": "L_ENC_A_N", "2": "L_ENC_A_P", "3": "GP8", "4": "3V3_MCU", "5": "GP9", "6": "L_ENC_B_P", "7": "L_ENC_B_N",
        "8": "0V_PELV", "9": "R_ENC_A_N", "10": "R_ENC_A_P", "11": "GP10", "12": "0V_PELV", "13": "GP11", "14": "R_ENC_B_P", "15": "R_ENC_B_N", "16": "3V3_MCU",
    }, 190.5, 66.04, "AM26LV32EIDR")
    add("U2", "TPL7407LDR", {
        "1": "BF_PUL_IN", "2": "BF_DIR_IN", "3": "BR_PUL_IN", "4": "BR_DIR_IN", "5": "FL_PUL_IN", "6": "FL_DIR_IN", "7": "FR_PUL_IN",
        "8": "0V_PELV", "9": "+24V_TPL_COM", "10": "FR_PUL_NEG", "11": "FL_DIR_NEG", "12": "FL_PUL_NEG", "13": "BR_DIR_NEG", "14": "BR_PUL_NEG", "15": "BF_DIR_NEG", "16": "BF_PUL_NEG",
    }, 182.88, 152.4, "TPL7407LDR")
    add("U3", "TPL7407LDR", {
        "1": "FR_DIR_IN", "2": "DRV_ENABLE_IN", "3": "GREEN_IN", "4": "RED_IN", "5": "BUZZER_IN", "6": "0V_PELV", "7": "0V_PELV",
        "8": "0V_PELV", "9": "+24V_TPL_COM", "10": None, "11": None, "12": "BUZZER_NEG", "13": "RED_NEG", "14": "GREEN_NEG", "15": "DRV_ENA_NEG", "16": "FR_DIR_NEG",
    }, 256.54, 152.4, "TPL7407LDR")

    add("F1", "FUSE", {"1": "+24V_PELV", "2": "+24V_TPL_COM"}, 101.6, 132.08, "MF-MSMF050/60X")
    add("F2", "FUSE", {"1": "+5V_LOGIC", "2": "+5V_ENC_L"}, 266.7, 43.18, "MF-MSMF050/16X")
    add("F3", "FUSE", {"1": "+5V_LOGIC", "2": "+5V_ENC_R"}, 266.7, 83.82, "MF-MSMF050/16X")

    # F03: the RU3100 is wired single-ended (TTL mode); the four RS-422 120 ohm
    # differential terminators (R1-R4) are removed.  Each receiver minus input is
    # held at the 2.5 V mid-bias of the encoder's own 5 V TTL swing by a 10k/10k
    # 1 % divider with a 100 nF filter.  R1-R4 no longer exist.
    for offset, (node_net, rail_net, top_ref, bottom_ref, cap_ref) in enumerate(RECEIVER_BIAS_CHANNELS):
        x = 190.5 + offset * 30.48
        y = 114.3
        add(top_ref, "R", {"1": rail_net, "2": node_net}, x, y, BIAS_RESISTOR_VALUE)
        add(bottom_ref, "R", {"1": "0V_PELV", "2": node_net}, x, y + 10.16, BIAS_RESISTOR_VALUE)
        add(cap_ref, "C", {"1": node_net, "2": "0V_PELV"}, x, y + 20.32, BIAS_CAPACITOR_VALUE)
    add("R5", "R", {"1": "0V_PELV", "2": "DRV_ENABLE_IN"}, 256.54, 193.04, "100k")

    driver_resistors = (
        ("GP0", "BF_PUL_IN"), ("GP1", "BF_DIR_IN"), ("GP2", "BR_PUL_IN"), ("GP3", "BR_DIR_IN"),
        ("GP4", "FL_PUL_IN"), ("GP5", "FL_DIR_IN"), ("GP6", "FR_PUL_IN"), ("GP7", "FR_DIR_IN"),
        ("GP17", "DRV_ENABLE_IN"), ("GP22", "GREEN_IN"), ("GP26", "RED_IN"), ("GP28", "BUZZER_IN"),
    )
    for offset, nets in enumerate(driver_resistors):
        add(f"R{offset + 6}", "R", {"1": nets[0], "2": nets[1]}, 139.7 + (offset % 4) * 17.78, 180.34 + (offset // 4) * 10.16, "1k")

    for offset, (field_net, gpio) in enumerate(CONTACT_INPUTS):
        column, row = offset % 5, offset // 5
        x = 25.4 + column * 68.58
        y = 203.2 + row * 35.56
        add(f"R{offset + 18}", "R", {"1": field_net, "2": gpio}, x, y, "10k")
        add(f"R{offset + 28}", "R", {"1": "3V3_MCU", "2": gpio}, x, y + 10.16, "47k")
        add(f"C{offset + 4}", "C", {"1": gpio, "2": "0V_PELV"}, x, y + 20.32, "10nF 16V")
        add(f"D{offset + 1}", "ZENER", {"1": "0V_PELV", "2": gpio}, x + 25.4, y + 20.32, "BZX384-C3V6")

    add("C1", "C", {"1": "3V3_MCU", "2": "0V_PELV"}, 213.36, 104.14, "100nF 16V")
    add("C2", "C", {"1": "+24V_TPL_COM", "2": "0V_PELV"}, 152.4, 132.08, "100nF 50V")
    add("C3", "C", {"1": "+24V_TPL_COM", "2": "0V_PELV"}, 231.14, 132.08, "100nF 50V")

    test_point_nets = (
        "+24V_PELV", "+5V_LOGIC", "3V3_MCU", "0V_PELV", "DRV_ENA_NEG", "GP0", "GP8", "GP10",
        "L_ENC_A_N", "L_ENC_B_N", "R_ENC_A_N", "R_ENC_B_N",
    )
    for index, net in enumerate(test_point_nets, start=1):
        add(f"TP{index}", "TP", {"1": net}, 127 + (index - 1) * 17.78, 279.4, f"TP {net}")
    for index, (x, y) in enumerate(((12.7, 12.7), (398.78, 12.7), (12.7, 279.4), (398.78, 279.4)), start=1):
        add(f"MH{index}", "MH", {}, x, y, "M3 3.2mm")
    add("#FLG01", "PWR_FLAG", {"1": "+24V_TPL_COM"}, 127, 119.38, "PWR_FLAG", in_bom=False, on_board=False)
    add("#FLG02", "PWR_FLAG", {"1": "0V_PELV"}, 127, 109.22, "PWR_FLAG", in_bom=False, on_board=False)
    add("#FLG03", "PWR_FLAG", {"1": "+5V_LOGIC"}, 127, 99.06, "PWR_FLAG", in_bom=False, on_board=False)

    return components


def validate(parts: dict[str, dict[str, str]], components: list[Component]) -> None:
    bom_refs = set(parts)
    source_refs = {component.reference for component in components if component.in_bom}
    if bom_refs != source_refs:
        missing = sorted(bom_refs - source_refs)
        unexpected = sorted(source_refs - bom_refs)
        raise ValueError(f"BOM/source mismatch: missing={missing}, unexpected={unexpected}")

    by_reference = {component.reference: component for component in components}
    for (connector, pin), expected_net in load_connector_input().items():
        actual_net = by_reference[connector].nets.get(pin)
        if actual_net != expected_net:
            raise ValueError(f"{connector}-{pin}: expected {expected_net}, got {actual_net}")

    required = {
        "U2": {"16": "BF_PUL_NEG", "15": "BF_DIR_NEG", "14": "BR_PUL_NEG", "13": "BR_DIR_NEG", "12": "FL_PUL_NEG", "11": "FL_DIR_NEG", "10": "FR_PUL_NEG"},
        "U3": {"16": "FR_DIR_NEG", "15": "DRV_ENA_NEG", "14": "GREEN_NEG", "13": "RED_NEG", "12": "BUZZER_NEG", "6": "0V_PELV", "7": "0V_PELV", "10": None, "11": None},
        "U1": {"2": "L_ENC_A_P", "1": "L_ENC_A_N", "6": "L_ENC_B_P", "7": "L_ENC_B_N", "10": "R_ENC_A_P", "9": "R_ENC_A_N", "14": "R_ENC_B_P", "15": "R_ENC_B_N", "3": "GP8", "5": "GP9", "11": "GP10", "13": "GP11"},
    }
    for reference, pins in required.items():
        for pin, expected_net in pins.items():
            if by_reference[reference].nets.get(pin) != expected_net:
                raise ValueError(f"{reference}-{pin} allocation is not {expected_net}")

    for reference in ("C2", "C3"):
        if parts[reference]["exact_order"] != "GRM188R71H104KA93D":
            raise ValueError(f"{reference} must be a 50 V COM bypass capacitor")

    if {"R1", "R2", "R3", "R4"} & set(by_reference):
        raise ValueError("the RU3100 interface is single-ended; the R1-R4 120 ohm terminators must not exist")

    ptc_expectations = {"F1": "MF-MSMF050/60X", "F2": "MF-MSMF050/16X", "F3": "MF-MSMF050/16X"}
    for reference, order in ptc_expectations.items():
        if parts[reference]["exact_order"] != order:
            raise ValueError(f"{reference} must be the verified 1812 SMD PTC {order}")
        if by_reference[reference].footprint != "PTC_1812":
            raise ValueError(f"{reference} must use the SMD PTC_1812 footprint")

    for node_net, rail_net, top_ref, bottom_ref, cap_ref in RECEIVER_BIAS_CHANNELS:
        expected = {
            top_ref: {"1": rail_net, "2": node_net},
            bottom_ref: {"1": "0V_PELV", "2": node_net},
            cap_ref: {"1": node_net, "2": "0V_PELV"},
        }
        for reference, nets in expected.items():
            if by_reference[reference].nets != nets:
                raise ValueError(f"{reference} must implement the 2.5 V mid-bias for {node_net}")
        if by_reference[top_ref].value != BIAS_RESISTOR_VALUE or by_reference[bottom_ref].value != BIAS_RESISTOR_VALUE:
            raise ValueError(f"{top_ref}/{bottom_ref} must be {BIAS_RESISTOR_VALUE} bias divider resistors")
        if by_reference[cap_ref].value != BIAS_CAPACITOR_VALUE:
            raise ValueError(f"{cap_ref} must be the {BIAS_CAPACITOR_VALUE} bias filter")

    contract_gpios = {by_reference[f"R{offset + 28}"].nets["2"] for offset in range(len(CONTACT_INPUTS))}
    if contract_gpios != set(PULL_DISABLED_GPIO_ORDER):
        raise ValueError("dry-contact external pull-up GPIO set does not match the pull contract")

    pin_net_by_function = {"GND": "0V_PELV", "AGND": "0V_PELV", "3V3": "3V3_MCU", "VSYS": "+5V_LOGIC"}
    for reference, physical_base in (("J1", 0), ("J2", 20)):
        component = by_reference[reference]
        for pin in range(1, 21):
            function = PICO2_PHYSICAL_ROW_FUNCTION[physical_base + pin - 1]
            expected_net = pin_net_by_function.get(function)
            if expected_net is None and function.startswith("GP"):
                expected_net = function
            if component.nets[str(pin)] != expected_net:
                raise ValueError(
                    f"{reference}-{pin} must carry physical Pico pin {physical_base + pin} ({function}) net {expected_net}"
                )

    for component in components:
        symbol = SYMBOLS[component.symbol]
        expected_pins = {pin.number for pin in symbol.pins}
        if set(component.nets) != expected_pins:
            raise ValueError(f"{component.reference} does not map every {component.symbol} pin")


def render_symbol(spec: SymbolSpec, *, top_name: str | None = None, indent: str = "\t") -> list[str]:
    top_name = spec.name if top_name is None else top_name
    lines = [
        f"{indent}(symbol {quoted(top_name)}",
        f"{indent}\t(exclude_from_sim no)",
        f"{indent}\t(in_bom yes)",
        f"{indent}\t(on_board yes)",
        f"{indent}\t(property \"Reference\" {quoted(spec.reference_prefix)} (at 0 {fmt(spec.half_height + 3.81)} 0) {effects()})",
        f"{indent}\t(property \"Value\" {quoted(spec.default_value)} (at 0 {fmt(spec.half_height + 1.27)} 0) {effects()})",
        f"{indent}\t(property \"Footprint\" \"\" (at 0 0 0) {effects(hide=True)})",
        f"{indent}\t(property \"Datasheet\" \"\" (at 0 0 0) {effects(hide=True)})",
        f"{indent}\t(property \"Description\" {quoted(spec.description)} (at 0 0 0) {effects(hide=True)})",
        f"{indent}\t(symbol {quoted(spec.name + '_0_1')}",
        f"{indent}\t\t(rectangle (start {fmt(-spec.half_width)} {fmt(spec.half_height)}) (end {fmt(spec.half_width)} {fmt(-spec.half_height)}) (stroke (width 0.254) (type default)) (fill (type background)))",
        f"{indent}\t)",
        f"{indent}\t(symbol {quoted(spec.name + '_1_1')}",
    ]
    for pin in spec.pins:
        lines.append(
            f"{indent}\t\t(pin {pin.electrical_type} line (at {fmt(pin.x)} {fmt(pin.y)} {pin.orientation}) (length 2.54) "
            f"(name {quoted(pin.name)} {effects()}) (number {quoted(pin.number)} {effects()}))"
        )
    lines.extend((f"{indent}\t)", f"{indent})"))
    return lines


def endpoint(component: Component, pin_number: str) -> tuple[float, float]:
    pin = next(pin for pin in SYMBOLS[component.symbol].pins if pin.number == pin_number)
    return component.x + pin.x, component.y - pin.y


def render_instance(component: Component, parts: dict[str, dict[str, str]]) -> list[str]:
    bom = parts.get(component.reference, {})
    description = bom.get("value_or_description", SYMBOLS[component.symbol].description)
    manufacturer = bom.get("manufacturer", "STARZ")
    order = bom.get("exact_order", component.value)
    y_label = component.y - SYMBOLS[component.symbol].half_height - 5.08
    lines = [
        "\t(symbol",
        f"\t\t(lib_id {quoted('IFB01:' + component.symbol)})",
        f"\t\t(at {fmt(component.x)} {fmt(component.y)} 0)",
        "\t\t(unit 1)",
        "\t\t(exclude_from_sim no)",
        f"\t\t(in_bom {'yes' if component.in_bom else 'no'})",
        f"\t\t(on_board {'yes' if component.on_board else 'no'})",
        "\t\t(dnp no)",
        f"\t\t(uuid {quoted(stable_uuid('symbol/' + component.reference))})",
        "\t\t" + property_line("Reference", component.reference, component.x, y_label),
        "\t\t" + property_line("Value", component.value, component.x, y_label + 2.54),
        "\t\t" + property_line("Footprint", f"IFB01:{component.footprint}" if component.footprint else "", component.x, component.y, hide=True),
        "\t\t" + property_line("Datasheet", "", component.x, component.y, hide=True),
        "\t\t" + property_line("Description", description, component.x, component.y, hide=True),
        "\t\t" + property_line("Manufacturer", manufacturer, component.x, component.y, hide=True),
        "\t\t" + property_line("MPN", order, component.x, component.y, hide=True),
        "\t\t" + property_line("Design status", "DRAFT — NOT RELEASED FOR FABRICATION OR ENERGISATION", component.x, component.y, hide=True),
        f"\t\t(instances (project {quoted(PROJECT)} (path {quoted('/' + ROOT_UUID)} (reference {quoted(component.reference)}) (unit 1))))",
        "\t)",
    ]
    return lines


def render_schematic(parts: dict[str, dict[str, str]], components: list[Component]) -> str:
    lines = [
        "(kicad_sch",
        "\t(version 20250114)",
        "\t(generator \"eeschema\")",
        "\t(generator_version \"10.0\")",
        f"\t(uuid {quoted(ROOT_UUID)})",
        "\t(paper \"A3\")",
        "\t(title_block",
        "\t\t(title \"IF-B01 PELV interface board — native logical capture\")",
        "\t\t(date \"2026-09-13\")",
        f"\t\t(rev {quoted(REVISION)})",
        "\t\t(company \"STARZ S1-B\")",
        "\t\t(comment 1 \"DRAFT — NOT FOR FABRICATION, ASSEMBLY, ENERGISATION OR SAFETY USE\")",
        "\t\t(comment 2 \"Placed/routed draft PCB source; review and release gates remain open\")",
        "\t\t(comment 3 \"C2/C3: 50 V minimum because TPL COM is connected to +24V_TPL_COM\")",
        "\t\t(comment 4 \"J8/J9 RU3100: single-ended TTL via IO-Link; A-/B- at a 2.5 V mid-bias (10k/10k 1 %, 100 nF); R1-R4 120 ohm terminators removed\")",
        "\t\t(comment 5 \"F1-F3: verified 1812 SMD PTCs MF-MSMF050/60X (24 V branch) and MF-MSMF050/16X (5 V encoder branches)\")",
        "\t)",
        "\t(lib_symbols",
    ]
    for spec in SYMBOLS.values():
        lines.extend(render_symbol(spec, top_name=f"IFB01:{spec.name}", indent="\t\t"))
    lines.append("\t)")
    for component in components:
        for pin_number, net in component.nets.items():
            x, y = endpoint(component, pin_number)
            if net is None:
                lines.append(f"\t(no_connect (at {fmt(x)} {fmt(y)}) (uuid {quoted(stable_uuid('nc/' + component.reference + '/' + pin_number))}))")
            else:
                lines.append(f"\t(label {quoted(net)} (at {fmt(x)} {fmt(y)} 0) {effects()} )")
    for component in components:
        lines.extend(render_instance(component, parts))
    contract_text = (
        f"{PULL_CONTRACT_MARKER}: RP2350 internal pull-up/-down MUST be disabled in BOTH firmware images on "
        + " ".join(PULL_DISABLED_GPIO_ORDER)
        + "; the only pull on each dry-contact pin is the board external 47k 1 % pull-up to 3V3_MCU. "
        "Nominal LOW = 3.3 V x 10k / (10k + 47k) = 0.579 V < VIL max 0.8 V."
    )
    bias_text = (
        "J8/J9 RU3100 single-ended TTL (TTL mode selected via IO-Link): encoder A/B drive J8/J9 pins 3/5 (A+/B+); "
        "A-/B- (pins 4/6) are held at the 2.5 V mid-bias by R38-R45 10k 1 % and C14-C17 100 nF; "
        "the R1-R4 120 ohm differential terminators are removed."
    )
    for text, y in ((contract_text, 288.29), (bias_text, 293.37)):
        lines.append(
            f"\t(text {quoted(text)} (exclude_from_sim no) (at 12.7 {fmt(y)} 0) {effects()})"
        )
    lines.extend(("\t(sheet_instances (path \"/\" (page \"1\")))", "\t(embedded_fonts no)", ")", ""))
    return "\n".join(lines)


def footprint_header(name: str, attributes: str = "smd") -> list[str]:
    return [
        f"(footprint {quoted(name)}",
        "\t(version 20241229)",
        "\t(generator \"pcbnew\")",
        "\t(layer \"F.Cu\")",
        f"\t(attr {attributes})",
        "\t(fp_text reference \"REF**\" (at 0 -2 0) (layer \"F.SilkS\") (effects (font (size 1 1) (thickness 0.15))))",
        f"\t(fp_text value {quoted(name)} (at 0 2 0) (layer \"F.Fab\") (effects (font (size 1 1) (thickness 0.15))))",
    ]


def render_smd_two_pad(name: str, half_pad_spacing: float, pad_width: float, pad_height: float) -> str:
    lines = footprint_header(name)
    lines.extend((
        f"\t(fp_rect (start {fmt(-half_pad_spacing - pad_width / 2 - 0.35)} {fmt(-pad_height / 2 - 0.35)}) (end {fmt(half_pad_spacing + pad_width / 2 + 0.35)} {fmt(pad_height / 2 + 0.35)}) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
        f"\t(fp_rect (start {fmt(-half_pad_spacing - pad_width / 2 - 0.65)} {fmt(-pad_height / 2 - 0.65)}) (end {fmt(half_pad_spacing + pad_width / 2 + 0.65)} {fmt(pad_height / 2 + 0.65)}) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        f"\t(pad \"1\" smd roundrect (at {fmt(-half_pad_spacing)} 0) (size {fmt(pad_width)} {fmt(pad_height)}) (layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25))",
        f"\t(pad \"2\" smd roundrect (at {fmt(half_pad_spacing)} 0) (size {fmt(pad_width)} {fmt(pad_height)}) (layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25))",
        ")",
        "",
    ))
    return "\n".join(lines)


def render_terminal_header(name: str, count: int) -> str:
    lines = footprint_header(name, "through_hole")
    half = (count - 1) * 5.08 / 2
    # This is the 90 degree board-orientation of KiCad's verified
    # PhoenixContact_MSTBVA_2,5_*_G-5,08 vertical footprint.  Its pad pitch,
    # 1.4 mm drill and 2.08 x 3.60 mm land pattern therefore match the selected
    # 1759033/1759059/1759091/1759114 header family while keeping all field
    # terminals aligned along a board edge.
    lines.extend((
        f"\t(fp_rect (start -4.91 {fmt(-half - 3.65)}) (end 3.91 {fmt(half + 3.63)}) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
        f"\t(fp_rect (start -5.3 {fmt(-half - 4.04)}) (end 4.3 {fmt(half + 4.04)}) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        f"\t(fp_line (start -4.91 {fmt(-half)}) (end -3.7 {fmt(-half)}) (stroke (width 0.3) (type default)) (layer \"F.SilkS\"))",
    ))
    for index in range(count):
        y = -half + index * 5.08
        shape = "roundrect" if index == 0 else "oval"
        roundrect = " (roundrect_rratio 0.120192)" if index == 0 else ""
        lines.append(
            f"\t(pad {quoted(str(index + 1))} thru_hole {shape} (at 0 {fmt(y)}) "
            f"(size 3.6 2.08) (drill 1.4) (layers \"*.Cu\" \"*.Mask\"){roundrect})"
        )
    lines.extend((")", ""))
    return "\n".join(lines)


def pico_socket_silk(reverse: bool) -> list[str]:
    """Silk/courtyard lines shared by the library file and board instance."""
    marker_y = 49.59 if reverse else -1.33
    marker_end = 0.0
    lines = [
        "\t(fp_rect (start -1.33 -1.33) (end 1.33 49.59) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
        "\t(fp_rect (start -1.77 -1.77) (end 1.77 50.03) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        f"\t(fp_line (start -1.33 {fmt(marker_y)}) (end {fmt(marker_end)} {fmt(marker_y)}) (stroke (width 0.3) (type default)) (layer \"F.SilkS\"))",
    ]
    return lines


def render_pico_socket(name: str = "PICO_SOCKET_1X20", reverse: bool = False) -> str:
    lines = footprint_header(name, "through_hole")
    lines.extend(pico_socket_silk(reverse))
    for index in range(20):
        pad_y = (48.26 - index * 2.54) if reverse else (index * 2.54)
        shape = "rect" if index == 0 else "circle"
        lines.append(f"\t(pad {quoted(str(index + 1))} thru_hole {shape} (at 0 {fmt(pad_y)}) (size 1.8 1.8) (drill 1) (layers \"*.Cu\" \"*.Mask\"))")
    lines.extend((")", ""))
    return "\n".join(lines)


def render_soic16() -> str:
    lines = footprint_header("SOIC_16_3_9x9_9_P1_27")
    lines.extend((
        "\t(fp_line (start -2.06 -5.06) (end 2.06 -5.06) (stroke (width 0.12) (type default)) (layer \"F.SilkS\"))",
        "\t(fp_line (start 2.06 5.06) (end -2.06 5.06) (stroke (width 0.12) (type default)) (layer \"F.SilkS\"))",
        "\t(fp_poly (pts (xy -2.6 -5.01) (xy -2.84 -5.34) (xy -2.36 -5.34)) (stroke (width 0.12) (type default)) (fill yes) (layer \"F.SilkS\"))",
        "\t(fp_rect (start -3.7 -5.2) (end 3.7 5.2) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
    ))
    for pin in range(1, 9):
        y = (pin - 4.5) * 1.27
        lines.append(f"\t(pad {quoted(str(pin))} smd roundrect (at -2.475 {fmt(y)}) (size 1.95 0.6) (layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25))")
    for pin in range(9, 17):
        y = ((16 - pin) - 3.5) * 1.27
        lines.append(f"\t(pad {quoted(str(pin))} smd roundrect (at 2.475 {fmt(y)}) (size 1.95 0.6) (layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25))")
    lines.extend((")", ""))
    return "\n".join(lines)


def render_test_point() -> str:
    lines = footprint_header("TestPoint_1_0mm", "through_hole")
    lines.extend((
        "\t(fp_circle (center 0 0) (end 1.4 0) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
        "\t(fp_circle (center 0 0) (end 1.65 0) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        "\t(pad \"1\" thru_hole circle (at 0 0) (size 2 2) (drill 1) (layers \"*.Cu\" \"*.Mask\"))",
        ")",
        "",
    ))
    return "\n".join(lines)


def render_mounting_hole() -> str:
    lines = footprint_header("MountingHole_3_2mm_M3", "through_hole")
    lines.extend((
        "\t(fp_circle (center 0 0) (end 2.2 0) (stroke (width 0.15) (type default)) (fill none) (layer \"F.SilkS\"))",
        "\t(fp_circle (center 0 0) (end 2.4 0) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        "\t(pad \"\" np_thru_hole circle (at 0 0) (size 3.2 3.2) (drill 3.2) (layers \"*.Cu\" \"*.Mask\"))",
        ")",
        "",
    ))
    return "\n".join(lines)


def footprint_sources() -> dict[Path, str]:
    library = DESIGN_DIR / "if-b01.pretty"
    files: dict[Path, str] = {
        library / "PICO_SOCKET_1X20.kicad_mod": render_pico_socket(),
        library / "PICO_SOCKET_1X20_RIGHT.kicad_mod": render_pico_socket("PICO_SOCKET_1X20_RIGHT", reverse=True),
        library / "SOIC_16_3_9x9_9_P1_27.kicad_mod": render_soic16(),
        library / "PTC_1812.kicad_mod": render_smd_two_pad("PTC_1812", 2.39, 1.68, 2.95),
        library / "R_0603_1608.kicad_mod": render_smd_two_pad("R_0603_1608", 0.825, 0.8, 0.95),
        library / "R_1206_3216.kicad_mod": render_smd_two_pad("R_1206_3216", 1.4625, 1.125, 1.75),
        library / "C_0603_1608.kicad_mod": render_smd_two_pad("C_0603_1608", 0.775, 0.9, 0.95),
        library / "SOD_323.kicad_mod": render_smd_two_pad("SOD_323", 1.05, 0.6, 0.45),
        library / "TestPoint_1_0mm.kicad_mod": render_test_point(),
        library / "MountingHole_3_2mm_M3.kicad_mod": render_mounting_hole(),
    }
    for count in (4, 6, 8, 12):
        name = f"MSTB_2_5_{count}_G_5_08"
        files[library / f"{name}.kicad_mod"] = render_terminal_header(name, count)
    return files


BOARD_WIDTH = 750.0
BOARD_HEIGHT = 400.0
BOARD_CLEARANCE = 0.20
BUS_START_Y = 280.0


def board_placements(components: list[Component]) -> dict[str, BoardPlacement]:
    """Return the checked physical placement for every board-mounted part.

    The 750 x 400 mm board is intentionally spacious: it keeps the selected
    5.08 mm field connectors at the two board edges, preserves the Pico 2's
    17.78 mm socket-row separation, and provides a separate lower routing bus
    alley.  That separation makes every via and copper path inspectable rather
    than relying on an unreviewable autorouter result.
    """
    placements = {
        # Pico 2 socket rows: physical pin columns are exactly 17.78 mm apart.
        # J2 uses the dedicated reversed (right-row) socket footprint so its pad 1
        # (physical pin 21, GP16) sits at the row bottom exactly where a real
        # Pico 2 places pin 21.
        "J1": BoardPlacement(100.0, 28.0),
        "J2": BoardPlacement(117.78, 28.0),
        # Left-edge field wiring: terminal pins are vertical and fan inward.
        "J3": BoardPlacement(12.0, 20.0),
        "J4": BoardPlacement(12.0, 46.0),
        "J5": BoardPlacement(12.0, 70.0),
        "J6": BoardPlacement(12.0, 94.0),
        "J7": BoardPlacement(12.0, 118.0),
        "J12": BoardPlacement(12.0, 146.0),
        # Right-edge encoder, input and alarm wiring: terminals fan inward.
        "J8": BoardPlacement(738.0, 24.0),
        "J9": BoardPlacement(738.0, 60.0),
        "J10": BoardPlacement(12.0, 235.0),
        "J11": BoardPlacement(12.0, 180.0),
        "MH1": BoardPlacement(5.0, 5.0),
        "MH2": BoardPlacement(745.0, 5.0),
        "MH3": BoardPlacement(5.0, 395.0),
        "MH4": BoardPlacement(745.0, 395.0),
    }
    # The electrical-component yard reserves an independent horizontal escape
    # cell for each footprint.  The allocation follows router order so each
    # F.Cu dogbone remains local; it is not a dense experimental packing.  When
    # the primary lane set reaches BOARD_WIDTH - 75 mm the remainder wraps into
    # a second lower lane set above the routing-bus alley.
    cursor = 180.0
    lane_y = (92.0, 116.0, 140.0, 164.0, 188.0, 212.0)
    overflow_lane_y = (236.0, 260.0)
    overflow = False
    for index, component in enumerate(
        sorted(
            (component for component in components if component.on_board and component.reference not in placements),
            key=lambda component: (component.reference[0], int(re.search(r"\d+", component.reference).group())),
        )
    ):
        if component.symbol in {"AM26LV32EIDR", "TPL7407LDR"}:
            cell_width = 24.0
        elif component.symbol == "FUSE":
            cell_width = 10.0
        elif component.footprint == "R_1206_3216":
            cell_width = 7.0
        elif component.symbol in {"C", "ZENER"}:
            cell_width = 6.0
        elif component.symbol == "TP":
            cell_width = 4.0
        else:
            cell_width = 5.0
        if not overflow and cursor + cell_width > BOARD_WIDTH - 75.0:
            overflow = True
            cursor = 180.0
        lanes = overflow_lane_y if overflow else lane_y
        y = lanes[index % len(lanes)]
        if component.symbol == "TP":
            y = 264.0
        placements[component.reference] = BoardPlacement(cursor + cell_width / 2, y)
        cursor += cell_width
    if cursor > BOARD_WIDTH - 75.0:
        raise ValueError(f"component yard exceeds reserved board width: final x={cursor}")

    expected = {component.reference for component in components if component.on_board}
    actual = set(placements)
    if actual != expected:
        raise ValueError(
            f"PCB placement/reference mismatch: missing={sorted(expected - actual)}, "
            f"unexpected={sorted(actual - expected)}"
        )
    return placements


def render_placement_csv(components: list[Component]) -> str:
    """Render the actual board coordinates; never retain a stale concept CSV."""
    placements = board_placements(components)

    def reference_key(component: Component) -> tuple[str, int]:
        match = re.search(r"\d+", component.reference)
        return component.reference.rstrip("0123456789"), int(match.group()) if match else 0

    def placement_region(component: Component) -> str:
        if component.reference in {"J1", "J2"}:
            return "Pico 2 carrier envelope"
        if component.reference in {"J3", "J4", "J5", "J6", "J7", "J8", "J9", "J10", "J11", "J12"}:
            return "field-connector edge"
        if component.symbol == "MH":
            return "M3 panel mounting"
        if component.symbol == "TP":
            return "fixture access row"
        return "isolated routing-cell component yard"

    rows = [
        "reference,footprint,x_mm,y_mm,rotation_deg,side,layout_region,status",
    ]
    for component in sorted((component for component in components if component.on_board), key=reference_key):
        placement = placements[component.reference]
        side = "none" if component.symbol == "MH" else "top"
        rows.append(
            ",".join((
                component.reference,
                component.footprint,
                fmt(placement.x),
                fmt(placement.y),
                str(placement.rotation),
                side,
                placement_region(component),
                "DRAFT — human layout and fabricator review required",
            ))
        )
    return "\n".join(rows) + "\n"


def rotate(local_x: float, local_y: float, placement: BoardPlacement) -> tuple[float, float]:
    """Transform a local pad coordinate through a footprint placement."""
    radians = math.radians(placement.rotation)
    cosine, sine = math.cos(radians), math.sin(radians)
    return (
        placement.x + local_x * cosine - local_y * sine,
        placement.y + local_x * sine + local_y * cosine,
    )


def pad_geometry(component: Component) -> dict[str, tuple[float, float, str, float, float]]:
    """Return local pad centres, technology and copper half-size by pin."""
    if component.symbol in {"HDR4_PELV", "HDR4_IO", "ENC_HDR6", "CONTACT_HDR8", "CONTACT_HDR12"}:
        count = len(component.nets)
        half = (count - 1) * 5.08 / 2
        return {
            str(index + 1): (0.0, -half + index * 5.08, "tht", 1.8, 1.04)
            for index in range(count)
        }
    if component.symbol in {"PICO2_LEFT_SOCKET", "PICO2_RIGHT_SOCKET"}:
        reverse = component.symbol == "PICO2_RIGHT_SOCKET"
        return {
            str(index + 1): (0.0, (48.26 - index * 2.54) if reverse else (index * 2.54), "tht", 0.9, 0.9)
            for index in range(20)
        }
    if component.symbol in {"AM26LV32EIDR", "TPL7407LDR"}:
        result: dict[str, tuple[float, float, str, float, float]] = {}
        for pin in range(1, 9):
            result[str(pin)] = (-2.475, (pin - 4.5) * 1.27, "smd", 0.975, 0.3)
        for pin in range(9, 17):
            result[str(pin)] = (2.475, ((16 - pin) - 3.5) * 1.27, "smd", 0.975, 0.3)
        return result
    two_pad_geometries = {
        "R_0603_1608": (0.825, 0.4, 0.475),
        "C_0603_1608": (0.775, 0.45, 0.475),
        "R_1206_3216": (1.4625, 0.5625, 0.875),
        "PTC_1812": (2.39, 0.84, 1.475),
        "SOD_323": (1.05, 0.3, 0.225),
    }
    if component.footprint in two_pad_geometries:
        half_spacing, half_x, half_y = two_pad_geometries[component.footprint]
        return {
            "1": (-half_spacing, 0.0, "smd", half_x, half_y),
            "2": (half_spacing, 0.0, "smd", half_x, half_y),
        }
    if component.symbol == "TP":
        return {"1": (0.0, 0.0, "tht", 1.0, 1.0)}
    if component.symbol == "MH":
        return {}
    raise ValueError(f"no PCB pad geometry for {component.reference} ({component.symbol}/{component.footprint})")


def board_pads(components: list[Component], placements: dict[str, BoardPlacement]) -> list[BoardPad]:
    """Flatten placed electrical pads into the router's coordinate system."""
    left_edge = {"J3", "J4", "J5", "J6", "J7", "J10", "J11", "J12"}
    right_edge = {"J8", "J9"}
    pads: list[BoardPad] = []
    for component in components:
        if not component.on_board:
            continue
        placement = placements[component.reference]
        geometries = pad_geometry(component)
        for pin, net in component.nets.items():
            if net is None:
                continue
            local_x, local_y, kind, half_x, half_y = geometries[pin]
            x, y = rotate(local_x, local_y, placement)
            if component.reference in left_edge:
                direction, base_escape = 1, 3.0
            elif component.reference in right_edge:
                direction, base_escape = -1, 3.0
            elif component.reference == "J1":
                direction, base_escape = -1, 1.5
            elif component.reference == "J2":
                direction, base_escape = 1, 1.5
            elif local_x < 0:
                direction, base_escape = -1, max(1.05, half_x + 0.75)
            elif local_x > 0:
                direction, base_escape = 1, max(1.05, half_x + 0.75)
            else:
                direction, base_escape = (1 if x < BOARD_WIDTH / 2 else -1), 1.65
            # The board uses rotations of zero today; preserve conservative
            # collision geometry if a later placement uses a right angle.
            angle = math.radians(placement.rotation)
            projected_half_x = abs(math.cos(angle)) * half_x + abs(math.sin(angle)) * half_y
            pads.append(
                BoardPad(component.reference, pin, net, x, y, kind, projected_half_x, direction, base_escape)
            )
    return pads


def net_width(net: str) -> float:
    """Return the board rule width selected for a named net."""
    if net == "0V_PELV":
        return 1.2
    if net in {"+24V_PELV", "+24V_TPL_COM"}:
        return 1.0
    if net.startswith("+5V"):
        return 0.75
    if net == "DRV_ENA_NEG":
        return 0.5
    return 0.25


def via_dimensions(net: str) -> tuple[float, float]:
    """Use conventional through-vias sized for the selected routing width."""
    if net_width(net) >= 0.75:
        return 1.2, 0.6
    return 0.7, 0.35


def bus_positions(nets: Iterable[str]) -> dict[str, float]:
    """Assign a clear B.Cu routing-bus lane to every electrical net."""
    preferred = ("0V_PELV", "+24V_PELV", "+24V_TPL_COM", "+5V_LOGIC", "+5V_ENC_L", "+5V_ENC_R")
    ordered = [net for net in preferred if net in nets]
    ordered.extend(sorted(set(nets) - set(ordered)))
    positions: dict[str, float] = {}
    y = BUS_START_Y
    previous_via = 0.0
    for net in ordered:
        via, _ = via_dimensions(net)
        if positions:
            y += previous_via / 2 + via / 2 + BOARD_CLEARANCE + 0.1
        positions[net] = y
        previous_via = via
    if y > BOARD_HEIGHT - 18.0:
        raise ValueError(f"routing-bus alley exceeds board clearance: final lane y={y}")
    return positions


def assign_escape_vias(pads: list[BoardPad], buses: dict[str, float]) -> list[EscapeVia]:
    """Assign collision-free through-via columns for all pad escape routes.

    F.Cu is used only for a straight local dogbone from each pad.  Every
    uniquely-spaced through-via then carries its net vertically on In1.Cu to
    a dedicated B.Cu horizontal bus lane.  A column therefore cannot cross a
    differently-named column, and the resulting topology is reproducible and
    straightforward to inspect in KiCad.
    """
    # Every placed copper pad of a differently-named net is a physical blocker
    # for an inner-layer vertical breakout: a through-via barrel passes all
    # copper layers, and F.Cu dogbones must not cross a foreign pad.  The four
    # non-plated mounting holes are blockers even though they have no schematic
    # pin or electrical net.
    blockers = list(pads) + [
        BoardPad(f"MH{index}", "", "", x, y, "tht", 1.6, 0, 0.0)
        for index, (x, y) in enumerate(((5.0, 5.0), (745.0, 5.0), (5.0, 395.0), (745.0, 395.0)), start=1)
    ]
    assigned: list[EscapeVia] = []
    # Group edge connectors first to reserve their deliberately empty fan-out
    # corridors; stable ordering makes the board byte-for-byte reproducible.
    priority = {reference: index for index, reference in enumerate(("J3", "J4", "J5", "J6", "J7", "J12", "J8", "J9", "J10", "J11", "J1", "J2"))}
    ordered = sorted(pads, key=lambda pad: (priority.get(pad.reference, 99), pad.reference, int(pad.pin)))
    for pad in ordered:
        via_diameter, drill = via_dimensions(pad.net)
        width = net_width(pad.net)
        candidate: float | None = None
        for step in range(800):
            # 0.20 mm candidate granularity lets unrelated footprint rows use
            # nearby columns while all accepted copper stays clearance-safe.
            proposed = pad.x + pad.escape_direction * (pad.base_escape + step * 0.20)
            if not (4.0 <= proposed <= BOARD_WIDTH - 4.0):
                continue
            if any(
                abs(proposed - existing.x) < (via_diameter + existing.diameter) / 2 + BOARD_CLEARANCE
                for existing in assigned
            ):
                continue
            blocked = False
            for other in blockers:
                if other.net == pad.net and other.net:
                    continue
                # The inner-layer vertical trace runs from this pad toward its
                # lower bus lane.  It must not pass through another THT annulus
                # or a mounting hole on any copper layer.
                if other.y + 1e-6 < pad.y or other.y - 1e-6 > buses[pad.net]:
                    continue
                required = other.half_x + width / 2 + BOARD_CLEARANCE
                if abs(proposed - other.x) < required:
                    blocked = True
                    break
            if not blocked:
                candidate = proposed
                break
        if candidate is None:
            raise ValueError(f"could not allocate a safe via column for {pad.reference}.{pad.pin}")
        assigned.append(EscapeVia(pad, candidate, pad.y, via_diameter, drill))
    return assigned


def pcb_net_name(net: str) -> str:
    """Match KiCad's root-sheet local-label spelling used by the schematic."""
    return "/" + net


def pad_net_clause(
    component: Component,
    pin: str,
    net: str | None,
    net_ids: dict[str, int],
    unconnected_net_ids: dict[tuple[str, str], tuple[int, str]],
) -> str:
    if net is not None:
        return f" (net {net_ids[net]} {quoted(pcb_net_name(net))})"
    number, name = unconnected_net_ids[(component.reference, pin)]
    return f" (net {number} {quoted(name)})"


def render_board_footprint(
    component: Component,
    placement: BoardPlacement,
    net_ids: dict[str, int],
    unconnected_net_ids: dict[tuple[str, str], tuple[int, str]],
    parts: dict[str, dict[str, str]],
) -> list[str]:
    """Embed one physical local-library footprint with its assigned pad nets."""
    geometry = pad_geometry(component)
    terminal_symbols = {"HDR4_PELV", "HDR4_IO", "ENC_HDR6", "CONTACT_HDR8", "CONTACT_HDR12"}
    if component.symbol in terminal_symbols:
        terminal_reference_clearance = (len(component.nets) - 1) * 5.08 / 2 + 6.0
        reference_y = terminal_reference_clearance if component.reference == "J10" else -terminal_reference_clearance
    elif component.symbol in {"AM26LV32EIDR", "TPL7407LDR"}:
        reference_y = -6.2
    elif component.symbol == "TP":
        reference_y = -2.5
    else:
        reference_y = -3.0
    bom = parts[component.reference]
    description = bom["value_or_description"]
    manufacturer = bom["manufacturer"]
    order = bom["exact_order"]
    lines = [
        f"\t(footprint {quoted('IFB01:' + component.footprint)}",
        "\t\t(layer \"F.Cu\")",
        f"\t\t(at {fmt(placement.x)} {fmt(placement.y)} {placement.rotation})",
        f"\t\t(property \"Reference\" {quoted(component.reference)} (at 0 {fmt(reference_y)} 0) (layer \"F.SilkS\") {effects(1.0)})",
        f"\t\t(property \"Value\" {quoted(component.value)} (at 0 3.2 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        f"\t\t(property \"Footprint\" {quoted('IFB01:' + component.footprint)} (at 0 0 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        f"\t\t(property \"Datasheet\" \"\" (at 0 0 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        f"\t\t(property \"Description\" {quoted(description)} (at 0 0 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        f"\t\t(property \"Manufacturer\" {quoted(manufacturer)} (at 0 0 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        f"\t\t(property \"MPN\" {quoted(order)} (at 0 0 0) (layer \"F.Fab\") (hide yes) {effects(1.0)})",
        "\t\t(property \"Design status\" \"DRAFT — NOT RELEASED FOR FABRICATION OR ENERGISATION\" (at 0 0 0) (layer \"F.Fab\") (hide yes) (effects (font (size 1 1))))",
    ]
    if component.symbol in terminal_symbols:
        count = len(component.nets)
        half = (count - 1) * 5.08 / 2
        lines.extend((
            "\t\t(attr through_hole)",
            f"\t\t(fp_rect (start -4.91 {fmt(-half - 3.65)}) (end 3.91 {fmt(half + 3.63)}) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
            f"\t\t(fp_rect (start -5.3 {fmt(-half - 4.04)}) (end 4.3 {fmt(half + 4.04)}) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
            f"\t\t(fp_line (start -4.91 {fmt(-half)}) (end -3.7 {fmt(-half)}) (stroke (width 0.3) (type default)) (layer \"F.SilkS\"))",
        ))
        for pin, (x, y, _, _, _) in geometry.items():
            shape = "roundrect" if pin == "1" else "oval"
            roundrect = " (roundrect_rratio 0.120192)" if pin == "1" else ""
            lines.append(
                f"\t\t(pad {quoted(pin)} thru_hole {shape} (at {fmt(x)} {fmt(y)}) (size 3.6 2.08) "
                f"(drill 1.4) (layers \"*.Cu\" \"*.Mask\"){pad_net_clause(component, pin, component.nets[pin], net_ids, unconnected_net_ids)}{roundrect})"
            )
    elif component.symbol in {"PICO2_LEFT_SOCKET", "PICO2_RIGHT_SOCKET"}:
        reverse = component.symbol == "PICO2_RIGHT_SOCKET"
        lines.append("\t\t(attr through_hole)")
        lines.extend(pico_socket_silk(reverse))
        for pin, (x, y, _, _, _) in geometry.items():
            shape = "rect" if pin == "1" else "circle"
            lines.append(
                f"\t\t(pad {quoted(pin)} thru_hole {shape} (at {fmt(x)} {fmt(y)}) (size 1.8 1.8) "
                f"(drill 1) (layers \"*.Cu\" \"*.Mask\"){pad_net_clause(component, pin, component.nets[pin], net_ids, unconnected_net_ids)})"
            )
    elif component.symbol in {"AM26LV32EIDR", "TPL7407LDR"}:
        lines.extend((
            "\t\t(attr smd)",
            "\t\t(fp_line (start -2.06 -5.06) (end 2.06 -5.06) (stroke (width 0.12) (type default)) (layer \"F.SilkS\"))",
            "\t\t(fp_line (start 2.06 5.06) (end -2.06 5.06) (stroke (width 0.12) (type default)) (layer \"F.SilkS\"))",
            "\t\t(fp_poly (pts (xy -2.6 -5.01) (xy -2.84 -5.34) (xy -2.36 -5.34)) (stroke (width 0.12) (type default)) (fill yes) (layer \"F.SilkS\"))",
            "\t\t(fp_rect (start -3.7 -5.2) (end 3.7 5.2) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        ))
        for pin, (x, y, _, _, _) in geometry.items():
            lines.append(
                f"\t\t(pad {quoted(pin)} smd roundrect (at {fmt(x)} {fmt(y)}) (size 1.95 0.6) "
                f"(layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25){pad_net_clause(component, pin, component.nets[pin], net_ids, unconnected_net_ids)})"
            )
    elif component.symbol in {"R", "C", "FUSE", "ZENER"}:
        half_spacing, half_x, half_y = {
            "R_0603_1608": (0.825, 0.4, 0.475),
            "C_0603_1608": (0.775, 0.45, 0.475),
            "R_1206_3216": (1.4625, 0.5625, 0.875),
            "PTC_1812": (2.39, 0.84, 1.475),
            "SOD_323": (1.05, 0.3, 0.225),
        }[component.footprint]
        lines.extend((
            "\t\t(attr smd)",
            f"\t\t(fp_rect (start {fmt(-half_spacing - half_x - 0.35)} {fmt(-half_y - 0.35)}) (end {fmt(half_spacing + half_x + 0.35)} {fmt(half_y + 0.35)}) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
            f"\t\t(fp_rect (start {fmt(-half_spacing - half_x - 0.65)} {fmt(-half_y - 0.65)}) (end {fmt(half_spacing + half_x + 0.65)} {fmt(half_y + 0.65)}) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
        ))
        for pin, (x, y, _, half_pad_x, half_pad_y) in geometry.items():
            lines.append(
                f"\t\t(pad {quoted(pin)} smd roundrect (at {fmt(x)} {fmt(y)}) (size {fmt(half_pad_x * 2)} {fmt(half_pad_y * 2)}) "
                f"(layers \"F.Cu\" \"F.Paste\" \"F.Mask\") (roundrect_rratio 0.25){pad_net_clause(component, pin, component.nets[pin], net_ids, unconnected_net_ids)})"
            )
    elif component.symbol == "TP":
        lines.extend((
            "\t\t(attr through_hole)",
            "\t\t(fp_circle (center 0 0) (end 1.4 0) (stroke (width 0.12) (type default)) (fill none) (layer \"F.SilkS\"))",
            "\t\t(fp_circle (center 0 0) (end 1.65 0) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
            f"\t\t(pad \"1\" thru_hole circle (at 0 0) (size 2 2) (drill 1) (layers \"*.Cu\" \"*.Mask\"){pad_net_clause(component, '1', component.nets['1'], net_ids, unconnected_net_ids)})",
        ))
    elif component.symbol == "MH":
        lines.extend((
            "\t\t(attr through_hole)",
            "\t\t(fp_circle (center 0 0) (end 2.2 0) (stroke (width 0.15) (type default)) (fill none) (layer \"F.SilkS\"))",
            "\t\t(fp_circle (center 0 0) (end 2.4 0) (stroke (width 0.05) (type default)) (fill none) (layer \"F.CrtYd\"))",
            "\t\t(pad \"\" np_thru_hole circle (at 0 0) (size 3.2 3.2) (drill 3.2) (layers \"*.Cu\" \"*.Mask\"))",
        ))
    else:
        raise ValueError(f"no board footprint renderer for {component.reference}")
    lines.append("\t)")
    return lines


def render_pcb(parts: dict[str, dict[str, str]], components: list[Component]) -> str:
    """Render all placed components, local escapes and complete copper routing."""
    placements = board_placements(components)
    pads = board_pads(components, placements)
    nets = sorted({pad.net for pad in pads})
    net_ids = {net: index for index, net in enumerate(nets, start=1)}
    symbols = SYMBOLS
    unconnected_pins = [
        (component, pin)
        for component in components
        if component.on_board
        for pin, net in component.nets.items()
        if net is None
    ]
    unconnected_net_ids = {
        (component.reference, pin): (
            len(net_ids) + index,
            f"unconnected-({component.reference}-{next(symbol_pin.name for symbol_pin in symbols[component.symbol].pins if symbol_pin.number == pin).replace(' ', '_')}-Pad{pin})",
        )
        for index, (component, pin) in enumerate(unconnected_pins, start=1)
    }
    buses = bus_positions(nets)
    vias = assign_escape_vias(pads, buses)
    lines = [
        "(kicad_pcb",
        "\t(version 20241229)",
        "\t(generator \"pcbnew\")",
        "\t(generator_version \"10.0\")",
        "\t(general (thickness 1.6))",
        "\t(paper \"A3\")",
        "\t(title_block",
        "\t\t(title \"IF-B01 PELV interface board — placed and routed draft\")",
        "\t\t(date \"2026-09-13\")",
        f"\t\t(rev {quoted(REVISION)})",
        "\t\t(company \"STARZ S1-B\")",
        "\t\t(comment 1 \"DRAFT — NOT RELEASED FOR FABRICATION, ASSEMBLY, ENERGISATION OR SAFETY USE\")",
        "\t\t(comment 2 \"4-layer, 1.6 mm FR-4 design source; final fabricator stack-up remains a release gate\")",
        "\t)",
        "\t(layers",
        "\t\t(0 \"F.Cu\" signal)",
        "\t\t(2 \"In1.Cu\" power \"Inner layer 1\")",
        "\t\t(4 \"In2.Cu\" power \"Inner layer 2\")",
        "\t\t(31 \"B.Cu\" signal)",
        "\t\t(34 \"B.Paste\" user \"b.Paste\")",
        "\t\t(35 \"F.Paste\" user \"f.Paste\")",
        "\t\t(36 \"B.SilkS\" user \"b.Silkscreen\")",
        "\t\t(37 \"F.SilkS\" user \"f.Silkscreen\")",
        "\t\t(38 \"B.Mask\" user \"b.Mask\")",
        "\t\t(39 \"F.Mask\" user \"f.Mask\")",
        "\t\t(40 \"Dwgs.User\" user \"User.Drawings\")",
        "\t\t(44 \"Edge.Cuts\" user)",
        "\t\t(46 \"B.CrtYd\" user \"b.Courtyard\")",
        "\t\t(47 \"F.CrtYd\" user \"f.Courtyard\")",
        "\t\t(48 \"B.Fab\" user)",
        "\t\t(49 \"F.Fab\" user)",
        "\t)",
        "\t(setup (pad_to_mask_clearance 0))",
        "\t(net 0 \"\")",
    ]
    for net, number in net_ids.items():
        lines.append(f"\t(net {number} {quoted(pcb_net_name(net))})")
    for number, name in unconnected_net_ids.values():
        lines.append(f"\t(net {number} {quoted(name)})")
    lines.extend((
        f"\t(gr_rect (start 0 0) (end {fmt(BOARD_WIDTH)} {fmt(BOARD_HEIGHT)}) (stroke (width 0.25) (type default)) (fill none) (layer \"Edge.Cuts\"))",
        "\t(gr_rect (start 97.5 26.5) (end 138.5 80.0) (stroke (width 0.2) (type dash)) (fill none) (layer \"Dwgs.User\"))",
        "\t(gr_text \"Pico 2 SC1631 mechanical envelope; J1–J2 row spacing = 17.78 mm\" (at 118 23.5 0) (layer \"Dwgs.User\") (effects (font (size 1.2 1.2) (thickness 0.2))))",
        f"\t(gr_rect (start 18 {fmt(BUS_START_Y - 4)}) (end 732 374) (stroke (width 0.2) (type dash)) (fill none) (layer \"Dwgs.User\"))",
        "\t(gr_text \"B.Cu net-bus alley / In1.Cu vertical breakout; do not place components in this area\" (at 375 273 0) (layer \"Dwgs.User\") (effects (font (size 1.2 1.2) (thickness 0.2))))",
        "\t(gr_text \"IF-B01 R2 DRAFT — placed/routed source only; no release or safety approval\" (at 375 388 0) (layer \"F.SilkS\") (effects (font (size 1.3 1.3) (thickness 0.22))))",
    ))
    for component in components:
        if component.on_board:
            lines.extend(render_board_footprint(component, placements[component.reference], net_ids, unconnected_net_ids, parts))
    for via in vias:
        width = net_width(via.pad.net)
        net_id = net_ids[via.pad.net]
        bus_y = buses[via.pad.net]
        # The local F.Cu dogbone joins the pad to its own escape via; the
        # unique-column In1.Cu segment and final via reach the B.Cu bus lane.
        lines.extend((
            f"\t(segment (start {fmt(via.pad.x)} {fmt(via.pad.y)}) (end {fmt(via.x)} {fmt(via.y)}) (width {fmt(width)}) (layer \"F.Cu\") (net {net_id}))",
            f"\t(via (at {fmt(via.x)} {fmt(via.y)}) (size {fmt(via.diameter)}) (drill {fmt(via.drill)}) (layers \"F.Cu\" \"B.Cu\") (net {net_id}))",
            f"\t(segment (start {fmt(via.x)} {fmt(via.y)}) (end {fmt(via.x)} {fmt(bus_y)}) (width {fmt(width)}) (layer \"In1.Cu\") (net {net_id}))",
            f"\t(via (at {fmt(via.x)} {fmt(bus_y)}) (size {fmt(via.diameter)}) (drill {fmt(via.drill)}) (layers \"F.Cu\" \"B.Cu\") (net {net_id}))",
        ))
    for net in nets:
        net_vias = [via for via in vias if via.pad.net == net]
        if len(net_vias) < 2:
            continue
        lines.append(
            f"\t(segment (start {fmt(min(via.x for via in net_vias))} {fmt(buses[net])}) "
            f"(end {fmt(max(via.x for via in net_vias))} {fmt(buses[net])}) (width {fmt(net_width(net))}) "
            f"(layer \"B.Cu\") (net {net_ids[net]}))"
        )
    lines.extend((")", ""))
    return "\n".join(lines)


def render_manifest(parts: dict[str, dict[str, str]], components: list[Component]) -> str:
    connector_map = {
        component.reference: component.nets
        for component in components
        if component.reference.startswith("J") and component.reference != "J1" and component.reference != "J2"
    }
    input_hashes = {
        path.name: hashlib.sha256(path.read_bytes()).hexdigest()
        for path in (DESIGN_DIR / "if-b01-bom.csv", DESIGN_DIR / "if-b01-netlist.csv", ROOT / "shared" / "controls_spec.hpp")
    }
    data = {
        "schema": 1,
        "revision": REVISION,
        "status": "native logical schematic and placed/routed four-layer PCB source complete; draft remains unreleased",
        "input_sha256": input_hashes,
        "bom_references": sorted(parts),
        "component_mpn_expectations": {reference: parts[reference]["exact_order"] for reference in sorted(parts)},
        "schematic_references": sorted(component.reference for component in components if component.in_bom),
        "netlist_references": sorted(component.reference for component in components if component.in_bom and (component.on_board or component.nets)),
        "board_references_present": sorted(component.reference for component in components if component.on_board),
        "connector_net_expectations": connector_map,
        "component_pin_net_expectations": {component.reference: component.nets for component in components if component.in_bom},
        "intentional_no_connects": {component.reference: [pin for pin, net in component.nets.items() if net is None] for component in components if any(net is None for net in component.nets.values())},
        "pico_socket_evidence": {
            "row_spacing_mm": 17.78,
            "row_span_mm": 48.26,
            "pin_pitch_mm": 2.54,
            "physical_pin_21_position": "bottom of the right socket row, same Y as physical pin 20",
            "detection": "tools/check_if_b01_gpio_mapping.py recomputes the physical Pico pin of every J1/J2 pad from pad coordinates and compares it with the intended pin function",
        },
        "gpio_pull_configuration_contract": {
            "marker": PULL_CONTRACT_MARKER,
            "requirement": "RP2350 internal pull-up/pull-down MUST be disabled in BOTH firmware images on the listed dry-contact/alarm GPIOs; the only pull is the board external 47 kOhm 1 % pull-up to 3V3_MCU.",
            "gpios": list(PULL_DISABLED_GPIO_ORDER),
            "gpio_nets": {gpio: net for net, gpio in CONTACT_INPUTS},
            "series_ohm": 10000,
            "external_pull_up_ohm": 47000,
            "rail_v_nominal": 3.3,
            "filter": "10 nF",
            "clamp": "BZX384-C3V6",
            "nominal_low_v": 0.579,
            "vil_max_v": 0.8,
            "worst_case_low_v": 0.621,
            "worst_case_basis": "3.465 V (+5 %) rail and 46.53 kOhm (-1 %) external pull-up give 0.613 V; 1 uA pin leakage into the 10 kOhm/47 kOhm node adds 8 mV -> 0.621 V < 0.8 V",
        },
        "receiver_minus_input_bias": {
            "encoder_mode": "RU3100 single-ended TTL, TTL mode selected via IO-Link",
            "terminator_state": "R1-R4 120 ohm differential terminators removed; no 120 ohm part exists on the board",
            "channels": {
                node_net: {"top_resistor": top_ref, "bottom_resistor": bottom_ref, "filter_capacitor": cap_ref, "rail": rail_net}
                for node_net, rail_net, top_ref, bottom_ref, cap_ref in RECEIVER_BIAS_CHANNELS
            },
            "divider_ohm": 10000,
            "unloaded_bias_v": 2.5,
            "thevenin_ohm": 5000,
        },
        "release_holds": [
            "This source is a placed/routed draft; actual connector fit, fabricator stack-up and human layout review remain incomplete.",
            "A clean source DRC cannot replace fabrication inspection or physical board verification.",
            "TPL COM slew-rate/bulk-capacitor behaviour, branch loading and physical fixture results remain open.",
            "This interface board has no safety function and does not release cabinet or machine energisation.",
        ],
    }
    return json.dumps(data, indent=2, sort_keys=True) + "\n"


def sources() -> dict[Path, str]:
    validate_controls_spec()
    parts = load_bom()
    components = make_components(parts)
    validate(parts, components)
    files = {
        DESIGN_DIR / "if-b01.kicad_pro": "{}\n",
        DESIGN_DIR / "sym-lib-table": "(sym_lib_table\n\t(version 7)\n\t(lib (name \"IFB01\")(type \"KiCad\")(uri \"${KIPRJMOD}/if-b01.kicad_sym\")(options \"\")(descr \"IF-B01 local draft symbols\"))\n)\n",
        DESIGN_DIR / "fp-lib-table": "(fp_lib_table\n\t(version 7)\n\t(lib (name \"IFB01\")(type \"KiCad\")(uri \"${KIPRJMOD}/if-b01.pretty\")(options \"\")(descr \"IF-B01 local draft footprints\"))\n)\n",
        DESIGN_DIR / "if-b01.kicad_sym": "(kicad_symbol_lib\n\t(version 20250114)\n\t(generator \"eeschema\")\n\t(generator_version \"10.0\")\n" + "\n".join(line for spec in SYMBOLS.values() for line in render_symbol(spec)) + "\n)\n",
        DESIGN_DIR / "if-b01.kicad_sch": render_schematic(parts, components),
        DESIGN_DIR / "if-b01.kicad_pcb": render_pcb(parts, components),
        DESIGN_DIR / "if-b01-placement.csv": render_placement_csv(components),
        DESIGN_DIR / "if-b01-native-intent.json": render_manifest(parts, components),
    }
    files.update(footprint_sources())
    return files


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="write native KiCad design source")
    args = parser.parse_args()
    try:
        expected = sources()
    except (OSError, ValueError, csv.Error) as error:
        print(f"IF-B01 generation input error: {error}", file=sys.stderr)
        return 2
    stale = [path for path, content in expected.items() if not path.exists() or path.read_text(encoding="utf-8") != content]
    if not args.write:
        if stale:
            print("IF-B01 native source is stale or absent; run tools/generate_if_b01_kicad.py --write", file=sys.stderr)
            for path in stale:
                print(path.relative_to(ROOT), file=sys.stderr)
            return 1
        print("IF-B01 generated native source matches design inputs")
        return 0
    for path, content in expected.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
    print(f"Wrote {len(expected)} IF-B01 native draft source files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())