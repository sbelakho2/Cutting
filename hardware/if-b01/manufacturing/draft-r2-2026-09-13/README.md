# IF-B01 manufacturing output — draft only

**Source revision:** `S1-B-IF-B01-NATIVE-DRAFT-R2-2026-09-13`  
**Status:** source-generated DRAFT. It is **not** a fabrication, assembly, energisation, cabinet-integration, safety, or machine-release package.

This directory is generated only after the source generator, KiCad ERC, native BOM/netlist parity check, and physical plus schematic-parity PCB DRC pass. The exact files and SHA-256 values are in `manifest.json`.

## Contents

- `gerbers/` — four copper layers, paste, masks, silkscreens and outline plots.
- `drill/` — Excellon PTH/NPTH data, map and drill report.
- `assembly/` — CSV placement data and top/bottom visual assembly exports.
- `mechanical/` — board-only STEP representation; custom footprint 3D models are not present.
- `fabrication/` — IPC-D-356 electrical test netlist and board statistics.
- `bom/` — KiCad-exported fabrication BOM.

Before any quotation, ordering, fabrication or assembly, complete the human electrical/mechanical review, confirm the unusually large 750 x 400 mm draft panel dimensions and fabricator capability, validate every footprint and connector, and follow all release holds in the IF-B01 source review.
