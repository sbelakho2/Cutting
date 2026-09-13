#!/usr/bin/env python3
"""Verify an exported KiCad netlist against the IF-B01 native-source intent."""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from pathlib import Path
from typing import Iterator


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = ROOT / "hardware" / "if-b01" / "if-b01-native-intent.json"
DEFAULT_NETLIST = ROOT / "hardware" / "if-b01" / "review" / "if-b01-netlist.net"
DEFAULT_BOM = ROOT / "hardware" / "if-b01" / "review" / "if-b01-native-bom.csv"


class NetlistError(RuntimeError):
    """An exported netlist does not match its native-source design intent."""


def tokens(text: str) -> Iterator[str]:
    pattern = re.compile(r'\s*(\(|\)|"(?:[^"\\]|\\.)*"|[^\s()]+)')
    position = 0
    while position < len(text):
        match = pattern.match(text, position)
        if not match:
            if text[position:].strip():
                raise NetlistError(f"unparseable netlist text at byte {position}")
            return
        position = match.end()
        token = match.group(1)
        if token.startswith('"'):
            yield json.loads(token)
        else:
            yield token


def parse_sexp(text: str) -> list[object]:
    stream = iter(tokens(text))

    def parse_list() -> list[object]:
        result: list[object] = []
        for token in stream:
            if token == "(":
                result.append(parse_list())
            elif token == ")":
                return result
            else:
                result.append(token)
        raise NetlistError("unexpected end of netlist")

    if next(stream, None) != "(":
        raise NetlistError("netlist must begin with an S-expression")
    result = parse_list()
    if next(stream, None) is not None:
        raise NetlistError("unexpected text after root S-expression")
    return result


def children(node: list[object], name: str) -> list[list[object]]:
    return [child for child in node[1:] if isinstance(child, list) and child and child[0] == name]


def child(node: list[object], name: str) -> list[object] | None:
    matches = children(node, name)
    return matches[0] if matches else None


def atom(node: list[object], name: str) -> str | None:
    entry = child(node, name)
    if entry is None or len(entry) != 2 or not isinstance(entry[1], str):
        return None
    return entry[1]


def parse_export(path: Path) -> tuple[set[str], dict[tuple[str, str], str], dict[str, dict[str, str]]]:
    root = parse_sexp(path.read_text(encoding="utf-8"))
    if not root or root[0] != "export":
        raise NetlistError(f"{path}: not a KiCad exported netlist")
    components_section = child(root, "components")
    nets_section = child(root, "nets")
    if components_section is None or nets_section is None:
        raise NetlistError(f"{path}: missing components or nets section")

    references: set[str] = set()
    properties: dict[str, dict[str, str]] = {}
    for component in children(components_section, "comp"):
        reference = atom(component, "ref")
        if reference is None:
            raise NetlistError("component without reference")
        if reference in references:
            raise NetlistError(f"duplicate component reference {reference}")
        references.add(reference)
        properties[reference] = {}
        for property_node in children(component, "property"):
            name = atom(property_node, "name")
            value = atom(property_node, "value")
            if name is not None and value is not None:
                properties[reference][name] = value

    pin_nets: dict[tuple[str, str], str] = {}
    for net in children(nets_section, "net"):
        net_name = atom(net, "name")
        if net_name is None:
            raise NetlistError("net without a name")
        for node in children(net, "node"):
            reference = atom(node, "ref")
            pin = atom(node, "pin")
            if reference is None or pin is None:
                raise NetlistError(f"malformed node on net {net_name}")
            key = reference, pin
            if key in pin_nets:
                raise NetlistError(f"{reference}-{pin} appears on both {pin_nets[key]} and {net_name}")
            pin_nets[key] = net_name
    return references, pin_nets, properties


def parse_bom(path: Path) -> dict[str, dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None or "Reference" not in reader.fieldnames:
            raise NetlistError(f"{path}: expected a KiCad BOM with a Reference column")
        rows = {row["Reference"]: row for row in reader}
    if len(rows) == 0:
        raise NetlistError(f"{path}: BOM has no source records")
    return rows


def canonical_net_name(name: str | None) -> str | None:
    """KiCad exports root-sheet local labels as /NAME; intent uses NAME."""
    return name[1:] if name is not None and name.startswith("/") else name


def verify(manifest_path: Path, netlist_path: Path, bom_path: Path) -> str:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    references, pin_nets, properties = parse_export(netlist_path)
    expected_references = set(manifest["netlist_references"])
    if references != expected_references:
        raise NetlistError(
            "BOM reference parity failed: "
            f"missing={sorted(expected_references - references)}, "
            f"unexpected={sorted(references - expected_references)}"
        )

    expected_pins = manifest["component_pin_net_expectations"]
    for reference, pins in expected_pins.items():
        for pin, expected_net in pins.items():
            actual_net = canonical_net_name(pin_nets.get((reference, pin)))
            if expected_net is None:
                if actual_net is not None and not actual_net.startswith("unconnected-"):
                    raise NetlistError(f"{reference}-{pin}: expected intentional no-connect, got {actual_net!r}")
                continue
            if actual_net != expected_net:
                raise NetlistError(f"{reference}-{pin}: expected {expected_net!r}, got {actual_net!r}")

    for reference, pins in manifest["connector_net_expectations"].items():
        for pin, expected_net in pins.items():
            actual_net = canonical_net_name(pin_nets.get((reference, pin)))
            if actual_net != expected_net:
                raise NetlistError(f"connector parity failed: {reference}-{pin}: expected {expected_net!r}, got {actual_net!r}")

    for reference in ("C2", "C3"):
        actual_mpn = properties[reference].get("MPN")
        if actual_mpn != "GRM188R71H104KA93D":
            raise NetlistError(f"{reference}: expected 50 V COM bypass MPN GRM188R71H104KA93D, got {actual_mpn!r}")

    bom = parse_bom(bom_path)
    expected_bom_references = set(manifest["bom_references"])
    if set(bom) != expected_bom_references:
        raise NetlistError(
            "native BOM reference parity failed: "
            f"missing={sorted(expected_bom_references - set(bom))}, "
            f"unexpected={sorted(set(bom) - expected_bom_references)}"
        )
    for reference, expected_mpn in manifest["component_mpn_expectations"].items():
        actual_mpn = bom[reference].get("MPN")
        if actual_mpn != expected_mpn:
            raise NetlistError(f"native BOM {reference}: expected MPN {expected_mpn!r}, got {actual_mpn!r}")

    intentional_no_connects = sum(len(pins) for pins in manifest["intentional_no_connects"].values())
    return (
        f"IF-B01 native source parity passed: {len(bom)} BOM references, {len(references)} netlist references, "
        f"{len(pin_nets)} connected symbol pins, {intentional_no_connects} intentional no-connects"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--netlist", type=Path, default=DEFAULT_NETLIST)
    parser.add_argument("--bom", type=Path, default=DEFAULT_BOM)
    args = parser.parse_args()
    try:
        print(verify(args.manifest, args.netlist, args.bom))
    except (OSError, ValueError, NetlistError) as error:
        print(f"IF-B01 native netlist parity failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())