# F101 R6 distributed-support FEM verification

**Historical44-test source-state record.** The later [focused-refinement verification](local-shaft-refinement.md) documents the current46-test implementation. Original four-mesh/96-phase artifacts and the fingerprints below are preserved; later source changes do not retroactively belong to these runs.

2026-09-12. Scope: nominal F101 R6 shaft elasticity with motor-driven coupling holding reaction, and the new C++ distributed-constraint solver. Follower/passive cartridge torque reactions are not qualified by imposing a nonexistent coupling at their unused shaft ends. Mechanical authority remains **head R6 / feed R6 / measurement R1**. No shaft, assembly, drawing, procurement or hardware-control definition changed in this increment.

**The four-mesh calculation completed. Regional stress qualification has not passed.** The preserved [status](../../engineering/generated/feed-fem/status.json) reports16 unmet checks and `fabrication_released:false`. Later focused results do not overwrite this baseline.

## Automated verification status

| Run | Status |
|---|---|
| Earlier scalar-METIS full native suite | 44/44 passed, 908.05 s; superseded factorization backend |
| Earlier scalar-METIS full ASan/UBSan suite | 43/44 passed; fine circular-bar test timed out at1800.35 s; not a complete pass |
| CHOLMOD native distributed/shaft regression group | 5/5 passed, 57.96 s; circular-bar test50.66 s |
| CHOLMOD instrumented regression group | 5/5 passed, 81.84 s; circular-bar test64.67 s |
| CHOLMOD full native suite | 44/44 passed, 623.16 s; circular-bar test56.77 s |
| CHOLMOD full ASan/UBSan suite | 44/44 passed, 967.36 s; circular-bar test90.28 s |

The previous [39-test R6 record](feed-r6.md) predates this solver and remains historical evidence for its own source state. It is not used to validate the new code.

Full suites use two CTest workers. The native build is Release; the sanitizer build is RelWithDebInfo with `STARZ_SANITIZE=ON` and address/undefined-behavior instrumentation. The replacement backend is **SuiteSparse7.14.0 / CHOLMOD5.3.5 / METIS5.1.0**. Project code and the Eigen adapter are instrumented; the prebuilt numerical dependencies themselves are not rebuilt with sanitizers. The instrumented run used `UBSAN_OPTIONS=halt_on_error=1` and `ASAN_OPTIONS=halt_on_error=1`. Both completed logs independently contain44 passed test records and no sanitizer diagnostics. Numerical tolerances, five analytical mesh levels and the150000-node cap are unchanged. No C++/CMake source changed during the final full-suite runs; both subsequent build dry runs reported no pending work. The earlier timeout is resolved by the verified backend change, not by increasing its1800-second timeout or weakening its checks.

### Independent analytical benchmark

Ø10 × 40 mm circular bar, $E=205$ GPa, $\nu=0.29$, with axial force and pure torque. No guessed concentration factor is used.

| Target mesh, mm | Nodes | Torsional compliance relative error | Interior full-tensor relative RMS error |
|---:|---:|---:|---:|
| 1.80 | 1269 | 0.125576 | 0.461209 |
| 0.90 | 4393 | 0.0424352 | 0.327319 |
| 0.60 | 13172 | 0.0194181 | 0.229020 |
| 0.40 | 40872 | 0.00902713 | 0.158818 |
| 0.28 | 113107 | 0.00446675 | 0.112913 |

The unchanged finest compliance criterion is 4%; the interior stress-tensor criterion is 12%. The RMS comparison is **unweighted over interior element centroids**, not a bound on every individual element. Compliance improved before the stress criterion passed; the coarser meshes were not accepted as stress validation.

The imposed-strain test uses a rectangular bar, seven and twelve distributed constraints, three stabilization scales, exact affine displacement/Poisson contraction, full stress tensors and independent end reactions. Dependent constraints are rejected. Shaft tests cover phase rotation, torque-polygon support maxima, an independent dense-angle check of the angular bound, invalid input including extreme signed integers, analytical bearing statics and rejection of a single mesh as converged.

## Four-mesh nominal shaft study

Eight linear load bases; eight radial-sign corners; six torque-polygon vertices; 48 phase samples plus an analytic unsampled-phase bound. Working/fault transmission envelopes are 1/5 Nm. Actual mesh gravity is retained. [Model equations and scope](../15-feed-shaft-stress-model.md).

The published run uses `--meshes 2.0:0.30,1.6:0.20,1.2:0.13,1.0:0.09` and `--phases 48`, with the public shaft-study directory as `--report`. It is not the driver's shorter three-level default.

| Bulk/root target, mm | Nodes | Tetrahedra | Working stress bound, MPa | Fault stress bound, MPa | Working displacement bound, mm |
|---|---:|---:|---:|---:|---:|
| 2.0 / 0.30 | 21549 | 88683 | 54.4627 | 122.317 | 0.0196447 |
| 1.6 / 0.20 | 37989 | 162559 | 60.0514 | 132.248 | 0.0199753 |
| 1.2 / 0.13 | 73958 | 330524 | 77.5577 | 163.251 | 0.0203447 |
| 1.0 / 0.09 | 131909 | 603290 | 76.5876 | 158.193 | 0.0205005 |

The final working/fault **sampled raw maxima** are 73.0733454/154.678486 MPa. The larger tabulated values bound unsampled phase for each discrete mesh; they do not bound mesh error or omitted physical effects. The sampled maxima occur near the rear shoulder/washer-slot intersection at X2.05282/S31.46204/Z5.71479 mm. In the regenerated CHOLMOD record, the working maximum uses phase52.5°, sign index5 and torque vertex3; the fault maximum uses phase232.5°, sign index5 and torque vertex0. The earlier scalar solver selected the exactly sign-reversed working load pair: shifting phase by180° and reversing both transmission torques negates every linear load coefficient, leaving stress/displacement magnitudes equal. Roundoff can select either representative; the CSV and exported field identify the actual one used.

Finest B-rep/FE volume difference: 0.192090%. Last displacement change: 0.759822%, below 3%. Whole-shaft working/fault raw-peak changes: 1.324712%/3.268609%, below 5%; corresponding angular-bound changes: 1.266555%/3.197269%.

**46 numerical checks: 30 pass their stated scope, 16 remain unmet.** These 16 are the separate sampled-peak and angular-bound refinement checks for:

- Working: A12 groove, M15 relief, neck shoulder, roller shoulder, pulley-key/MB2 slot and roller key root.
- Fault: A12 groove and roller key root.

All regional checks are retained even though the whole-shaft peak has passed its last-refinement criterion. A fifth globally refined mesh must not simply bypass the 150000-node budget. Further local resolution, thread/edge geometry and boundary-condition qualification remain work, not passed results.

### Actual-shaft phase-density cross-check

A separate [96-phase audit](../../engineering/generated/feed-fem-phase-audit/report.md) reran the finest1.0/0.09 mm mesh. The exported mesh coordinates/connectivity matched the published mesh exactly at export precision; both geometry prefixes,through the closing Cells tag plus one appended newline,have SHA-256 `519732f867026f77d0787def6657ff65a5c74627e5785a01b0a114dab9aa2e80`.

All **20 working/fault regional raw peaks** from96 phases stayed below the corresponding published48-phase bounds: **0 failures**, minimum reserve0.0253970578 MPa. The96-phase upper stress bounds are74.86140125/156.4406381 MPa and the displacement bound0.01980370146 mm. Both additional VTUs passed XML validation.

This verifies denser angular sampling on the same discrete geometry; it does not clear any of the16 spatial-refinement holds. The audit has only one mesh and correctly retains its unmet three-mesh gate and `fabrication_released:false`. The four-level,48-phase study remains the spatial-convergence record.

## Equilibrium and gravity

Finest maximum relative residuals across the eight bases:

| Check | Observed maximum | Limit |
|---|---:|---:|
| Discrete equilibrium | 1.09952910×10⁻¹⁰ | 10⁻⁷ |
| Distributed constraints | 9.24055460×10⁻¹⁶ | 10⁻⁷ |
| Force balance | 2.17657932×10⁻¹⁵ | 10⁻⁷ |
| Moment balance | 1.74105745×10⁻¹⁶ | 10⁻⁷ |

The rotating eccentric-gravity drive-holding amplitude is **0.000179096828 Nm**. It is additional to the transmission torque polygon, not silently discarded as a solver artifact. Balanced transmission bases have negligible spin-reference reaction; all body-load reactions must balance their actual Y moment. The ideal supports still do not represent bearing fit, contact or housing stiffness.

An independent [custom STEP readback](../../engineering/generated/feed-fem/step-readback/vendor-geometry.csv) found one valid solid,19×113×19 mm envelope,14390.4167475 mm³ volume and centroid S55.1007414/Z−0.162485352 mm (X numerically zero). At the specified density this gives0.112964771468 kg and0.000180002244 Nm gravity-holding amplitude. The FE amplitude is0.503003% lower. These are independent nominal geometry calculations,not weighing or physical balance measurements. The reused inspector's generic output filename does not make this custom F101 STEP original supplier CAD.

## Export checks and provenance

- Public [regional CSV](../../engineering/generated/feed-fem/regions.csv):80 data rows,14 columns, no malformed rows.
- Both public studies were regenerated with the final CHOLMOD-backed executable. All11 published source/result fingerprints were independently recomputed;149 local documentation links were checked with no missing targets.
- Manual I/O rejection check: an isolated test directory deliberately blocked the STEP filename. The process returned1 and retained `run_complete:false` and `fabrication_released:false`. This expected rejection is separate from the44 registered CTests.
- All ten regenerated governing-load VTU fields (eight main-study,two phase-audit) passed XML syntax validation with large-document mode. Every field also passed independent checks of all six arrays: finite numeric values, lengths, integer node indices, tetrahedral type10, four-node offsets and raw maximum matching its CSV row within1e-7 MPa. The finest working field has131909 nodes,603290 tetrahedra,395727 coordinate values,395727 displacement values,2413160 connectivity indices and603290 stress values.
- Independent scans of the finest working/fault fields reproduced73.0733454469/154.678485603 MPa raw maxima from the regional CSV. The working stress-governing case has0.0178695229424 mm maximum displacement,below the0.02050048391 mm multi-case angular envelope.
- VTUs represent one actual sampled governing stress load case each, not a field made from incompatible envelopes. No rendered stress-map review is claimed by the XML/data checks.
- Initial all-bases-zero-spin rejection was corrected to retain real eccentric gravity. Dense support augmentation and default sparse ordering were superseded by exact gauge cancellation and METIS nested dissection. The obsolete long-running default-order benchmark was stopped; it is not counted as a pass. The scalar-METIS implementation passed natively but timed out under instrumentation; CHOLMOD supernodal Cholesky replaces that factorization without changing the equations or acceptance criteria.
- The superseded Eigen METIS adapter required an explicit preceding standard iostream include; CHOLMOD now owns ordering and no longer uses that adapter. Large ASCII XML arrays exceeded the parser's default text-node limit and were checked successfully in explicit large-document mode; field data were not truncated to satisfy the parser.

SHA-256 fingerprints of the historical44-test source/result state used for both regenerated public studies. Source links open the present files,which may have newer hashes; the later record lists the current fingerprints:

| File | SHA-256 |
|---|---|
| [Engineering build configuration](../../engineering/CMakeLists.txt) | 0afb07d5f40953f36a0ee201795f1b7aa9197f7a607e83235269343c4de35987 |
| [Elastic API](../../engineering/include/fem.hpp) | 31ead0e40c47da07f2e2b5c5d16f4367bcff71ff2d7191e345053a695ddff3a4 |
| [Elastic solver](../../engineering/src/fem.cpp) | 4790baa47506bb314f0b8a8266e3b2c5ce7eb3f5138c8b634e8d416bd0f7595d |
| [Shaft model API](../../engineering/include/feed_fem.hpp) | ead419ac8622f0a50052ef5aad4cc66590a959963ad7fa7c8d249f8b2753efdb |
| [Shaft model](../../engineering/src/feed_fem.cpp) | 212153c8d0bdb37e8c3fb07e950a46864791aed897482cbd95b78d906fb36b38 |
| [Study driver](../../engineering/src/feed_fem_analysis.cpp) | 27448c3fa459a9af2ad33c277eeccc2b4f210162a5ee73cc499b123ae9600413 |
| [Shaft geometry](../../engineering/src/feed_geometry.cpp) | 618cd29f8901cca97f5fccc4f470a324369d0059806fa532f70b372bd9e534e3 |
| [Feed specification](../../shared/feed_drive_spec.hpp) | 3c1c725b93413f55609dbe4e725ec562a23a30d720dc5d5d402f6361e44bd236 |
| [Study status](../../engineering/generated/feed-fem/status.json) | 345657ab75d66c2a1e4faa403512911de0e647065b881099cb3eabbff892f832 |
| [Regional results](../../engineering/generated/feed-fem/regions.csv) | 0dce5b1ce2f1159afcb6fb103b55a891028f18d34d21ed3105ad7fbfaf30afe2 |
| [Phase-audit status](../../engineering/generated/feed-fem-phase-audit/status.json) | 01d08a098d05349ebb0cb30d12f1ed9030577ebb93f9eea86bb75f63d58a9868 |

## Release holds unchanged

Actual helical threads, final edge breaks, tolerances, bearing-fit prestress, KM2/M5 preload, key/collar contact, bearing/housing/module compliance and fatigue remain unqualified. The ±60 N roller-force envelope still needs physical pressure/friction/jam qualification:5 Nm at20 mm nominal radius can correspond to250 N tangential force,so the torque limit alone does not prove the force limit. The study cannot establish finished-wire accuracy or physical service life. The wider machine's pressure/tensioning mechanisms, measuring mechanics, thermal/electrical/safety integration and complete fabrication package remain unfinished. The **legacy reference** retains disabled outputs and `NotCommissioned`; it is distinct from the unqualified RP2350 target.