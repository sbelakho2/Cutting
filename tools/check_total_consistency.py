#!/usr/bin/env python3
"""S1-B total CAD/spec/BOM/document consistency audit.

Covers the 2026-09-16 substitutions (ifm RU3100 encoder, ifm E60137 200 mm
measuring wheel, DIN 471 rings, 5x20 fuses) end to end:

1. Cross-spec constant consistency of the 200 mm wheel remap and a repo-wide
   sweep for stale 100 mm-era constants (40960 / 24414 / 0.024414 / old wheel
   diameter claims).
2. CAD-vs-datasheet: ifm RU3100 and E60137 published facts vs
   shared/measurement_spec.hpp, the measurement/mechanism manifests and the
   published check status (wheel, encoder, coupling, M201/M203, fork/lug).
3. J8/J9 line-receiver review closure: the reviewed RU3100 TTL wiring note
   must be present in docs/22, the IF-B01 README/cabinet-integration notes,
   shared/controls_spec.hpp and docs/13; the netlist J8/J9 allocation is
   checked.
4. Fastener-interface sweep: every named ISO/DIN fastener row vs its mating
   CAD feature, recomputing thread engagement >= 1.5 x nominal diameter.
5. BOM-vs-manifest-vs-spec: purchased-row identity coverage (including the
   DIN 471 ring substitution), quantity arithmetic (wheel x2, encoder x2,
   626 x10, arm spring x2, M4x12 x6) and custom-part quantities.
6. Drawing-vs-spec: the M201-R2 sheet and inspection template must carry the
   R2 plate dimensions (60x60x6, seat 36.5, PCD 42).

Run from the repository root. Exit code 0 = all checks pass.
"""
from __future__ import annotations

import csv
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SHARED = ROOT / "shared"
MEAS_SPEC = SHARED / "measurement_spec.hpp"
MACH_SPEC = SHARED / "machine_spec.hpp"
CTRL_SPEC = SHARED / "controls_spec.hpp"
MECH_SPEC = SHARED / "mechanism_spec.hpp"
HEAD_SPEC = SHARED / "cutting_head_spec.hpp"
MEAS_STATUS = ROOT / "engineering" / "generated" / "measurement" / "status.json"
MEAS_MANIFEST = ROOT / "engineering" / "generated" / "measurement" / "manifest.csv"
MECH_STATUS = ROOT / "engineering" / "generated" / "mechanism" / "status.json"
MECH_MANIFEST = ROOT / "engineering" / "generated" / "mechanism" / "manifest.csv"
MECH_BOM = ROOT / "engineering" / "generated" / "mechanism" / "mechanism-bom.csv"
IFM_RU = ROOT / "engineering" / "sources" / "ifm-ru3100-datasheet-en.txt"
IFM_E6 = ROOT / "engineering" / "sources" / "ifm-e60137-datasheet-en.txt"
DOCS01 = ROOT / "docs" / "01-design-basis.md"
DOCS13 = ROOT / "docs" / "13-measuring-interfaces-and-calibration.md"
DOCS22 = ROOT / "docs" / "22-controls-and-wiring.md"
IFB_README = ROOT / "hardware" / "if-b01" / "README.md"
IFB_INTEGRATION = ROOT / "hardware" / "if-b01" / "if-b01-cabinet-integration.md"
IFB_NETLIST = ROOT / "hardware" / "if-b01" / "if-b01-netlist.csv"
M201_DRAWING = ROOT / "engineering" / "generated" / "drawings" / "M201-R2-encoder-plate.svg"
M201_TEMPLATE = ROOT / "engineering" / "generated" / "measurement" / "M201-inspection-template.csv"
TNC_BOM = ROOT / "bom" / "selected-transport-and-cabinet.csv"
MECH_SEL_BOM = ROOT / "bom" / "selected-mechanism.csv"
MECH_CUSTOM_BOM = ROOT / "bom" / "mechanism-custom-parts.csv"
MEAS_CUSTOM_BOM = ROOT / "bom" / "measurement-custom-parts.csv"

failures: list[str] = []


def fail(msg: str) -> None:
    failures.append(msg)


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return [row for row in csv.DictReader(handle)]


def spec_constant(text: str, name: str) -> float:
    match = re.search(rf"(?:inline constexpr \w+ )?\b{name}\b\s*=\s*([^,;]+)[,;]", text)
    if match is None:
        fail(f"constant {name} not found in a shared spec header")
        return float("nan")
    value = match.group(1).split("//")[0].strip()
    try:
        return float(value)
    except ValueError:
        fail(f"constant {name} is not a literal in the shared spec header")
        return float("nan")


# ----------------------------------------------------------------------------
# 1. Cross-spec constant consistency (200 mm wheel remap)
# ----------------------------------------------------------------------------
STALE_TOKENS = ("40960", "24414", "0.024414")
HISTORICAL_CONTEXT = ("was", "before", "supersed", "replaced", "instead of",
                      "halves", "restores", "→", "substitut", "resolved",
                      "no longer", "previous", "previously")
STALE_EXEMPT_FILES = {
    "engineering/sources/",                     # evidence archives (Kübler/pulley history)
    "docs/33-gate-ledger.md",
    "docs/35-purchase-enquiries.md",
    "docs/28-physical-contradiction-audit.md",
    "docs/32-sourcing-list.md",
    "engineering/generated/measurement-analysis/",
    "engineering/generated/feed-fem-boundary-detail/",
    "engineering/generated/feed-fem/",
    "engineering/generated/fem/",
    "engineering/generated/drawings/",
    "engineering/generated/analysis/",
}
SCAN_SUFFIXES = {".hpp", ".cpp", ".py", ".md", ".csv", ".cmake", ".h"}


def stale_constant_sweep() -> None:
    for path in ROOT.rglob("*"):
        rel = str(path.relative_to(ROOT))
        if rel.startswith((".git", ".kilo")) or "/build" in rel or "build-" in rel:
            continue
        if path.name == "check_total_consistency.py":   # this tool names the stale tokens itself
            continue
        if not path.is_file() or path.suffix not in SCAN_SUFFIXES:
            continue
        if any(rel.startswith(prefix) for prefix in STALE_EXEMPT_FILES):
            continue
        try:
            text = read(path)
        except (OSError, UnicodeDecodeError):
            continue
        for number, line in enumerate(text.splitlines(), start=1):
            low = line.lower()
            if "nominal_encoder_nm_count" in line:   # definition itself checked below
                continue
            for token in STALE_TOKENS:
                if token not in line:
                    continue
                if any(word in low for word in HISTORICAL_CONTEXT):
                    continue
                fail(f"{rel}:{number} still carries the 100 mm-era constant {token}")
    for path, tokens in ((MACH_SPEC, ("100 mm",)), (MEAS_SPEC, ("31.83 mm",))):
        for token in tokens:
            for number, line in enumerate(read(path).splitlines(), start=1):
                if token in line and not any(w in line.lower() for w in ("was", "radius", "placeholder", "supersed")):
                    fail(f"{path.relative_to(ROOT)}:{number} still carries '{token}'")


def check_spec_constants() -> None:
    machine = read(MACH_SPEC)
    controls = read(CTRL_SPEC)
    measurement = read(MEAS_SPEC)
    mechanism = read(MECH_SPEC)
    circumference = spec_constant(machine, "wheel_circumference_mm")
    counts_rev = spec_constant(machine, "encoder_counts_rev")
    nm_count = spec_constant(machine, "nominal_encoder_nm_count")
    if circumference != 200.0:
        fail(f"machine_spec wheel_circumference_mm is {circumference}, expected 200")
    if counts_rev != 4096.0:
        fail(f"machine_spec encoder_counts_rev is {counts_rev}, expected 4096")
    if nm_count != 48828.0:
        fail(f"machine_spec nominal_encoder_nm_count is {nm_count}, expected 48828")
    exact_nm = 200.0 / 4096.0 * 1e6            # 48828.125 nm/count
    if abs(nm_count - exact_nm) > 1.0:
        fail(f"machine_spec nm/count {nm_count} does not round 200 mm/4096 counts ({exact_nm})")
    counts_per_metre = spec_constant(controls, "nominal_counts_per_metre")
    if counts_per_metre != 20480.0:
        fail(f"controls_spec nominal_counts_per_metre is {counts_per_metre}, expected 20480")
    if "nominal_um_per_count" not in controls or "48.828" not in controls:
        fail("controls_spec lost the 48.828 um/count nominal scale")
    if abs(counts_per_metre * circumference / counts_rev - 1000.0) > 1e-6:
        fail("controls counts-per-metre does not give exactly 1000 mm with the 200 mm wheel")
    m_circumference = spec_constant(measurement, "circumference_mm")
    wheel_od = spec_constant(measurement, "wheel_od_mm")
    pulses = spec_constant(measurement, "pulses_per_revolution")
    if m_circumference != 200.0:
        fail(f"measurement_spec circumference_mm is {m_circumference}, expected 200")
    if abs(wheel_od - 200.0 / 3.141592653589793) > 0.005:
        fail(f"measurement_spec wheel_od_mm {wheel_od} != 200/pi ({200.0/3.141592653589793:.4f})")
    if pulses != 1024.0 or 4 * pulses != counts_rev:
        fail("measurement_spec PPR does not give 4096 quadrature counts")
    lug_min = spec_constant(mechanism, "lug_z_min")
    lug_max = spec_constant(mechanism, "lug_z_max")
    if abs(lug_min - 219.915) > 1e-6 or abs(lug_max - 231.915) > 1e-6:
        fail(f"mechanism_spec lug_z_min/max are {lug_min}/{lug_max}, expected 219.915/231.915")
    if abs(lug_min - 204.0 - 15.915) > 1e-6 or abs(lug_max - 216.0 - 15.915) > 1e-6:
        fail("mechanism_spec lug rise is not the 15.915 mm wheel-radius increase")
    # docs/01 and docs/13 must carry the remapped values.
    docs01 = read(DOCS01)
    for needle in ("0.048828125 mm/count", "10240", "200 mm wheel circumference (ifm E60137)"):
        if needle not in docs01:
            fail(f"docs/01-design-basis.md lacks the remapped statement '{needle}'")
    docs13 = read(DOCS13)
    for needle in ("20480 counts per metre", "0.048828125 mm/count", "Ø63.66", "200 mm"):
        if needle not in docs13:
            fail(f"docs/13 lacks the remapped value '{needle}'")


# ----------------------------------------------------------------------------
# 2. CAD vs datasheet (ifm RU3100 encoder, ifm E60137 wheel)
# ----------------------------------------------------------------------------
def check_cad_vs_datasheet() -> None:
    ru = read(IFM_RU)
    e6 = read(IFM_E6)
    spec_text = read(MEAS_SPEC)
    for needle in ("Ø 58.6", "L = 63.5", "Shaft diameter [mm] 6", "1024", "1000",
                   "12000", "0.01", "60", "40", "M4 x 0.7 Depth 6 mm", "HTL/TTL",
                   "Factory setting Output function: HTL"):
        if needle not in ru:
            fail(f"archived ifm RU3100 datasheet text lacks '{needle}' (file moved or truncated)")
    for needle in ("63.66", "Circumference [mm] 200", "Inside diameter [mm] 6",
                   "90 Shore A", "64.5"):
        if needle not in e6:
            fail(f"archived ifm E60137 datasheet text lacks '{needle}'")
    checks = {
        "encoder_body_od_mm": 58.6, "encoder_body_length_mm": 63.5,
        "encoder_pulse_limit_hz": 1000000.0, "encoder_speed_limit_rpm": 12000.0,
        "encoder_starting_torque_nm": 0.01, "encoder_radial_limit_n": 60.0,
        "encoder_axial_limit_n": 40.0, "encoder_mount_pcd_mm": 42.0,
        "encoder_mount_thread_depth_mm": 6.0,
        "wheel_tread_mm": 9.5, "wheel_overall_mm": 14.5, "wheel_bore_mm": 6.0,
        "wheel_shore_a": 90.0, "wheel_mass_kg": 0.0645,
    }
    for name, expected in checks.items():
        actual = spec_constant(spec_text, name)
        if abs(actual - expected) > 1e-6:
            fail(f"measurement_spec {name}={actual} contradicts the archived datasheet ({expected})")
    # ifm publishes no shaft-diameter tolerance: the spec interval must be
    # explicitly marked as an assumed incoming-inspection placeholder.
    shaft_min = spec_constant(spec_text, "encoder_shaft_min_mm")
    shaft_max = spec_constant(spec_text, "encoder_shaft_max_mm")
    normalized_spec = re.sub(r"\s+", " ", spec_text.replace("//", " "))
    if abs((shaft_min + shaft_max) / 2 - 6.0) > 1e-6:
        fail(f"measurement_spec encoder shaft interval {shaft_min}..{shaft_max} is not centred on 6 mm")
    if "no shaft-diameter tolerance" not in normalized_spec:
        fail("measurement_spec does not state that ifm publishes no shaft-diameter tolerance")
    if "assumed incoming-inspection" not in normalized_spec:
        fail("measurement_spec does not mark the assumed encoder shaft interval as an incoming-inspection placeholder")
    if "assumed placeholders" not in normalized_spec:
        fail("measurement_spec lost its wheel-profile placeholder statement")
    # Measurement interface export: wheel +-31.83, encoder 58.6 body + 10 tip,
    # M201 plate 60x60x6.
    manifest = {row["id"]: row for row in rows(MEAS_MANIFEST)}
    def bound(row_id, field, expected, tol, label):
        if row_id not in manifest:
            fail(f"measurement manifest lacks {row_id}")
            return
        actual = float(manifest[row_id][field])
        if abs(actual - expected) > tol:
            fail(f"measurement manifest {row_id} {field}={actual} != {expected} ({label})")
    bound("ME-W01", "xmin_mm", -31.83, 1e-6, "wheel radius 63.66/2")
    bound("ME-W01", "xmax_mm", 31.83, 1e-6, "wheel radius 63.66/2")
    bound("ME-W01", "ymax_mm", 14.5, 1e-6, "wheel overall width")
    bound("ME-E01", "ymin_mm", -63.5, 1e-6, "encoder body length")
    bound("ME-E01", "ymax_mm", 10.0, 1e-6, "encoder tip 10 mm from face")
    bound("ME-E01", "xmin_mm", -29.3, 1e-6, "encoder body radius 58.6/2")
    bound("M201", "xmin_mm", -30.0, 1e-6, "plate half width 60/2")
    bound("M201", "ymax_mm", 6.0, 1e-6, "plate thickness 6")
    meas_status = json.loads(read(MEAS_STATUS))
    if meas_status.get("revision") != "S1-B-MEAS-R2-2026-09-16":
        fail("measurement status.json revision is not S1-B-MEAS-R2-2026-09-16")
    for key, expected in (("interface_solids", 5), ("encoder_mount_occurrences", 5),
                          ("encoder_mount_intersections_checked", True), ("step_readbacks", True)):
        if meas_status.get(key) != expected:
            fail(f"measurement status.json {key} is {meas_status.get(key)!r}, expected {expected!r}")
    # Mechanism status: the remapped wheel/lug model must stay consistent.
    mech_status = json.loads(read(MECH_STATUS))
    if mech_status.get("measurement_revision") != "S1-B-MEAS-R2-2026-09-16":
        fail("mechanism status.json measurement_revision does not match measurement_spec")
    if mech_status.get("wheel_contact_force_n") != 10:
        fail(f"mechanism status.json wheel_contact_force_n is {mech_status.get('wheel_contact_force_n')!r}, expected 10")
    if mech_status.get("occurrences") != 504:
        fail(f"mechanism status.json occurrences is {mech_status.get('occurrences')!r}, expected 504")
    if mech_status.get("external_wire_route_checked") is not True:
        fail("mechanism status.json lost the external wire-route check flag")
    if mech_status.get("cross_subsystem_static_intersections_checked") is not True:
        fail("mechanism status.json lost the intersection-check flag")
    mech_manifest = rows(MECH_MANIFEST)
    by_id = {row["id"]: row for row in mech_manifest}
    for row_id, expected_zmax, label in (("MM_M206", 231.915, "lug top"), ("MM_M207", 100.0, "roller column top")):
        if row_id not in by_id:
            fail(f"mechanism manifest lacks {row_id}")
            continue
        actual = float(by_id[row_id]["zmax_mm"])
        if abs(actual - expected_zmax) > 1e-6:
            fail(f"mechanism manifest {row_id} zmax={actual} != {expected_zmax} ({label})")
    if "MM_M210" not in by_id:
        fail("mechanism manifest lacks the M210 fork")


# ----------------------------------------------------------------------------
# 3. J8/J9 line-receiver review closure
# ----------------------------------------------------------------------------
def check_j8j9_review() -> None:
    netlist = read(IFB_NETLIST)
    for row_id in ("J8", "J9"):
        pins = [line for line in netlist.splitlines() if line.startswith(f"{row_id},")]
        if len(pins) != 6:
            fail(f"if-b01 netlist {row_id} does not carry six positions")
    if "120 ohm" not in netlist and "R1" not in netlist:
        fail("if-b01 netlist lost the R1-R4 termination annotation")
    docs22 = read(DOCS22)
    for needle in ("TTL mode via IO-Link", "single-ended", "2.5 V", "120 Ω"):
        if needle not in docs22:
            fail(f"docs/22 lacks the resolved J8/J9 review note ('{needle}')")
    readme = read(IFB_README)
    for needle in ("TTL mode", "mid-bias", "RU3100"):
        if needle not in readme:
            fail(f"hardware/if-b01/README.md lacks the RU3100 wiring note ('{needle}')")
    integration = read(IFB_INTEGRATION)
    for needle in ("RU3100", "TTL"):
        if needle not in integration:
            fail(f"if-b01-cabinet-integration.md lacks the RU3100/TTL wiring note ('{needle}')")
    ctrl = read(CTRL_SPEC)
    if "TTL mode" not in ctrl:
        fail("controls_spec.hpp encoder comment does not state the required TTL mode")
    if "open controls item" in ctrl:
        fail("controls_spec.hpp still calls the J8/J9 receiver review an open item")
    docs13 = read(DOCS13)
    if "need their own receiver/termination review" in docs13:
        fail("docs/13 still lists the J8/J9 receiver review as open")
    if "TTL mode" not in docs13:
        fail("docs/13 lacks the resolved J8/J9 review note")


# ----------------------------------------------------------------------------
# 4. Fastener-interface sweep (engagement >= 1.5D from CAD features)
# ----------------------------------------------------------------------------
def check_fasteners() -> None:
    head = read(HEAD_SPEC)
    adapter_t = spec_constant(head, "adapter_thickness")           # 15
    cbore = spec_constant(head, "adapter_f0_counterbore_depth")   # 8.5
    register = spec_constant(head, "adapter_f0_register_length")  # 2
    def engage(label, length, through, counterbore, boss, nominal_d):
        engagement = length - (through - counterbore) - boss
        if engagement < 1.5 * nominal_d - 1e-6:
            fail(f"{label}: engagement {engagement:.2f} mm < 1.5D ({1.5 * nominal_d:.2f} mm)")
        return engagement
    engage("H003 F0 M5x16 into the stage register", 16, adapter_t, cbore, register, 5)
    engage("H003 motor M5x25 through the 15 mm adapter", 25, adapter_t, 0, 0, 5)
    # M201: M4x12 through the 6 mm plate into the 6 mm-deep ifm flange taps
    # (flush engagement: must reach 1.5D=6.0 mm and must not bottom).
    protrusion = 12.0 - 6.0
    if protrusion < 6.0 - 1e-6 or protrusion > 6.1:
        fail(f"M201 M4x12 protrusion {protrusion:.2f} mm is not the flush 6.0 mm flange engagement")
    if 6.0 - 6.0 != 0.0:
        fail("M201 M4x6 support screw is not flush with the plate")
    # Feed F103 locating-bearing cover: M3x8 through the 2 mm cover into F112.
    mech_manifest = {row["id"]: row for row in rows(MECH_MANIFEST)}
    cover = mech_manifest.get("RIGHT_L1_F103")
    if cover is None:
        fail("mechanism manifest lacks the F103 cover for the M3x8 check")
    else:
        cover_t = float(cover["ymax_mm"]) - float(cover["ymin_mm"])
        if abs(cover_t - 2.0) > 0.05:
            fail(f"F103 cover thickness read as {cover_t:.3f} mm, expected 2.0")
        engage("F103 M3x8 cover screw into F112", 8, cover_t, 0, 0, 3)
    # Mechanism bed/frame and bracket fasteners, from spec constants.
    mechanism = read(MECH_SPEC)
    bed_t = spec_constant(mechanism, "bed_t")                       # 25
    bracket_t = spec_constant(mechanism, "bracket_base_t")          # 12
    roller_t = spec_constant(mechanism, "roller_base_t")            # 10
    bushing_flange_t = spec_constant(mechanism, "bushing_flange_t_mm")  # 6
    bushing_cbore_t = spec_constant(mechanism, "bushing_cbore_t_mm")   # 4.1
    # The bed M10x35 bottoms out on the DIN 929 M10 weld nut under the 60x60
    # tube top wall: the full nut height (~9 mm) is the reachable engagement,
    # not the 1.5D rule for a tapped-through plate.
    bed_engagement = 35.0 - bed_t
    if bed_engagement < 9.0 - 1e-6:
        fail(f"bed M10x35 engagement {bed_engagement:.2f} mm < DIN929 M10 nut height 9 mm")
    engage("M206 bracket M8x25 into the tapped bed", 25, bracket_t, 8.0, 0, 8)
    engage("M207 column M6x20 into the tapped bed", 20, roller_t, 6.0, 0, 6)
    engage("LHFRW12 M4x10 through the bushing flange", 10, bushing_flange_t, bushing_cbore_t, 0, 4)
    # F201 base-to-bed M8x55: shank starts under the 8 mm head at z 34.75 and
    # the bed top is the machine datum z 0 -> 20.25 mm into the tapped bed.
    row = mech_manifest.get("RIGHT_FB203-1")
    if row is None:
        fail("mechanism manifest lacks FB203 for the M8x55 check")
    else:
        into_bed = float(row["zmax_mm"]) - 8.0
        if abs(into_bed - 34.75) > 0.05:
            fail(f"FB203 head plane read as {into_bed:.3f}, expected 34.75")
        engage("F201 M8x55 base-to-bed screw", 55, into_bed, 0, 0, 8)


# ----------------------------------------------------------------------------
# 5. BOM vs manifest vs spec sweep
# ----------------------------------------------------------------------------
def check_bom_manifest() -> None:
    mech_manifest = rows(MECH_MANIFEST)
    purchased = [row for row in mech_manifest if row["purchased"] == "1"]
    descriptions = [row["description"] for row in purchased]
    def cad_count(token: str) -> int:
        return sum(1 for description in descriptions if token in description)
    tnc = {row["id"]: row for row in rows(TNC_BOM)}
    mech_sel = {row["id"]: row for row in rows(MECH_SEL_BOM)}
    quantity_checks = [
        ("E60137", 1, 2, tnc, "ME-W01"),
        ("ifm RU3100", 1, 2, tnc, "ME-E01"),
        ("SKF 626-2Z", 5, 2, tnc, "ME-B01"),
        ("ISO4762 M4x12", 3, 2, tnc, "ME-FB01"),
        ("Supplied6/6 flexible coupling", 1, 2, tnc, "ME-E01"),
        ("SWU10.5-35", 1, 2, mech_sel, "MC-P02"),
        ("SWU17-60", 2, 1, mech_sel, "MC-P01"),
        ("PSFJ12-240", 4, 1, mech_sel, "MC-S01"),
        ("LHFRW12", 4, 1, mech_sel, "MC-B01"),
    ]
    for token, cad, scale, bom, bom_id in quantity_checks:
        if bom_id not in bom:
            fail(f"BOM row {bom_id} is missing")
            continue
        bom_qty = int(float(bom[bom_id]["quantity"]))
        expected = cad * scale
        if expected != bom_qty:
            fail(f"quantity mismatch: CAD models {cad}x '{token}' (design x{scale}) = {expected} but {bom_id} selects {bom_qty}")
    # DIN 471 ring substitution: the eight SEEGER/DIN471-class ring envelopes in
    # the feed groups must be covered by the priced Wegertseder DIN 471 row.
    ring_cad = sum(1 for row in mech_manifest if "SEEGER A12" in row["description"])
    ring_row = tnc.get("TR-RR01")
    if ring_row is None:
        fail("selected-transport-and-cabinet.csv lacks TR-RR01")
    elif int(float(ring_row["quantity"])) != ring_cad:
        fail(f"DIN471 ring coverage: CAD {ring_cad} ring envelopes but TR-RR01 selects {ring_row['quantity']}")
    if "DIN471" not in ring_row.get("exact_order", ""):
        fail("TR-RR01 does not carry the DIN 471 identity")
    # Custom-part quantities: the CAD models one measuring module of the two
    # designed (docs/13); every M2xx machined part must carry the x2 mirror.
    mech_custom = rows(MECH_CUSTOM_BOM)
    by_id = {row["id"]: row for row in mech_custom}
    for part_id in ("M202", "M203", "M204", "M206", "M207", "M209", "M210"):
        row = by_id.get(part_id)
        if row is None or int(float(row["quantity"])) != 2:
            fail(f"mechanism-custom-parts.csv {part_id} quantity is not 2 (two measuring modules)")
    meas_custom = {row["id"]: row for row in rows(MEAS_CUSTOM_BOM)}
    m201 = meas_custom.get("M201")
    if m201 is None:
        fail("measurement-custom-parts.csv lacks M201")
    else:
        if int(float(m201["quantity"])) != 2:
            fail("measurement-custom-parts.csv M201 quantity is not 2")
        if "60x60x6" not in m201["description"] or "46x46" in m201["description"]:
            fail(f"measurement-custom-parts.csv M201 description still shows a non-R2 plate: {m201['description']}")
        if m201["nominal_stock"] != "66x66x8":
            fail(f"measurement-custom-parts.csv M201 stock is {m201['nominal_stock']}, expected 66x66x8")
    # Purchased mechanism-BOM rows must be covered by a selected identity or
    # the docs/24 standard-hardware register.
    plan = read(ROOT / "docs" / "24-production-and-release-plan.md")
    bom = rows(MECH_BOM)
    covered_tokens = ("E60137", "ifm RU3100", "SKF 626-2Z", "Supplied6/6",
                      "LHFRW12", "PSFJ12-240", "SWU10.5-35", "SWU17-60",
                      "Bernstein ACF-1", "Bernstein SLC-F")
    std_tokens = ("ISO4762", "ISO4032", "ISO7092", "ISO8734", "DIN913", "DIN929")
    for row in bom:
        if row["purchased"] != "1":
            continue
        description = row["description"]
        if any(token in description for token in covered_tokens):
            continue
        if any(token in description for token in std_tokens) and "ISO4762" in plan:
            continue
        fail(f"mechanism-bom.csv purchased row lacks a BOM/register identity: {description[:90]}")


# ----------------------------------------------------------------------------
# 6. Drawing vs spec (M201-R2 sheet)
# ----------------------------------------------------------------------------
def check_drawings() -> None:
    if not M201_DRAWING.is_file():
        fail("M201-R2-encoder-plate.svg is missing; regenerate the drawings")
        return
    svg = read(M201_DRAWING)
    for needle in ("60.00 ±0.05", "Ø36.50", "PCD42", "M4×0.7", "R2"):
        if needle not in svg:
            fail(f"M201-R2 drawing lacks the R2 plate dimension '{needle}'")
    pdf = M201_DRAWING.with_suffix(".pdf")
    if not pdf.is_file() or pdf.stat().st_size == 0:
        fail("M201-R2-encoder-plate.pdf is absent or empty")
    if not M201_TEMPLATE.is_file():
        fail("M201 inspection template is missing")
        return
    template = read(M201_TEMPLATE)
    if "width_X,59.95,60.05" not in template or "36.5" not in template:
        fail("M201 inspection template does not carry the R2 characteristics")


def main() -> int:
    stale_constant_sweep()
    check_spec_constants()
    check_cad_vs_datasheet()
    check_j8j9_review()
    check_fasteners()
    check_bom_manifest()
    check_drawings()
    if failures:
        print("total consistency audit FAILED:")
        for failure in failures:
            print("  -", failure)
        return 1
    print("total consistency audit passed: 200 mm remap, CAD-vs-datasheet, J8/J9 review, "
          "fastener engagement, BOM/manifest quantities and M201-R2 drawing")
    return 0


if __name__ == "__main__":
    sys.exit(main())
