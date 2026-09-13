# CAD-feature shaft refinement — verification record

2026-09-12. Numerical implementation and artifact checks only. **Head R6 / feed R6 / measurement R1 remain the physical authority.** See the [study record](../17-cad-feature-shaft-refinement.md) for exact configurations, results, A12 boundary discrepancy and unreleased work.

**Historical49-test source state.** The subsequent [boundary-topology verification](a12-boundary-topology.md) has different meshing/audit code and its own tests and fingerprints. The old hashes below are preserved, not rewritten to imply revalidation of this exact source. The linked transient test logs may now contain later runs.

## Complete automated suites

| Configuration | Result | Total elapsed | Circular bar | CAD guide | CAD peak audit | CAD-feature gate |
|---|---:|---:|---:|---:|---:|---:|
| Native Release | **49/49 passed** | 440.53 s | 28.83 s | 3.05 s | 5.55 s | 6.04 s |
| RelWithDebInfo ASan/UBSan | **49/49 passed** | 707.15 s | 56.19 s | 3.94 s | 9.11 s | 17.21 s |

Two CTest workers in each run. Sanitized execution used `ASAN_OPTIONS=halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`. Other engineering work overlapped the runs; elapsed values are not controlled performance comparisons. No C++ or CMake changes occurred during the final suites or public studies. Subsequent native and sanitized Ninja dry runs both reported no work.

The native log (`build-engineering/Testing/Temporary/LastTest.log`, not kept) and sanitized log (`build-engineering-sanitize/Testing/Temporary/LastTest.log`, not kept) each contain49 passed records. Independent scans found no AddressSanitizer, UndefinedBehaviorSanitizer, runtime-error or LeakSanitizer diagnostics. These logs are overwritten by later test runs; the table and hashes here identify this completed state.

Toolchain remains C++20/LLVM23.1.0, OpenCASCADE7.9.3, Eigen5.0.1, Gmsh4.15.2-git, SuiteSparse7.14.0/CHOLMOD5.3.5 and METIS5.1.0. Project code and the Eigen adapter are instrumented; prebuilt third-party libraries are not. There is no claim that all dependencies were sanitizer-built.

### Added and extended regression coverage

- [CAD feature guide benchmark](../../engineering/tests/test_feed_refinement.cpp): all five feature selections, both distinct A12 roots, all four key-floor blends, observed M15/key peak coverage, nonfinite points, unknown features, wrong shape and empty guide. **2769 sampled comparisons** check sizing distance against independent trimmed-face projection. The lower-bound argument is analytical; the samples are regression evidence, not a continuous proof from sampling.
- [CAD peak-audit regression](../../engineering/tests/feed_fem_geometry_audit.cmake): a declared synthetic parser/geometry row at an interior45° point on the nominal A12 torus, correct membership/nearest type, malformed header, no data rows, NaN, invalid sign index, trailing field, and input/output alias rejection before modification. It does not manufacture a stress result from the synthetic row.
- [CAD-feature fail-closed regression](../../engineering/tests/feed_fem_feature_gate.cmake): full M15-slot-exit mesh1.6/.128, background.20, growth.55,8 phases. It completes, retains all20 regional rows and selected-face provenance, and `--require-converged` returns2. The focused scope hold and false release flags are mandatory.
- [CLI rejection regression](../../engineering/tests/feed_fem_arguments.cmake): missing/unknown CAD feature, feature without region and mismatched feature/region, in addition to the earlier mesh/background/growth/phase checks.
- The driver checks that the model returns exactly ten stress regions, six supports and eight solved bases. All original mechanical, protocol, calibration and elasticity tests remain in the49-test total.

The six-coordinate solver, Gmsh meshing backend,150000-node limit, raw5%/displacement3% gates and analytical circular-bar4% compliance/12% tensor thresholds are unchanged. There is no new numerical backend or untested physical CAD revision.

## Resolved development issues and retained rejects

An initial peak-audit test used the tangency edge shared by a torus and adjacent cylinder. The exact closest distances tied, and the classifier correctly selected the cylinder first; the test's unique-torus expectation was wrong. The regression now uses an interior45° fillet point. The corrected test passed individually and in both full suites. No nearest-face tie was hidden by changing the projection algorithm.

A duplicate transitive library link warning was removed before final builds. The over-budget170423-node A12 and151003-node pulley-floor meshes were rejected before the stress solve; the budget was not increased. Their incomplete statuses and research history are not passed simulation artifacts. Final public study runs all completed normally while retaining numerical/fabrication holds.

## Independent published-artifact checks

Nine completed study statuses,26 meshes,208 basis rows and520 regional rows were checked for row shape/count, criteria/status hold agreement and declared scope. A separate CAD audit contains85 faces and640 peak-location rows. Public artifacts are linked from the [study record](../17-cad-feature-shaft-refinement.md).

| Diagnostic over all26 meshes | Result |
|---|---:|
| Maximum nodes | 137503 |
| Minimum positive signed inverse-condition quality | 0.0196478551819 |
| Maximum surface-node CAD projection | 6.09405450759×10⁻⁷ mm |
| Maximum facet-centroid CAD projection | 0.147635738974 mm |
| Maximum relative CAD/mesh volume difference | 0.0044074305795 |
| Maximum equilibrium relative residual | 1.37486179932×10⁻¹⁰ |
| Maximum constraint relative residual | 3.28191319078×10⁻¹⁶ |
| Maximum force/moment relative balance residual | 2.76390106836×10⁻¹⁵ / 3.76698902792×10⁻¹⁶ |

Each public solve checks imported CAD volume against the independently generated nominal B-rep volume and Gmsh tetrahedral volume against FEM-assembled volume, with1×10⁻⁹ relative limits. Surface projection and positive quality are **not** exact curved-volume containment checks: the identified A12 front-root peak tetrahedron has its centroid0.000281177056 mm outside the nominal CAD solid despite four accurately located surface vertices. That finding and all raw peaks remain visible.

**All52 VTUs passed** independent large-document XML validation and checks of all six numeric arrays: required names/counts, finite numeric syntax and values, exact lengths, integer connectivity in range, offsets4/8/…/4N, tetrahedron type10 and nonnegative cell stress. Independently reconstructed centroid membership gives the selected region's raw maximum with **zero difference at printed precision**, against a1×10⁻⁷ MPa comparison limit. No cell filtering, smoothing or rendered contour review was used.

The M15 finest background changes are at most0.661357%; its96-phase raw results remain inside all twenty48-phase bounds, minimum reserve0.0202231474 MPa. Pulley background changes are at most2.755893%, but this comparison is explicitly at the **penultimate** level. These observations do not establish finest-background or all-root convergence for the pulley/slot system.

The default all-root rerun agrees with the original finest raw/bound rows within5.00001817727×10⁻¹⁰ MPa. Geometry hashes use literal XML bytes from file start through the closing Cells tag **without any appended newline**:

| Matching geometry/connectivity pair | SHA-256 |
|---|---|
| [Original all-root](../../engineering/generated/feed-fem/working-4.vtu) / [new compatibility](../../engineering/generated/feed-fem-cad-features/all-root-compatibility/working-1.vtu) | d56ff3f9540af25437d8dd7c9ff4f58fe1e8459e5d9526e28cb2e531c7b00bbc |
| [M15 finest48 phases](../../engineering/generated/feed-fem-cad-features/M15-slot/working-5.vtu) / [96 phases](../../engineering/generated/feed-fem-cad-features/M15-phase/working-1.vtu) | 0536c2dfbe02d8c3d1f53aee5eff7539acca2aa55c4f9faeab6296c95742acc0 |

## Historical49-test source fingerprints

The following identify the49-test source and final public studies. The [46-test verification](local-shaft-refinement.md) retains its historical hashes; they were not overwritten to imply a retest of its exact source.

| File | SHA-256 |
|---|---|
| [Build configuration](../../engineering/CMakeLists.txt) | b1c8fb7a19a9b4a423dee8ada1905f442812e159bd20e8942945089faa286cd1 |
| [CAD feature API](../../engineering/include/feed_refinement.hpp) | ca5d0cf0b19a078991ba22667e3d4303ec461c7fd48192d60b52faf98e69f9d4 |
| [CAD sizing guides](../../engineering/src/feed_refinement.cpp) | 647ae08b9ea0e6d62fef312b2b0048af2dfaaf30f1c90660b56cd0f1ebecc406 |
| [CAD peak audit](../../engineering/src/feed_fem_geometry_audit.cpp) | af8044cd5072f39bed160e2b054d3f488f6933d52b682932687562e23eae6ff0 |
| [Study driver](../../engineering/src/feed_fem_analysis.cpp) | ab61857398badb8e8df1bbec9391705a2818ece3bfdde8c5ea08d0a4fb25ad0e |
| [CAD guide regression](../../engineering/tests/test_feed_refinement.cpp) | 678fd1c0ec2a6095156fac52ccbdf37f6c94f6b0a03688b398276623d0083baa |
| [Feature-gate regression](../../engineering/tests/feed_fem_feature_gate.cmake) | 212b7daaff7f4ccf4585c6d0d603ebc4f52e2b9210228fc9d062d75bcd9131c5 |
| [CAD peak regression](../../engineering/tests/feed_fem_geometry_audit.cmake) | ec1554faecded52a04e9758ae9ca3af3b6cf480d20fc34dfa5f1e6f2c58d3161 |
| [CLI regression](../../engineering/tests/feed_fem_arguments.cmake) | 82945470c0287a08d14b5f09a50f2208e0bceac9679d58656f4f80036c381c89 |
| [Unchanged FEM solver](../../engineering/src/fem.cpp) | 97482a1393da4396347fe1d166c67997b84b4c3ebdc1ad8fc80e8850f1508d95 |
| [Unchanged shaft loads/original guides](../../engineering/src/feed_fem.cpp) | 5717f30824750233c5c76ed85b0a5cf2463adadc860de69452f3a6e8b271144f |
| [Unchanged shaft CAD](../../engineering/src/feed_geometry.cpp) | 618cd29f8901cca97f5fccc4f470a324369d0059806fa532f70b372bd9e534e3 |
| [Unchanged feed specification](../../shared/feed_drive_spec.hpp) | 3c1c725b93413f55609dbe4e725ec562a23a30d720dc5d5d402f6361e44bd236 |

## Release state

This verifies the added sizing/audit implementation and the integrity of its reported numerical results. It does not resolve A12, broader pulley/slot or all-root convergence, actual threads/edge breaks, fit/preload/contact, fatigue, measured loads, hardware safety or the complete fabrication/assembly package. No supplier account, contact, purchase or authentication bypass was used. Reference outputs remain disabled.