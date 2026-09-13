# Feed R6 verification record

Completed 2026-09-12. Scope: **head R6 / feed R6 / measurement R1** engineering software, nominal CAD and generated fabrication records. This is not a fabrication, safety or production release.

This is the **pre-distributed-FEM baseline**. The later [F101 shaft stress verification](feed-shaft-fem.md) records the added solver/tests and four-mesh nominal study; it retains16 regional convergence holds. The39-test results below are not validation of that later source state.

## Automated verification

| Run | Result | Elapsed time |
|---|---:|---:|
| Full native CTest suite | 39/39 passed | 713.65 s |
| Full ASan/UBSan CTest suite | 39/39 passed | 1324.91 s |
| Feed drivetrain calculation | 25 checks, 0 failures | Included in suites |
| Nominal machined-section audit | 191 sections, 0 sampled stiffness failures | Included in suites |

Both complete builds succeeded. The final chained build/test/export/PDF verification command exited **0**. No C++ source changed during that run. Earlier construction failures were corrected before this final run; they are not outstanding failures.

New tests cover solid/annular/translated/rotated/keyed section-integration benchmarks, rejection of the unfinished shaft-edge geometry, and the actual machined-section audit. Existing continuous insertion, deliberately obstructed assembly, independent jaw motion, selected-part interfaces, STEP readback, numerical benchmarks and fail-closed reference-control tests remain enabled. Passing expected-rejection tests does not convert their deliberately unqualified inputs into approved designs.

## Public CAD artifacts

| Artifact | Occurrences | Revision and result |
|---|---:|---|
| [Driven cartridge](../../engineering/generated/feed/feed-cartridge.step) | 32 | Feed R6; valid export/readback; static interfaces and declared critical axial insertion paths checked |
| [Four-roller cassette](../../engineering/generated/feed-cassette/feed-cassette.step) | 116 | Feed R6; valid export/readback; OD4.5 mm placement and belt-backing envelope checked |
| [Head and eight rollers](../../engineering/generated/layout/head-and-feed-layout.step) | 287 | Head R6/feed R6; valid export/readback; cross-subsystem static checks and17.5 mm X-separation certificate |

The [cartridge status](../../engineering/generated/feed/status.json), [cassette status](../../engineering/generated/feed-cassette/status.json) and [layout status](../../engineering/generated/layout/status.json) identify the correct R6 revisions and report completed STEP readback. They continue to identify absent supports, pressure/tensioning mechanisms and full-machine integration. Cassette insertion and belt tooth-motion sweeps are not claimed.

## Drawings and inspection

- [F101 R6 shaft dimensions](../../engineering/generated/drawings/F101-R6-shaft.pdf): one A3 page,1191×842 pt. Its SVG review had37 text labels, all within the border, with no overlapping label boxes.
- [F101 R6 machining details](../../engineering/generated/drawings/F101-R6-details.pdf): one A3 page,1191×842 pt. Its SVG review had38 text labels, all within the border, with no overlapping label boxes. The drawing was also visually reviewed.
- H003 R6 and M201 R1 drawings were regenerated without geometry changes. Earlier F101 sheets remain historical.
- [Blank F101 inspection record](../inspection/F101-R6.csv):55 characteristics,10 columns; measured values, uncertainties, instrument identities, results, inspectors and dates remain blank.
- [Selected M5 tapping operation](../14-f101-tapping-operation.md): actual Dormer E412M5/7441118, physical-tip allowance distinguished from cutting lead, same-lot coupon and depth-gauge requirements. [Tooling BOM](../../bom/fabrication-tooling.csv):2×USD40.55=USD81.10 observed merchandise, not Tunisia-delivered cost. Source-interface CSV structure and price arithmetic checked.

## Numerical interpretation

The [section audit](../../engineering/generated/feed-sections/report.md) gives minimum sampled nominal stiffness ratio1.00. It integrates native B-rep section faces and compares the smaller principal bending inertia with each reduced beam section. It does not establish a continuous tolerance bound, local notch stress, torsional stiffness, fatigue life or physical accuracy. OpenCASCADE's reported integration estimate is relative **area** convergence, not a certified uncertainty for all inertia components.

The updated [beam calculation](../../engineering/generated/feed-analysis/report.md) gives0.0263693405854 mm maximum displacement under its declared load signs. Working/fault3× stress-sensitivity values are175.371316408/329.550980348 MPa. The3× multiplier is not a verified notch factor.

## Release state preserved

Local shaft/key/collar/contact stresses, joint preloads, actual thermal fields, pressure/tensioning/support mechanics and complete machine assembly remain open. The current analytical report still exposes cabinet-temperature and uncalibrated length-budget failures. R6 adapter raw peak-stress convergence is still unresolved; the historical full-head FEM is not relabelled as current assembled-machine proof. Original coupling interior clearance has not been verified from supplier CAD.

The **legacy reference** retains disabled outputs and `NotCommissioned`; it is distinct from the unqualified RP2350 target. No physical machining, calibration, endurance, safety validation, procurement or supplier contact is represented as completed by this record.