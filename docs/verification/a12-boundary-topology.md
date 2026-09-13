# A12 boundary topology — verification record

2026-09-12. Numerical implementation and reported-artifact verification only. **Physical authority remains head R6 / feed R6 / measurement R1. No fabrication or energisation release.** The [study record](../18-a12-boundary-topology.md) contains the exact configurations, results and remaining geometry/stress holds.

## Automated verification

Both complete suites passed: **52/52 native tests in579.68 s** and **52/52 ASan/UBSan tests in873.81 s**, including all original mechanical, reference-control, calibration and elasticity regressions. Independent scans of the native log (`build-engineering/Testing/Temporary/LastTest.log`, not kept) and sanitized log (`build-engineering-sanitize/Testing/Temporary/LastTest.log`, not kept) each found52 passed records and no AddressSanitizer, UndefinedBehaviorSanitizer, runtime-error or LeakSanitizer diagnostics. These transient logs may be overwritten by subsequent CTest runs; the table and fingerprints identify this source state.

| Regression | Native PASS / elapsed | ASan/UBSan PASS / elapsed |
|---|---:|---:|
| Exact A12 exterior-cell fixture | 1.41 s | 1.79 s |
| Full-shaft baseline/Netgen topology regression | 110.30 s | 128.55 s |
| Optimized stress driver and original exterior-case gates | 62.82 s | 133.52 s |
| Analytic box/cylinder audit, both optimizers | 6.89 s | 8.77 s |
| Circular-bar distributed elasticity | 36.03 s | 53.97 s |

Sanitized execution used `ASAN_OPTIONS=halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`. The optional full542803-centroid audit was also run under those flags, with matching native counts and a byte-identical25-row exterior-centroid CSV. Source contents and all16 recorded fingerprints remained unchanged throughout verification.

A subsequent source modification-time change triggered two recompile/relink steps in each configuration despite unchanged source contents. All four rebuilt object files and both native executables were byte-identical. The two sanitizer executables had different file hashes, so their refreshed versions were checked directly with both halt-on-error flags:

- The analytic box/cylinder tests passed with both optimizers.
- The complete baseline/Netgen topology regression passed, reproducing136805/568908 and136210/542803 nodes/cells respectively.
- The post-link full-centroid audit (`build-engineering-sanitize/engineering/boundary-centroid-relinked-check/status.json`, not kept) checked all542803 cells and retained25 exterior/0 on-boundary centroids. Its exterior-cell list (`outside-cell-centroids.csv`, not kept) is byte-identical to the published native list: SHA-256 `2e8aa74c4fc96e7b213043e25a2ff9d2fd8426bd1fd4361a5dcd359b45f82853`.

The combined direct recheck exited0 without sanitizer diagnostics. The complete52-test CTest logs were preserved; both final Ninja dry runs reported no work. No executable-byte-equivalence claim is made for the two relinked sanitizer files.

Two CTest workers; numerical studies and full-centroid audits overlapped some tests, so elapsed times are not controlled performance comparisons. Toolchain remains C++20, LLVM23.1.0, OpenCASCADE7.9.3, Eigen5.0.1, Gmsh4.15.2-git and SuiteSparse7.14.0/CHOLMOD5.3.5 with METIS5.1.0. ASan/UBSan instrument project code and the Eigen adapter, **not the prebuilt numerical/CAD libraries**.

### New and extended regression coverage

- [Exact-cell regression](../../engineering/tests/test_a12_boundary.cpp): the actual four published vertices, geometric torus selection, trimmed-face projection, independent torus equation, nonzero linear volume, exterior centroid, inward material control point and explicit mm/µm conversion. No physical-CAD perturbation or relaxed tolerance substitutes for reproduction.
- [Full-shaft topology regression](../../engineering/tests/feed_boundary_topology.cmake): original136805-node/568908-tetrahedron mesh,142 common-surface cells,130 exterior centroids in that subset,9 A12 exterior cells and explicit presence of original element489924. At identical sizing, Netgen must produce zero common-surface cells, improved minimum SICN and remain within150000 nodes. Both modes use the same production mesher. It does not claim continuous containment.
- [End-to-end driver regression](../../engineering/tests/feed_fem_boundary_gate.cmake): optimized coarse A12 run, all20 regional stress rows and40 raw/bound CAD-location rows, optimizer/node provenance and mandatory scope hold. A separate original finest A12 run must retain its130 exterior common-surface centroids and nonzero exterior peak count, with both zero-count gates explicitly failing. Both runs complete with exit2 under `--require-converged`.
- [Analytic mesh regression](../../engineering/tests/test_refined_mesh.cpp): both optimizer paths on box/cylinder CAD, positive quality, independently integrated linear volume, surface-node/facet diagnostics, optimized topology and rejection of an invalid optimizer enumeration.
- [CLI rejection checks](../../engineering/tests/feed_fem_arguments.cmake): missing/unknown optimizer and unsupported high-order name, in addition to the existing size/focus/feature/phase validation.

The constant-strain stiffness/load/constraint formulation, six-coordinate rigid equilibrium/lifting, seven–twelve-coordinate Schur path, circular-bar finest113107-node level,4% compliance/12% tensor limits,5% stress/3% displacement gates and150000-node budget are retained. No cell percentile, smoothing, exterior-cell filter or stress-region omission is introduced.

## Independent artifact checks

All **eight numerical studies are complete and held**, with17 meshes,136 basis rows,340 regional rows,680 raw/bound CAD-location records and34 VTUs. Row counts, status/criteria hold counts, requested optimizer/sizing and unreleased flags were independently checked.

- **34/34 fields:** separate XML parser and all-six-array validation; finite numeric syntax/values, exact lengths, unique required arrays, connectivity/offset/type checks, nonnegative stresses and independently reconstructed governing-region maxima. Maximum CSV/field difference is **zero at printed precision**, limit1×10⁻⁷ MPa.
- **340/340 raw peaks:** the separate85-trimmed-face CAD audit agrees with every driver solid classification. The680 raw/bound records are explicitly two peak kinds, not680 independent load cases.
- **680/680 element indices:** independently reconstructed exported centroids agree with the recorded raw/bound locations within4.99995×10⁻¹⁰ mm, against1×10⁻⁷ mm. All340 raw positions also agree with the original regional CSV rows. No rendered-contour review is claimed.
- **Full-centroid audit:** original568908 cells have136 exterior centroids, including9 in A12; optimized542803 cells have25 exterior centroids, none in A12. Both have zero `on` centroids at1×10⁻⁷ mm classification tolerance. The25 non-A12 discrepancies are retained in the [complete list](../../engineering/generated/feed-fem-boundary/centroid-audit-netgen/outside-cell-centroids.csv), not filtered out of the FEM mesh. This remains a centroid test, not continuous containment.
- Both A12 series pass all eight local raw/bound checks and all eight specified finest-background0.20→0.17 mm comparisons. The front96-phase audit is enclosed by all20 corresponding48-phase bounds, minimum reserve0.0301218234 MPa.
- The new all-root series still has12 regional raw/bound refinement holds. Every focused study keeps its mandatory scope hold. A maximum0.00609627614811 volume discrepancy at the coarse all-root level is not a passed0.5% volume test; the original driver gates the finest level, which passes.

Extrema over all17 stress meshes:145366 nodes; minimum SICN0.0196478551819; maximum node/facet projection5.72522009334×10⁻⁷ /0.226661715394 mm; maximum equilibrium/constraint residual1.09931641426×10⁻¹⁰ /2.01826835323×10⁻¹⁶; maximum force/moment balance2.76506248983×10⁻¹⁵ /2.77555770486×10⁻¹⁶. Original compatibility meshes are included in these extrema.

### Compatibility and geometry hashes

Hashes cover literal XML bytes from file start through the closing Cells tag, **without an appended newline**. They concern geometry/connectivity, not the different displacement/stress load cases after that tag.

| Pair | SHA-256 |
|---|---|
| [Original front mesh](../../engineering/generated/feed-fem-cad-features/A12-front/working-4.vtu) / [original-method rerun](../../engineering/generated/feed-fem-boundary/baseline-A12-front/working-1.vtu) | af32b0ee2a63986ad0b9f8278e8ddefa85894cc5a90b0963b5bc4ad0c7e99b58 |
| [Original all-root](../../engineering/generated/feed-fem/working-4.vtu) / [compatibility rerun](../../engineering/generated/feed-fem-boundary/all-root-compatibility/working-1.vtu) | d56ff3f9540af25437d8dd7c9ff4f58fe1e8459e5d9526e28cb2e531c7b00bbc |
| [Optimized front48 phases](../../engineering/generated/feed-fem-boundary/A12-front/working-4.vtu) / [96 phases](../../engineering/generated/feed-fem-boundary/A12-front-phase/working-1.vtu) | 50ede673645e4d385f0ebba8127ba1739cad73c954d0b5fc712aca39a152de5a |

All20 original-method front raw/bound rows reproduce exactly at exported precision. The original all-root20 rows agree within5.00001817727×10⁻¹⁰ MPa. Old reports and49-test fingerprints remain historical evidence, not retroactively changed to match this increment.

## Source fingerprints

These identify the final52-test native/ASan/UBSan source and new numerical studies. The [49-test record](cad-feature-shaft-refinement.md) preserves its earlier hashes.

| File | SHA-256 |
|---|---|
| [Build configuration](../../engineering/CMakeLists.txt) | a7f526f55b929b8a6b43497213b41e7f21f4b9bd24c9f70b8c6a75892680ac3c |
| [FEM API](../../engineering/include/fem.hpp) | 42e64232ea4b5e5aea56b15e581f3f05736f2cdeb42d031b6a7cd90992ca7e65 |
| [Mesher and unchanged stiffness/constraint formulation](../../engineering/src/fem.cpp) | 6311296c01ba65d39bda2909b2df8ae35c64315c2f04e7cf9994e571c201b5d8 |
| [Stress driver and CAD gates](../../engineering/src/feed_fem_analysis.cpp) | 593b75a73e2c3ba37e969e931b5b992fecb7c2dba202b7fe342ed69ad361c7d7 |
| [Mesh-only boundary audit](../../engineering/src/feed_boundary_probe.cpp) | 733cd1f2760b6bf121ad257ce7b8a47c452555a6559774b79be623b447c49b34 |
| [Exact A12 regression](../../engineering/tests/test_a12_boundary.cpp) | f1fb6d1910d83c753db203238e06196098c65d705d56d117eefb111056d5d21d |
| [Analytic mesh regression](../../engineering/tests/test_refined_mesh.cpp) | 96af7ba5f6524d8775814cbcec445a1fa121743c579cb2bc480a335079a064cd |
| [Topology regression](../../engineering/tests/feed_boundary_topology.cmake) | 037fe0fce1d71dd255261565fbf1d609171971ff88aaf2d443440ac632912dd9 |
| [Driver geometry-gate regression](../../engineering/tests/feed_fem_boundary_gate.cmake) | c748c2cb76de54f4486d13cddfda3a46daeea2f691ff7953e6e4e92ecfdbc7a5 |
| [CLI regression](../../engineering/tests/feed_fem_arguments.cmake) | 624c0d35081480f9058abcc09a90cf9e6a386a53bbbfda5eac828fcccff32db5 |
| [Unchanged CAD-feature API](../../engineering/include/feed_refinement.hpp) | ca5d0cf0b19a078991ba22667e3d4303ec461c7fd48192d60b52faf98e69f9d4 |
| [Unchanged feature sizing](../../engineering/src/feed_refinement.cpp) | 647ae08b9ea0e6d62fef312b2b0048af2dfaaf30f1c90660b56cd0f1ebecc406 |
| [Unchanged independent peak audit](../../engineering/src/feed_fem_geometry_audit.cpp) | af8044cd5072f39bed160e2b054d3f488f6933d52b682932687562e23eae6ff0 |
| [Unchanged shaft loads and regions](../../engineering/src/feed_fem.cpp) | 5717f30824750233c5c76ed85b0a5cf2463adadc860de69452f3a6e8b271144f |
| [Unchanged shaft CAD](../../engineering/src/feed_geometry.cpp) | 618cd29f8901cca97f5fccc4f470a324369d0059806fa532f70b372bd9e534e3 |
| [Unchanged feed specification](../../shared/feed_drive_spec.hpp) | 3c1c725b93413f55609dbe4e725ec562a23a30d720dc5d5d402f6361e44bd236 |

## Release state

This record verifies the stated numerical implementation and data integrity, not physical accuracy, fatigue life, supplier dimensions, manufacturing capability or machine safety. Full geometry/region reconciliation, contact/preload/thread/tolerance/fatigue work and the complete machine fabrication/assembly package remain open. No supplier account, contact, purchase or authentication bypass was used. The **legacy reference** retains disabled outputs and `NotCommissioned`; it is distinct from the unqualified RP2350 target.