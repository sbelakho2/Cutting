# Physical qualification procedure — S1-B

Revision `S1-B-QUAL-R1-2026-09-13`. Tool: `starz_qualification` ([source](../engineering/src/qualification.cpp), limits in [qualification.hpp](../engineering/include/qualification.hpp)).

**Status: no physical qualification has been performed. No machine has been assembled. Every record template in the repository is blank and the tool rejects it.** The synthetic fixtures under `engineering/tests/fixtures/synthetic-*` exercise the arithmetic only; their `SYN-` identities are not machine results.

## What must be demonstrated

| Item | Acceptance | Sampling |
|---|---|---|
| Cut length | error ≤ ±(0.05 % + 0.20 mm) of commanded length, i.e. ±0.30 mm at 200 mm and ±0.70 mm at 1000 mm, after subtracting the gauge U(k=2) | ≥30 consecutive pieces per cell; cells at wire OD 1.2 and 4.5 mm × commanded 200 and 1000 mm are mandatory; Cpk ≥ 1.33 per cell |
| Nip preload, each side | 50–75 N per nip, guard-banded by gauge U | one reading per side at OD 1.2 and 4.5 mm |
| Measuring-wheel contact force | 6–19 N, guard-banded by gauge U | one reading at OD 1.2 and 4.5 mm |
| Environment | 15–30 °C recorded for every length sample | — |
| Gauges | length U(k=2) ≤ 0.05 mm; force U(k=2) ≤ 1.0 N; identities recorded | — |

The ±0.70 mm/1 m figure is the S1 design target from [docs/02](02-benchmarks-and-sources.md). The tool reports PASS only when every mandatory cell and force record is present, in range, and capable. It exits 1 on blank or malformed records, 2 on I/O errors, 3 on a complete but failing data set, and writes `status.json` with `qualification_passed` accordingly. `physical_results` is `true` in `status.json` only for a run whose sample and gauge identities carry no `SYN-` prefix; the report lists every gauge id and input file so the data source is auditable. The repository never contains such a run.

## Electrical and safety prerequisites — separate records required

This dimensional/process procedure is **not** an authorisation to energise the
machine and does not qualify the electrical installation or safety function. Do
not begin it until the applicable release holds in [controls and wiring](22-controls-and-wiring.md)
have been closed with reviewed evidence. At minimum, retain separate records for:

1. IF-B01 native schematic/PCB review, ERC/DRC, manufactured-board inspection
	and the bare-board fixture result using only `if-b01-factory-test.uf2`;
2. cabinet de-energised inspection: point-to-point continuity, separation,
	protective-earth continuity, insulation, terminal torque/marking and actual
	protection-device configuration;
3. XPS/E-stop/guard channel and reset/EDM tests, including relevant cross-wire
	and contactor-fault cases, performed to an approved test plan;
4. measured K1/K2/K3 opening response, driver DC-bus decay and regenerated-energy
	behavior before any guard-release permission is designed or enabled; and
5. completed guard, wire-handling, containment and emergency-access review.

The resulting length, preload and wheel-force data qualify only the stated
machine, configuration, material/recipe and test conditions. They never
substitute for the separate hardware safety validation above.

## Procedure

1. Generate templates: `starz_qualification --template <dir>` writes `length-records-template.csv` (120 blank rows: 30 per mandatory cell) and `force-records-template.csv` (6 rows).
2. Set the machine for the wire under test: fit the F206 insert for that OD, set the carrier adjuster AJ01 to the protrusion printed in `engineering/generated/mechanism/status.json` (`carrier_adjuster_protrusion_mm`), set the arm adjuster AJ02 to `arm_adjuster_protrusion_mm`, and set the counter-roller fork M210 so the 626 roller just touches the wire.
3. Measure nip preload by lifting each upper cartridge pair with a calibrated force gauge at the roller axis until the wire is released; record `force_n` and the gauge U. Measure the wheel force the same way under the arm hub. Enter the six force rows.
4. Cut 30 pieces at 200 mm and 30 at 1000 mm for each mandatory OD. Measure each piece on a calibrated scale or CMM at recorded temperature; enter every row. Do not discard pieces.
5. Run `starz_qualification --lengths <csv> --forces <csv> --out <dir>` and archive the output directory with the raw CSVs. A PASS is a property of that machine, that recipe and that day; it does not transfer to other builds.

## Outputs

`length-cells.csv` (n, mean error, SD, tolerance, Cpk, pass per cell), `force-checks.csv`, `report.md`, `status.json`.
