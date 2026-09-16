# Measurement replacement findings — ifm wheel and encoder substitution, 2026-09-16

Scope: replace the two unpriced measurement parts (Kübler measuring wheel
8.0000.3123.0006 ×2 and Autonics encoder E40S6-1024-6-L-5 ×2, ledger gates
33/34, letters L3/L4) with priced, in-stock equivalents and integrate them
through CAD, spec, checks and BOM.

## 1. Encoder selection

**Selected: ifm RU3100, EUR 186.40 ex VAT (RRP 207.10, −10%), in stock,
dispatch next working day, automation24 item 102717.**

Observed live 2026-09-16 at
<https://www.automation24.com/incremental-encoder-ifm-electronic-ru3100>.
The earlier recorded 185.50 (RRP 206.10) read is superseded by the
2026-09-16 page (186.40 / RRP 207.10). The RUP500 display variant
(EUR 255.50 ex VAT, item 102715, same synchro flange) was not needed — a
display adds nothing here and costs EUR 69.10 more.

ifm product data (ifm.com product page read 2026-09-16 via a reader
service; the site blocks direct fetch):
- Incremental encoder, magnetic, **solid shaft Ø6 × 10 mm, no flat**
  (drawing: 53.5 body + 10 shaft = 63.5 total).
- Body **Ø58.6 × 63.5 mm**; fixing flange **synchro-flange**; 3 × **M4 × 0.7
  tapped, 6 mm deep, on PCD Ø42** (drawing front view labels only Ø42 and
  Ø58.6; the callout "M4 x 0.7 Depth 6 mm" is the only hole note).
- 4.75–30 VDC, **HTL/TTL** push-pull, factory setting **1024 PPR** (IO-Link
  programmable 1…10000), phase offset 90°, accuracy 0.1°.
- **1000 kHz** switching frequency, **12000 rpm**, starting torque ≤ **0.01 Nm**,
  radial/axial shaft load 60/40 N, IP65/66 (housing IP67), M12 5-pole
  connector radial or axial, 381 g, −40…85 °C.
- Conrad cross-check (item 3320357) confirms the same magnetic synchro-flange
  identity and Ø58 / L 63.5 mm class.

Archived sources: `ifm-ru3100-datasheet.pdf` (official ifm RU3100-01 DE
datasheet, 2023-04-13, downloaded from the Conrad-hosted copy) and
`ifm-ru3100-datasheet-en.txt` (ifm EN-GB datasheet text, read 2026-09-16).

**Flange geometry decision:** the M201 plate provides a **Ø36.5-class
synchro-flange seat** (ifm mounting practice) plus **3 × Ø4.5 on PCD 42**
for M4 screws into the flange taps. The archived drawing labels no pilot on
the encoder, so the encoder CAD envelope models none (shaft directly from
the face); the Ø36.5 seat bore is harmless clearance either way. **Confirm
the centering feature at incoming inspection.**

**Integration deltas vs the Autonics:**
- Shaft 10 mm (was 15+5 pilot) → coupling moved Y44 → **Y48** so the shaft
  still reaches the coupling's encoder-side clamp; plate stays at Y70, tip
  10 mm from the face (was 20).
- Body Ø58.6 > Ø40 → the old FB301 plate-screw heads (Y76–80 at r 25.46)
  would clash with the body: the four M4×16 support screws became **M4×6
  with heads in M203 rear-face counterbores** (Ø7.5 × 4 at ±18, ±18), and
  the three flange screws (M4×12, heads in three more Ø7.5 × 4 counterbores
  on PCD 42). The M3 washers are dropped; the mount is now 5 parts
  (ME-E01, M201, 3 × ME-S). Mechanism occurrence count 507 → **504**.
- M201 plate 46×46 → **60×60 × 6** (backs the Ø58.6 flange, PCD 42 holes
  5.9 mm from the edge); stock 66×66×8; sheet R1 → **R2**.
- Encoder rate limits in the spec: 300 kHz → **1000 kHz**, 5000 → **12000 rpm**,
  starting torque 0.00392 → **0.01 Nm**, shaft load limits → **60/40 N**.

## 2. Measuring wheel selection

**Selected: ifm E60137 "U:200 N:06", EUR 57.00 ex VAT (RRP 61.90, −8%), in
stock, dispatch next working day, automation24 item 102756, EAN
4021179060219.**

Observed live 2026-09-16 at
<https://www.automation24.com/measuring-wheel-ifm-electronic-e60137> and on
ifm.com (43 pcs available there; list price EUR 61.90). ifm E60137-01 EN-GB
datasheet (text archived as `ifm-e60137-datasheet-en.txt`): **200 mm
circumference, Ø63.66 mm, bore Ø6**, aluminium wheel with **PU surface,
90 Shore A**, 64.5 g, −30…80 °C, "grooved plastic" design, for exact linear
measurement in transversal cutting processes.

**100 mm search record (all negative, 2026-09-16):**
- automation24: the ifm E40100-class measuring-wheel family lists 200 mm
  circumference wheels only (E60137 etc.); no priced 100 mm encoder wheel.
- Conrad search "Messrad encoder 100mm": no priced 100 mm encoder measuring
  wheel with a 6 mm bore.
- Amazon.de "measuring wheel encoder 100mm": no priced 100 mm unit; the
  priced 200 mm class anchor is B0CSFSCCTT (EUR 79.49 / 2 pcs, generic
  marketplace product, no manufacturer accuracy statement).
- Conclusion: no priced 100 mm-circumference wheel could be verified, so the
  **200 mm substitution was evaluated and adopted** per the task decision
  tree. The ifm E60137 is preferred over the Amazon pair: manufacturer-published
  200 mm circumference for length measurement, documented PU/90 Shore A
  surface, Ø6 bore, EU-stocked and traceable, EUR 57.00 each.

**200 mm substitution evaluation (rigorous):**
- Length scale: 1024 PPR × 4 quadrature = 4096 counts per turn; counts per
  mm halve from 40.96 to **20.48** (0.048828 mm/count vs 0.024414). The
  nominal counts-per-metre scale (machine/controls spec) changed
  40960 → **20480**; `nominal_encoder_nm_count` 24414 → **48828**.
- Calibration model: `measurement_analysis.cpp` synthetic circumference
  100.07 → **200.14** (same +0.07% basis); factory-scale gate stays at
  1.0 mm per commanded metre using an assumed ±0.2 mm tolerance (0.1%
  proportional — ifm publishes no tolerance); the "nominal_count_spacing"
  check limit was re-based to 0.05 mm with the quantisation allocation
  0.05 mm still covering the half-count (0.0244 mm); the 32-interval phase
  map, eccentricity bound and fitter are circumference-independent and
  unchanged. If finer resolution is ever required, the RU3100 is IO-Link
  programmable (e.g. 2048 PPR restores 0.024414 mm/count) with no
  mechanical change.
- CAD: wheel envelope OD 31.83 → **63.66**; `wheelCentreZ` raises the
  wheel/housing/pivot by **15.915 mm**, so the M206 bracket lug/column were
  raised by the same amount (`lug_z_min/max` 204/216 → **219.915/231.915**)
  to keep the SWU10.5-35 arm spring inside its deflection window and the
  wheel contact force at 10 N (6–19 N window). The fork M210, roller column
  M207 and counter roller were re-checked against the Ø63.66 envelope and
  need no change (wheel bottom clears the fork top by 6.25–8.2 mm across
  the wire-OD range and the roller column by 14.25 mm).
- Wheel profile stations: ifm publishes only OD/bore/circumference/surface.
  The 9.5 tread / 14.5 overall / Ø16.5 hub / Ø9 end-land stations and the
  four M4×6 radial clamp openings are **retained placeholders** from the
  previous envelope, documented as such in spec, CAD, docs/13 and the
  interface CSV. **Confirm the hub and clamping-screw geometry at incoming
  inspection** (runbook P0-4/P4-3; 2.0 Nm remains the placeholder M4
  cone-point desk value).

## 3. Calibration impact numbers (docs/13 R2)

| Quantity | Before (Kübler 100 mm) | After (ifm E60137 200 mm) |
|---|---|---|
| Nominal x4 resolution | 0.0244140625 mm/count | **0.048828125 mm/count** |
| Nominal counts per metre | 40960 | **20480** |
| At 500 mm/s (min circumference) | 300.30 rpm / 5125.1 A/B / 20500.5 edges | **150.15 rpm / 2562.6 A/B / 10250.3 edges** |
| Factory scale error at 1 m | 1.0 mm (published ±0.1) | **1.0 mm (assumed ±0.2 — ifm publishes none)** |
| Uncalibrated 1 m budget | 1.65 mm | **1.65 mm** (unchanged allocations) |
| Quantisation allocation | 0.05 mm | **0.05 mm** (covers half-count 0.0244) |
| 32-interval phase map / eccentricity bound | unchanged | **unchanged** (circumference-independent) |

## 4. CAD/spec changes

- `shared/measurement_spec.hpp` — revision S1-B-MEAS-R2-2026-09-16; wheel
  E60137 (circumference 200, OD 63.66, PU 90 Shore A, 64.5 g); encoder
  RU3100 (body 58.6×63.5, shaft 6×10 no flat, PCD 42 / M4×0.7×6, 1000 kHz,
  12000 rpm, 0.01 Nm, 60/40 N); plate 60×60, seat Ø36.5, clearance Ø4.5,
  support taps M4 (Ø4.0 major per convention).
- `shared/mechanism_spec.hpp` — revision S1-B-MECH-R4-2026-09-16;
  `coupling_y` 44→48; `lug_z_min/max` 204/216→219.915/231.915 (+15.915).
- `shared/machine_spec.hpp` — wheel circumference 200 mm; nm/count 48828.
- `shared/controls_spec.hpp` — nominal counts per metre 20480; encoder pin
  comment now notes the TTL/HTL substitution (receiver review open).
- `engineering/src/measurement_geometry.cpp` — new encoder envelope
  (no pilot, 10 mm shaft, 3×M4 on PCD 42), plate R2 (60×60, Ø36.5 seat,
  3×Ø4.5 PCD42, 4×M4 taps), 5-part mount (M4×12, no washers), interface
  checks re-based (tip 10 mm, body 63.5, ±31.83 wheel).
- `engineering/src/mechanism_geometry.cpp` — housing rear counterbores
  (3 on PCD42 + 4 at ±18,±18, Ø7.5×4), FB301 → M4×6 housing-side support
  screws, removed the old Ø40 rear pocket and Ø4 clearance holes.
- `engineering/src/measurement_analysis.cpp` — 200.14 synthetic
  circumference, re-based checks and report narrative.
- `engineering/src/measurement_cad.cpp` — ifm interface identities,
  5-part mount status, short-tip regression now 8 vs 10 mm.
- `engineering/src/fabrication_drawings.cpp` — M201 sheet R2 (60×60, Ø36.5
  seat, PCD42, M4 taps, counterbore and fastener notes).
- `tools/check_design_integrity.py` — coverage tokens E60137/RU3100, wheel
  Ø 200/π check, M4×12 engagement check, M4×6 support flush check.
- `tools/check_mechanism_outputs.py` — MM_ 46→43, occurrences 507→504, R4.
- `engineering/tests/mechanism_export.cmake` — 504 occurrences, R4 revision.
- `engineering/generated/measurement/M201-inspection-template.csv` — R2
  characteristics (60×60, Ø36.5 seat, Ø4.5 holes, M4 taps).

## 5. BOM

`bom/selected-transport-and-cabinet.csv`:
- ME-W01 → ifm E60137, automation24 link, EUR 57.00 ex VAT (USD 66.07 at
  the ECB 2026-09-11 1.1592 repo conversion).
- ME-E01 → ifm RU3100, automation24 link, EUR 186.40 ex VAT (USD 216.08).
- ME-FB01 → ISO4762 M4×12 class 8.8 ×6 (6.0 mm nominal flange engagement);
  the M3 washer row ME-FW01 is removed.

## 6. Open items (not blocking the CAD/check integration)

- **Electrical:** the J8/J9 wiring, AM26LV32EIDR line receivers and board
  PTCs (docs/22, controls_spec.hpp, hardware/if-b01) were designed for the
  Autonics 5 V RS-422 line driver. The RU3100's TTL/HTL push-pull outputs
  need their own receiver/termination review before wiring — flagged in
  docs/13 §5 and the controls_spec comment; the controls doc set is not
  edited by this record.
- **ifm wheel hub/clamping-screw geometry and the 200 mm circumference
  claim:** unpublished; verify at incoming inspection (runbook P0-4/P4-3).
- **Synchro-flange centering feature:** drawing labels none; confirm the
  Ø36.5-class seat interpretation at receipt.
- M201 torque values remain open (as before).

## 7. Test status

All engineering, software, validator and regression gates re-run green
against the R2/R4 source (see the session build/ctest record):
measurement interface checks, 5-part mount check, short-tip and M3-wheel
rejections, mechanism 504-occurrence export with STEP readback, wire-route
gate, spring/force windows, measurement CLI suite, drawing export, design
integrity, mechanism generated-output integrity, IF-B01 GPIO mapping and
production-record integrity.
