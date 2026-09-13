# 19 — Exterior-centroid diagnosis and torus-focused mesh resolution

2026-09-12. Numerical follow-up to the [A12 boundary-topology increment](18-a12-boundary-topology.md). **Physical authority remains head R6 / feed R6 / measurement R1. No physical CAD, load, region, acceptance limit or control state changed. No fabrication or energisation release.**

## Question

The finest optimized front-root mesh (136210 nodes /542803 tetrahedra) had **25 cells whose arithmetic centroids classify outside the exact nominal solid** at1×10⁻⁷ mm, none in the A12 band. That increment listed them but did not identify which trimmed CAD faces govern them or whether they are geometry defects. This record answers both questions with exact trimmed-face projections and then tests tighter mesh resolution on the governing curved features, without altering the shaft, discarding cells or weakening any limit.

## Exact diagnosis of the25 cells

Tools added for this purpose, all C++20 and covered by regression tests:

- [Geometry-only VTU reader](../engineering/src/mesh_io.cpp) (`starz_mesh_io`, LibXml2): ASCII linear-tetrahedron UnstructuredGrid only; DTDs, entities, binary/appended data, inconsistent counts, out-of-range connectivity and non-finite coordinates are rejected. [24 fail-closed cases](../engineering/tests/test_mesh_io.cpp) plus a production writer round-trip.
- [CAD boundary inspector](../engineering/src/cad_boundary.cpp) (`starz_cad_boundary`): exact solid classification, nearest trimmed face by `BRepExtrema_DistShapeShape`, support dimension, projected point and principal curvatures on all85 shaft faces.
- [Cell geometry audit](../engineering/src/feed_cell_geometry_audit.cpp): for each listed cell,19 diagnostic samples (4 vertices,6 edge midpoints,4 face centroids,4 degree-2 Gauss points, centroid). These are classification samples, **not** a new element formulation.

Published in [netgen-original](../engineering/generated/feed-fem-boundary-detail/netgen-original/status.json):25 cells,475 samples,85 faces. Every governing face is a **torus** — the concave fillet blends:

| Governing trimmed face | Torus (origin mm, axis, R / r mm) | Cells | Centroid gap range (µm) |
|---|---|---:|---:|
| 68 rear bearing shoulder R0.20 blend | (0,31.3,0), +S,6.2 /0.2 | 8 | 0.132–5.452 |
| 71 / 72 MB2 washer-entry floor blends | (∓1.6,31.7,5.65), −Z,0.35 /0.15 | 4 | 0.497–3.997 |
| 63 pulley key-floor rear end blend | (0,35.97,4.15), −Z,2.37 /0.15 | 4 | 1.696–2.624 |
| 61 pulley key-floor front end blend | (0,59.03,4.15), −Z,2.37 /0.15 | 8 | 0.0093–2.180 |
| 7 roller key-floor rear end blend | (0,95.97,−3.35), +Z,1.37 /0.15 | 1 | 1.987 |

Sample classification totals (inside / on / outside):vertices25 /75 /0, edge midpoints83 /7 /60, face centroids63 /0 /37, Gauss points25 /0 /75, centroids0 /0 /25. Each cell has three vertices on CAD faces and one interior vertex0.028–0.160 mm deep; the flat boundary triangle spans a concave curved patch, so its interior points lie micrometres outside the solid. Maximum gap0.0054519451894011 mm (cell284882). **No nominal CAD defect exists**: every projection lands on a trimmed face of the intended R0.15/R0.20 blends, and the [analytic torus regression](../engineering/tests/test_cad_boundary.cpp) reproduces four representative gaps from the full-torus equation within1×10⁻⁹ mm.

## Torus-focused mesh resolution

Two CAD-feature selections were added to [feed_refinement.cpp](../engineering/src/feed_refinement.cpp), selected by torus parameters rather than face IDs: `rear_shoulder_blends` (face68 plus the two washer-entry tori; region `rear_shoulder_and_slot_entry`) and `roller_key_floor` (both R1.37/r0.15 end tori; region `roller_key_root`). The existing `pulley_key_floor` already contains faces61 and63. The [mesh-only probe](../engineering/src/feed_boundary_probe.cpp) accepts `--focus FEATURE[,FEATURE...]`; sizing is the minimum over the selected guides, default unchanged (`A12_S19_62`). The [refinement test](../engineering/tests/test_feed_refinement.cpp) requires each of the four diagnosed centroids to be covered by its feature at less than its recorded gap and not by a distant feature (3144 trimmed-face lower-bound samples).

Same bulk0.85 mm, background0.20 mm, growth0.70, Netgen optimization, unchanged150000-node cap and0.5% volume criterion, every cell centroid classified:

| Focus | Root mm | Nodes / tets | Min SICN | Volume error | Exterior centroids / on |
|---|---:|---:|---:|---:|---:|
| `A12_S19_62` (published) | 0.009375 | 136210 /542803 | 0.10240457055039 | — | **25** /0 |
| four torus features | 0.009375 | over budget — [rejected before FEM](../engineering/generated/feed-fem-boundary-detail/netgen-torus-focus/status.json) | — | — | — |
| four torus features | 0.03 | [148934 /586087](../engineering/generated/feed-fem-boundary-detail/torus-focus-root-0.03/status.json) | 0.10402512795176 | 0.0014462730804417 | **3** /0 |
| four torus features | 0.02 | over budget — [rejected](../engineering/generated/feed-fem-boundary-detail/torus-focus-root-0.02/status.json) | — | — | — |

The [three remaining cells](../engineering/generated/feed-fem-boundary-detail/torus-focus-root-0.03/outside-cell-centroids.csv) are at S33.7259 (pulley slot side blend) and two at S19.616, on the A12 front root, which this run resolves at0.03 mm instead of0.009375 mm. Result: refining the governing fillet tori removes their exterior centroids, and the discrepancy appears on whichever curved blend is now coarsest. This is **not** monotone elimination or continuous containment. A run with all diagnosed tori at the published A12 root is not feasible inside the retained node budget; the cap was not raised.

## What this does and does not establish

- Established: the exact faces, gaps and curvature for all25 cells; they are linear-facet slivers on concave fillets, not geometry errors; a torus-focused sizing at0.03 mm reduces full-mesh exterior centroids25→3 within budget.
- Not established: continuous containment, stress results on the0.03 mm torus-focused mesh (the stress driver still takes one `--focus-feature`), convergence of the12 open all-root regional holds, or any change to the raw A12 values16.1699244824 /79.0315246074 MPa, which stand as published.

Global shaft convergence remains open. See the [verification record](verification/exterior-centroid-diagnosis.md).
