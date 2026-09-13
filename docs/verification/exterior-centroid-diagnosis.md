# Exterior-centroid diagnosis — verification record

2026-09-12. Numerical implementation and artifact verification only. **Physical authority remains head R6 / feed R6 / measurement R1. No fabrication or energisation release.** The [study record](../19-exterior-centroid-diagnosis.md) contains the diagnosis tables and the mesh-resolution results.

## Automated verification

**54/54 native tests passed in654.37 s** (two CTest workers). **54/54 ASan/UBSan tests passed in 740.25 s**, no sanitizer diagnostics. Sanitized execution used `ASAN_OPTIONS=halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`; project and Eigen adapter code are instrumented, prebuilt CAD/numerical libraries are not. The suite grows from52 to54 tests:

| New or extended regression | Scope |
|---|---|
| [`linear_VTU_geometry_reader`](../../engineering/tests/test_mesh_io.cpp) | Valid4-node fixture, production writer/readback,24 fail-closed cases (DTD/entities, binary/appended, count/connectivity/coordinate faults,150000-node cap). |
| [`feed_exterior_centroid_torus_regression`](../../engineering/tests/test_cad_boundary.cpp) | 85 faces; NaN rejection; four published cells (284882,74727,175303,261908) with recorded vertices: three `ON` /one `IN` vertices, exterior centroid, nearest face is the stated torus, OCC parameters within1×10⁻⁹, analytic full-torus distance equals trimmed projection within1×10⁻⁹ mm, recorded gap reproduced, principal curvature1/r. |
| [`feed_CAD_feature_refinement_benchmarks`](../../engineering/tests/test_feed_refinement.cpp) | Seven features,3144 trimmed-face lower-bound samples; `rear_shoulder_blends` must be exactly the R6.2/r0.2 shoulder torus plus both R0.35/r0.15 washer-entry tori; `roller_key_floor` exactly two R1.37/r0.15 tori; each diagnosed centroid covered below its recorded gap by its own feature and >60 mm from the other. |
| [`feed_boundary_topology`](../../engineering/tests/feed_boundary_topology.cmake) | Unchanged baseline/Netgen reproduction (136805/568908 and136210/542803) plus fail-closed `--focus` checks: unknown feature, empty list item, empty list and unknown option are rejected before meshing; default `focus_features` remains `A12_S19_62`. |

Before the successful native run, two consecutive launches of the freshly linked `test_feed_refinement` stalled in `_dyld_start` at0% CPU and were terminated (exit143); the third launch ran normally and passed. This was a process-launch stall, not a test failure, and is recorded for completeness.

## Independent checks on published artifacts

- The sanitized [cell geometry audit](../../engineering/src/feed_cell_geometry_audit.cpp) reproduces the native [cells.csv](../../engineering/generated/feed-fem-boundary-detail/netgen-original/cells.csv), points.csv, faces.csv and status.json **byte-identically**. Input list SHA-256 `2e8aa74c4fc96e7b213043e25a2ff9d2fd8426bd1fd4361a5dcd359b45f82853` (the25-row exterior list from the A12 increment).
- The audit requires every listed centroid to reproduce from the VTU within1×10⁻⁷ mm, manifold boundary incidence (each triangle in one or two cells) and centroid state `OUT`; it exits nonzero otherwise.
- Torus-focused probe runs at0.009375 and0.02 mm root ended with `run_complete:false` ("Over-budget refined FEM mesh"); their status files are retained rather than replaced by favourable levels. The completed0.03 mm run classifies all586087 centroids:3 outside,0 on; volume error0.0014462730804417 (<0.005); min SICN0.10402512795176; nodes148934 ≤150000.

## Source fingerprints (SHA-256)

| File | SHA-256 |
|---|---|
| [Build configuration](../../engineering/CMakeLists.txt) | a3375600db19f2534ee894b4f6d9a31c7671442b4d2e4af7a07a02b703b1237d |
| [VTU reader API](../../engineering/include/mesh_io.hpp) | d24284be9b272f2df6136ee9ed029a16775350d3024e863df9a0e9767f1925da |
| [VTU reader](../../engineering/src/mesh_io.cpp) | 70fef38c2c2288b7cbe5527f89cdf7eb02f82d501bd4a99672a560c0d12dfd73 |
| [VTU reader test](../../engineering/tests/test_mesh_io.cpp) | 58b278e8cafa3a57040ad04ce80711b3dac2cc6513be0f1cef1cd7b5fb4d29d2 |
| [CAD boundary API](../../engineering/include/cad_boundary.hpp) | e6dfaeaa665243a406e758bf3340a1354797161f877112bf1b9bb7e3b68bb297 |
| [CAD boundary inspector](../../engineering/src/cad_boundary.cpp) | ed9ff411da581e2fcf26bbeca05cb9c8a81b78d33967be6056f24ff4133103f3 |
| [Cell geometry audit](../../engineering/src/feed_cell_geometry_audit.cpp) | cbc9e07574498470ebf8b65bcd456799c5fbbc623d5be3bdba137cefdde7310c |
| [Torus fixture regression](../../engineering/tests/test_cad_boundary.cpp) | b01fd7dcb79610b7910f7506df4f291f21a75bee4ec7cd6c3a72654dbecee975 |
| [Unchanged feature API](../../engineering/include/feed_refinement.hpp) | ca5d0cf0b19a078991ba22667e3d4303ec461c7fd48192d60b52faf98e69f9d4 |
| [Feature sizing with two torus features](../../engineering/src/feed_refinement.cpp) | d4f4dbee31bcff8fbb5a64f503d5e7f7374ed50f5a41d0fe19901a77c84271a8 |
| [Mesh-only probe with `--focus`](../../engineering/src/feed_boundary_probe.cpp) | de6f3dcb7dd47f10c1809e7fd31dd007833cc09a7cdc2db40187322077fea490 |
| [Feature regression](../../engineering/tests/test_feed_refinement.cpp) | 35db745cd7d1a8de02557c62ff692b6e5d98ea4a0c3addaa9481d616c3199f52 |
| [Topology regression](../../engineering/tests/feed_boundary_topology.cmake) | 51ce9c1cadfc6c3a495c9825530c0273b7c437384990ff552d348502c6c1a040 |
| [Unchanged shaft CAD](../../engineering/src/feed_geometry.cpp) | 618cd29f8901cca97f5fccc4f470a324369d0059806fa532f70b372bd9e534e3 |
| [Unchanged mesher/FEM](../../engineering/src/fem.cpp) | 6311296c01ba65d39bda2909b2df8ae35c64315c2f04e7cf9994e571c201b5d8 |
| [Published25-cell audit table](../../engineering/generated/feed-fem-boundary-detail/netgen-original/cells.csv) | 1d59b1006e8099786830554947911686d8a6526cce74e9e137f2a4922ddaf6a2 |
| [Three remaining exterior centroids at0.03 mm](../../engineering/generated/feed-fem-boundary-detail/torus-focus-root-0.03/outside-cell-centroids.csv) | b321dfaa40aea09884eaa2b6ebc250dd6bb4105a8d0cf7d475209e36048672db |

The [52-test A12 record](a12-boundary-topology.md) keeps its own hashes; `feed_geometry.cpp` and `fem.cpp` are unchanged from that state.

## Release state

Mesh-only diagnosis and resolution study. Raw A12 stresses, all-root holds, focused scope holds, node cap, volume and stress gates are unchanged. Continuous containment is not claimed. **Not a fabrication, energisation or procurement release.**
