#!/usr/bin/env python3
"""Verify and export the IF-B01 KiCad source into an explicitly draft package.

This is deliberately a source-output pipeline, not a release command.  It
refuses stale generated source or non-clean ERC/DRC/parity reports, then writes
Gerber, drill, placement, assembly-view, board-only STEP, IPC-D-356, BOM and
hash manifest files beneath ``hardware/if-b01/manufacturing``.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DESIGN_DIR = ROOT / "hardware" / "if-b01"
REVIEW_DIR = DESIGN_DIR / "review"
OUTPUT_DIR = DESIGN_DIR / "manufacturing" / "draft-r2-2026-09-13"
PCB = DESIGN_DIR / "if-b01.kicad_pcb"
SCHEMATIC = DESIGN_DIR / "if-b01.kicad_sch"
REVISION = "S1-B-IF-B01-NATIVE-DRAFT-R2-2026-09-13"


def run(*arguments: str) -> None:
    """Run a checked command in the repository root while preserving output."""
    print("+", " ".join(arguments))
    subprocess.run(arguments, cwd=ROOT, check=True)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require_files(directory: Path, names: tuple[str, ...], description: str) -> None:
    """Reject an incomplete KiCad export before it reaches the draft package."""
    missing = [name for name in names if not (directory / name).is_file()]
    if missing:
        raise RuntimeError(f"missing {description}: {', '.join(missing)}")


def write_readme(directory: Path) -> None:
    (directory / "README.md").write_text(
        "# IF-B01 manufacturing output — draft only\n\n"
        f"**Source revision:** `{REVISION}`  \n"
        "**Status:** source-generated DRAFT. It is **not** a fabrication, "
        "assembly, energisation, cabinet-integration, safety, or machine-release package.\n\n"
        "This directory is generated only after the source generator, KiCad ERC, "
        "native BOM/netlist parity check, and physical plus schematic-parity PCB DRC pass. "
        "The exact files and SHA-256 values are in `manifest.json`.\n\n"
        "## Contents\n\n"
        "- `gerbers/` — four copper layers, paste, masks, silkscreens and outline plots.\n"
        "- `drill/` — Excellon PTH/NPTH data, map and drill report.\n"
        "- `assembly/` — CSV placement data and top/bottom visual assembly exports.\n"
        "- `mechanical/` — board-only STEP representation; custom footprint 3D models are not present.\n"
        "- `fabrication/` — IPC-D-356 electrical test netlist and board statistics.\n"
        "- `bom/` — KiCad-exported fabrication BOM.\n\n"
        "Before any quotation, ordering, fabrication or assembly, complete the human "
        "electrical/mechanical review, confirm the unusually large 750 x 400 mm draft "
        "panel dimensions and fabricator capability, validate every footprint and "
        "connector, and follow all release holds in the IF-B01 source review.\n",
        encoding="utf-8",
    )


def write_manifest(directory: Path) -> None:
    source_files = [
        DESIGN_DIR / "README.md",
        DESIGN_DIR / "if-b01.kicad_pro",
        DESIGN_DIR / "if-b01.kicad_sch",
        DESIGN_DIR / "if-b01.kicad_pcb",
        DESIGN_DIR / "if-b01.kicad_sym",
        DESIGN_DIR / "sym-lib-table",
        DESIGN_DIR / "fp-lib-table",
        DESIGN_DIR / "if-b01-bom.csv",
        DESIGN_DIR / "if-b01-netlist.csv",
        DESIGN_DIR / "if-b01-placement.csv",
        DESIGN_DIR / "if-b01-native-intent.json",
        DESIGN_DIR / "if-b01-assembly.md",
        DESIGN_DIR / "if-b01-test.md",
        DESIGN_DIR / "if-b01-cabinet-integration.md",
        REVIEW_DIR / "if-b01-native-source-review.md",
        ROOT / "bom" / "selected-controls.csv",
        ROOT / "bom" / "selected-cabinet-wiring.csv",
        ROOT / "shared" / "controls_spec.hpp",
        ROOT / "shared" / "cabinet_controls_spec.hpp",
        ROOT / "docs" / "22-controls-and-wiring.md",
        ROOT / "docs" / "23-cabinet-electrical-assembly.md",
        ROOT / "tools" / "generate_if_b01_kicad.py",
        ROOT / "tools" / "check_if_b01_kicad.py",
        Path(__file__),
    ]
    source_files.extend(sorted((DESIGN_DIR / "if-b01.pretty").glob("*.kicad_mod")))
    files = sorted(path for path in directory.rglob("*") if path.is_file() and path.name != "manifest.json")
    tool_version = subprocess.check_output(["kicad-cli", "--version"], text=True, cwd=ROOT).strip()
    document = {
        "schema": 1,
        "revision": REVISION,
        "status": "DRAFT — source outputs only; not a fabrication or machine release",
        "kicad_cli": tool_version,
        "source_sha256": {str(path.relative_to(ROOT)): sha256(path) for path in source_files},
        "output_sha256": {
            str(path.relative_to(directory)): {"bytes": path.stat().st_size, "sha256": sha256(path)}
            for path in files
        },
        "verification_gates": [
            "Generated native source is current.",
            "KiCad ERC reports zero violations.",
            "Native KiCad BOM/netlist parity matches the controlled intent manifest.",
            "KiCad physical DRC reports zero violations and zero unconnected pads.",
            "KiCad schematic-parity DRC reports zero issues.",
        ],
        "release_holds": [
            "Human electrical and mechanical review, including footprint, connector, stack-up, power-loading and return-path review.",
            "Fabricator panel-size, stack-up, impedance, copper, drill, clearance and assembly-capability confirmation.",
            "Bare-board inspection and the isolated IF-B01 fixture procedure with recorded actual results.",
            "Separate cabinet, DC-energy, PE, guarding, EMC and functional-safety design and qualification.",
        ],
    }
    (directory / "manifest.json").write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def export(staging_dir: Path) -> None:
    for relative in ("gerbers", "drill", "assembly", "mechanical", "fabrication", "bom"):
        (staging_dir / relative).mkdir(parents=True, exist_ok=True)

    erc_report = REVIEW_DIR / "if-b01-erc.rpt"
    netlist = REVIEW_DIR / "if-b01-netlist.net"
    native_bom = REVIEW_DIR / "if-b01-native-bom.csv"
    drc_report = REVIEW_DIR / "if-b01-pcb-drc.rpt"
    run(sys.executable, str(ROOT / "tools" / "generate_if_b01_kicad.py"))
    run("kicad-cli", "sch", "erc", "--severity-all", "--exit-code-violations", "--output", str(erc_report), str(SCHEMATIC))
    run("kicad-cli", "sch", "export", "netlist", "--format", "kicadsexpr", "--output", str(netlist), str(SCHEMATIC))
    run(
        "kicad-cli", "sch", "export", "bom", "--output", str(native_bom),
        "--fields", "Reference,Value,Footprint,Manufacturer,MPN,Description,QUANTITY,DNP",
        "--labels", "Reference,Value,Footprint,Manufacturer,MPN,Description,Quantity,DNP",
        "--sort-field", "Reference", str(SCHEMATIC),
    )
    run(sys.executable, str(ROOT / "tools" / "check_if_b01_kicad.py"))
    run(
        "kicad-cli", "pcb", "drc", "--schematic-parity", "--all-track-errors", "--severity-all",
        "--exit-code-violations", "--output", str(drc_report), str(PCB),
    )

    run(
        "kicad-cli", "pcb", "export", "gerbers", "--output", str(staging_dir / "gerbers"),
        "--layers", "F.Cu,In1.Cu,In2.Cu,B.Cu,F.Paste,B.Paste,F.Mask,B.Mask,F.SilkS,B.SilkS,Edge.Cuts",
        "--include-border-title", "--precision", "6", str(PCB),
    )
    require_files(
        staging_dir / "gerbers",
        (
            "if-b01-F_Cu.gtl", "if-b01-Inner layer 1.g1", "if-b01-Inner layer 2.g2", "if-b01-B_Cu.gbl",
            "if-b01-f_Paste.gtp", "if-b01-b_Paste.gbp", "if-b01-f_Mask.gts", "if-b01-b_Mask.gbs",
            "if-b01-f_Silkscreen.gto", "if-b01-b_Silkscreen.gbo", "if-b01-Edge_Cuts.gm1",
        ),
        "requested Gerber plots",
    )
    run(
        "kicad-cli", "pcb", "export", "drill", "--output", str(staging_dir / "drill"),
        "--excellon-separate-th", "--excellon-units", "mm", "--generate-map", "--map-format", "pdf",
        "--generate-report", "--report-path", str(staging_dir / "drill" / "if-b01-drill-report.txt"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "pos", "--side", "both", "--format", "csv", "--units", "mm",
        "--output", str(staging_dir / "assembly" / "if-b01-placement-all.csv"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "pos", "--side", "front", "--format", "csv", "--units", "mm",
        "--smd-only", "--output", str(staging_dir / "assembly" / "if-b01-placement-smd-top.csv"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "pdf", "--mode-single",
        "--output", str(staging_dir / "assembly" / "if-b01-top-assembly.pdf"),
        "--layers", "F.Cu,F.Mask,F.SilkS,Edge.Cuts", "--exclude-value", "--include-border-title",
        "--sketch-pads-on-fab-layers", "--scale", "0", str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "pdf", "--mode-single", "--mirror",
        "--output", str(staging_dir / "assembly" / "if-b01-bottom-assembly.pdf"),
        "--layers", "B.Cu,B.Mask,B.SilkS,Edge.Cuts", "--exclude-value", "--include-border-title",
        "--sketch-pads-on-fab-layers", "--scale", "0", str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "svg", "--mode-single",
        "--output", str(staging_dir / "assembly" / "if-b01-top-assembly.svg"),
        "--layers", "F.Cu,F.SilkS,Edge.Cuts", "--fit-page-to-board", "--exclude-drawing-sheet", str(PCB),
    )
    run(
        "kicad-cli", "pcb", "render", "--width", "2400", "--height", "1280", "--side", "top",
        "--background", "opaque", "--output", str(staging_dir / "assembly" / "if-b01-top-render.png"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "step", "--force", "--board-only", "--include-tracks", "--include-pads",
        "--include-inner-copper", "--include-silkscreen", "--include-soldermask",
        "--output", str(staging_dir / "mechanical" / "if-b01-board-only.step"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "ipcd356",
        "--output", str(staging_dir / "fabrication" / "if-b01.ipc-d-356"), str(PCB),
    )
    run(
        "kicad-cli", "pcb", "export", "stats", "--format", "json", "--units", "mm",
        "--subtract-holes-from-board", "--subtract-holes-from-copper",
        "--output", str(staging_dir / "fabrication" / "if-b01-board-statistics.json"), str(PCB),
    )
    run(
        "kicad-cli", "sch", "export", "bom", "--output", str(staging_dir / "bom" / "if-b01-fabrication-bom.csv"),
        "--fields", "Reference,Value,Footprint,Manufacturer,MPN,Description,QUANTITY,DNP",
        "--labels", "Reference,Value,Footprint,Manufacturer,MPN,Description,Quantity,DNP",
        "--sort-field", "Reference", str(SCHEMATIC),
    )
    shutil.copy2(erc_report, staging_dir / "fabrication" / "if-b01-erc.rpt")
    shutil.copy2(drc_report, staging_dir / "fabrication" / "if-b01-pcb-drc.rpt")
    shutil.copy2(netlist, staging_dir / "fabrication" / "if-b01-netlist.net")
    write_readme(staging_dir)
    write_manifest(staging_dir)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="write the verified draft output package")
    args = parser.parse_args()
    if not args.write:
        parser.error("refusing to write outputs without --write")
    staging_dir = OUTPUT_DIR.with_name(OUTPUT_DIR.name + ".staging")
    if staging_dir.exists():
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True)
    try:
        export(staging_dir)
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        shutil.rmtree(staging_dir, ignore_errors=True)
        print(f"IF-B01 manufacturing export failed: {error}", file=sys.stderr)
        return 1
    if OUTPUT_DIR.exists():
        shutil.rmtree(OUTPUT_DIR)
    staging_dir.replace(OUTPUT_DIR)
    print(f"Wrote verified IF-B01 draft output package: {OUTPUT_DIR.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())