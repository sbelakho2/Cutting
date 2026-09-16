# Total CAD/spec non-contradiction audit — 2026-09-16

Scope: total consistency audit after the 2026-09-16 substitutions
(ifm RU3100 encoder, ifm E60137 200 mm measuring wheel, DIN 471 rings,
5×20 fuses). New additive checker
[`tools/check_total_consistency.py`](../../tools/check_total_consistency.py)
registered as a core CTest (`check_total_consistency`) alongside
`check_design_integrity`. Every violation below was fixed in the owning file
and re-verified by the full build/test/validator run.

## 1. Violations found and fixed

| # | Violation | Fix |
|---|---|---|
| 1 | `software/hmi/tests/test_reference_hmi.cpp:42` sent the 100 mm-era `24414` nm/count calibration value | → `48828` (the current `machine_spec.hpp` nominal) |
| 2 | `bom/measurement-custom-parts.csv` M201 still described the R1 plate: `encoder_face_mount_plate46x46x6`, stock `50x50x8` | → `60x60x6`, stock `66x66x8` (matches docs/13 R2 and the M201-R2 drawing) |
| 3 | `bom/mechanism-custom-parts.csv` M202/M203/M204/M206/M207/M209/M210 quantity 1 — the design has two measuring modules (docs/13: "one upstream and one downstream"; the CAD models one representative module, the mirror is not modeled) | quantity → 2 each |
| 4 | `bom/selected-mechanism.csv` MC-P02 (SWU10.5-35 arm spring) quantity 1 for the two-module design (same mirror basis) | quantity → 2; `check_design_integrity.py` module scale for SWU10.5-35 → 2 |
| 5 | `shared/measurement_spec.hpp` presented the encoder shaft interval 5.99–6.01 mm without stating that **ifm publishes no shaft-diameter tolerance** (archived datasheet says only "Shaft diameter [mm] 6") | added the placeholder note: interval is assumed, confirm at incoming inspection (same treatment as the retained wheel-profile stations). The Autonics −0.004/−0.016 interval does not appear anywhere in active sources |
| 6 | `README.md:20` linked `M201-R1-encoder-plate.pdf` (deleted in R2) and called the mount "eight-part" (it is five) — `tools/check_links.zsh` was failing | → `M201-R2-encoder-plate.pdf`, "five-part"; links checker green |
| 7 | J8/J9 receiver review open item claimed in docs/22 §6, `hardware/if-b01/README.md`, `if-b01-cabinet-integration.md`, `shared/controls_spec.hpp`, docs/13 §5, docs/24 and docs/33 | resolved everywhere — verdict in §2 below |
| 8 | `shared/feed_drive_spec.hpp` A12 ring constants unnamed vs the DIN 471 substitution | comment links Wegertseder Art. 1950-010 DIN 471 12×1 (TR-RR01); geometry unchanged (per coupling-ring-replacement-findings.md, no CAD change) |
| 9 | DIN 471 ring coverage gap: no checker mapped the eight "SEEGER A12" purchased CAD envelopes to the TR-RR01 BOM row | the new checker verifies 8 CAD envelopes ↔ TR-RR01 ×8 DIN 471 identity |

## 2. J8/J9 line-receiver verdict (RU3100 → AM26LV32EIDR)

Facts: archived ifm RU3100-01 datasheet — **single-ended push-pull A/B/Z
only** (M12 pins 1 UB / 2 A / 3 GND / 4 Z=IO-Link / 5 B), factory output
setting **HTL (50 mA)**, TTL is an IO-Link parameter setting, 1024 PPR
factory. TI AM26LV32**E** datasheet — recommended input common-mode range
**−7…+7 V**, ±200 mV differential thresholds, open-input failsafe (output
high). Board: J8/J9 = +5V/0V/A+/A−/B+/B− with four 120 Ω differential
terminators R1–R4 and receiver outputs to GP8–GP11.

- **Required encoder configuration: TTL mode, set via IO-Link
  parameterisation** before wiring. The factory HTL setting must not be
  wired to the board. Keep the factory 1024 PPR (no commissioning).
- **Wiring:** A → A+ terminal, B → B+ terminal; the receiver − inputs
  (A−/B− terminals) are **not encoder wires** (the RU3100 has no Ā/B̄) and
  must be tied to a **2.5 V mid-bias** from the encoder +5 V branch.
  Grounding them would read a 0.4 V TTL low as high; floating them trips the
  failsafe.
- **Common mode:** TTL swing (≤5 V high / ≥0.4 V low) against 2.5 V bias
  gives ±2.1…2.5 V differential (≫ ±200 mV threshold) and 1.2…3.75 V common
  mode — inside the AM26LV32E −7…+7 V range. **PASS.**
- **Termination:** the four **120 Ω terminators are not fitted** for TTL
  drive (≈21 mA drawn from the TTL output per active channel against the
  bias, and unnecessary on a 2 m point-to-point link). Required IF-B01
  board change: replace R1–R4 with the mid-bias network (e.g. 2×10 kΩ per
  − input from +5V_ENC to 0V_PELV) — recorded as a draft-board change in
  docs/22 §6/§7 and the IF-B01 README/cabinet-integration notes, to be
  applied before the IF-B01 review hold closes.
- Z (M12 pin 4, doubles as the IO-Link line) stays insulated/unused; the
  M12 screen bonds at the cabinet PE entry only (unchanged).

Docs updated: docs/22 §6 (J8/J9 row) + §7, `hardware/if-b01/README.md`,
`hardware/if-b01/if-b01-cabinet-integration.md`, `shared/controls_spec.hpp`
encoder comment, docs/13 §5 item 9, docs/24 ME-E01 note, docs/33 gate 34.

## 3. Checks implemented (`tools/check_total_consistency.py`)

1. **Cross-spec constants:** machine_spec 200 mm / 4096 / 48828 nm-count;
   controls_spec 20480 counts/m / 48.828 µm; measurement_spec 200 / 63.66 /
   1024 PPR; mechanism lug rise 15.915 mm (219.915/231.915); docs/01 and
   docs/13 remapped values present; repo-wide sweep flags any surviving
   100 mm-era constants (`40960`, `24414`, `0.024414`, old wheel-diameter
   claims) outside explicitly historical records.
2. **CAD-vs-datasheet:** archived RU3100/E60137 texts vs spec constants
   (58.6×63.5, Ø6 shaft, PCD42/M4×0.7×6, 1000 kHz, 12000 rpm, 0.01 Nm,
   60/40 N; 200/63.66/6/90 Shore A/64.5 g); shaft-tolerance placeholder
   must be labelled; measurement manifest bounds (±31.83 wheel, −63.5…+10
   encoder, 60×60×6 plate); measurement + mechanism status.json (revision,
   5-part mount, wheel force 10 N, 504 occurrences, wire-route and
   intersection flags, M206 lug top 231.915, M210 fork present).
3. **J8/J9 review closure:** netlist 6-position allocation; the resolved
   TTL-mode/mid-bias/120 Ω verdict present in docs/22, README,
   cabinet-integration, controls_spec and docs/13; the "open item" claims
   gone.
4. **Fastener sweep (≥1.5D):** H003 F0 M5×16 = 7.5 mm; motor M5×25 = 10;
   M201 M4×12 flush 6.0 mm into the 6 mm flange taps (no bottoming); M4×6
   flush; feed M3×8 = 6.0 mm through the 2 mm F103 cover; bed M10×35 =
   10 mm into the DIN 929 M10 weld nut (full nut height ≈9 mm — the 1.5D
   rule applies to tapped-through plates, not the nut); bracket M8×25 =
   21 mm; roller column M6×20 = 16 mm; bushing M4×10 = 8.1 mm; base-to-bed
   M8×55 = 20.25 mm.
5. **BOM-vs-manifest-vs-spec:** wheel ×2, encoder ×2, 626 ×10 (5/module),
   M4×12 ×6, coupling ×2, SWU10.5-35 ×2, SWU17-60 ×2, PSFJ12-240 ×4,
   LHFRW12 ×4; DIN 471 ring coverage (8 envelopes ↔ TR-RR01 ×8); M2xx
   custom parts ×2; M201 R2 identity/stock; purchased mechanism-BOM rows
   covered by a selected identity or the docs/24 standard-hardware register.
6. **Drawing-vs-spec:** M201-R2 sheet and inspection template carry
   60.00±0.05 / Ø36.50+0.05/0 / PCD42 / M4×0.7 / R2; SVG + PDF present.

## 4. Files changed

- `tools/check_total_consistency.py` (new), `tools/check_design_integrity.py`
  (SWU10.5-35 ×2 module scale), `CMakeLists.txt` (registered the new test).
- `shared/measurement_spec.hpp` (shaft-tolerance placeholder note),
  `shared/feed_drive_spec.hpp` (DIN 471 identity comment),
  `shared/controls_spec.hpp` (J8/J9 verdict comment).
- `software/hmi/tests/test_reference_hmi.cpp` (48828).
- `bom/selected-mechanism.csv` (MC-P02 ×2),
  `bom/mechanism-custom-parts.csv` (M2xx ×2),
  `bom/measurement-custom-parts.csv` (M201 R2).
- `docs/22-controls-and-wiring.md`, `docs/13-measuring-interfaces-and-calibration.md`,
  `docs/24-production-and-release-plan.md`, `docs/33-gate-ledger.md`,
  `README.md`, `hardware/if-b01/README.md`,
  `hardware/if-b01/if-b01-cabinet-integration.md`.
- This file.

No CAD geometry source, BOM identity of a selected purchased part, or
generated artifact required regeneration: the R2 CAD/manifest/status
artifacts already match the spec (verified by the new checker and by the
mechanism/measurement export readback tests), so the generated outputs were
left byte-stable; the drawing export test regenerated the drawing set in the
build tree without drift.

## 5. Verification

Build: `/var/folders/z1/b8gks6rd3vg16f9nrld50bj80000gn/T/kilo/build-consistency`,
Ninja + Homebrew LLVM clang, Release, `STARZ_BUILD_ENGINEERING=ON` — clean
(only the pre-existing fem.hpp `weight` aggregate warnings from the adapter
contact session).

- Full CTest: **91/91 pass** (including `check_total_consistency`,
  `check_design_integrity`, measurement interface/export/rejection tests,
  mechanism full-machine min/max wire-route tests, drawing export, FEM and
  CLI suites).
- Repo validators all green: `check_production_records.py`,
  `check_mechanism_outputs.py`, `check_if_b01_gpio_mapping.py`,
  `check_if_b01_kicad.py`, `check_links.zsh` (README M201-R1 link fixed),
  `check_pio_contract.zsh`.

Not committed. Not fabricated/energised — records, not release evidence.
