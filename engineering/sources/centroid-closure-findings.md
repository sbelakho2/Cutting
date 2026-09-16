# Whole-shaft exterior-centroid gate closure: config sweep and final rerun

2026-09-16. Record of the `no_exterior_same_surface_centroids` gate closure
attempt on the whole-shaft gmsh torus-focused four-mesh series
(`engineering/generated/feed-fem-torus/gmsh`). Generated reports:
[gmsh](../generated/feed-fem-torus/gmsh/report.md),
[Netgen](../generated/feed-fem-torus/netgen/report.md).

## Question

The gmsh four-mesh series reported **34 exterior same-surface centroids**
(fail-closed audit gate: 0 required). The 34 decompose as **32 on the two A12
ring-groove tori** (16 front surface 79, 16 rear surface 81) plus **2 on the
blind bore** (drill-point cone surface 23 at S94.85 and cosmetic-thread
envelope cylinder surface 17 at S105). The five diagnosed fillet tori are
already clean at the 0.09 mm level root. The gate may only close with a
meshing configuration that removes every same-surface sliver tetrahedron at
≤150000 nodes without changing loads, phases, gates or the node cap.

## Tools added (all default-off; existing paths unchanged)

- `engineering/src/feed_refinement.cpp`: whole-shaft feature subsets —
  `five_fillet_tori` (the six diagnosed concave fillet tori, 6 faces),
  `five_fillet_tori_a12` (8 faces), `a12_roots` (both A12 ring-groove tori,
  2 faces), `concave_fillet_tori_and_bore` (the 9 tori plus the blind-bore
  cone and cylinder faces of S93.4–113.1, 13 faces). `concave_fillet_tori`
  (9 faces) is unchanged.
- `engineering/src/feed_fem_analysis.cpp`: `--torus-root mm` sets the root of
  the CAD-feature guide term independently of the level roots (valid
  0.005–0.2 mm, ≤ every level root, requires a feature). Default behaviour
  (feature root = level root) is byte-unchanged.
- `engineering/src/feed_boundary_probe.cpp`: `--background-root`,
  `--focus-growth`, `--torus-root` so the probe reproduces the four-mesh
  series sizing field exactly. Defaults (0.2 / 0.7 / level root) are
  unchanged and the pinned topology regression still reproduces the
  historical 136805-node baseline byte-for-byte.
- Tests: `test_feed_refinement.cpp` composition checks for the four new
  features; `feed_fem_arguments.cmake` rejection cases for `--torus-root`;
  `feed_boundary_topology.cmake` rejection cases for the new probe options.

The probe was first validated against the published series: bulk 1.0 / root
0.09 / background 0.09 / growth 0.45 / `concave_fillet_tori` / gmsh optimizer
reproduces the level-4 mesh **exactly** (132174 nodes, 604251 tetrahedra,
34 exterior, 32/32 A12).

## Sweep (probe, bulk 1.0 mm, all-root root 0.09 mm, background 0.09 mm)

| Config | Optimizer | Feature | Torus root mm | Growth | Nodes | Exterior |
|---|---|---|---|---|---|---|
| series reproduction | gmsh | concave_fillet_tori (9) | 0.09 | 0.45 | 132174 | 34 |
| a12_r04 | gmsh | a12_roots (2) | 0.04 | 0.45 | 139398 | 10 |
| a12_r03 | gmsh | a12_roots (2) | 0.03 | 0.45 | 142187 | 6 |
| a12_r025 | gmsh | a12_roots (2) | 0.025 | 0.45 | 142855 | 8 |
| a12_r025_g03 | gmsh | a12_roots (2) | 0.025 | 0.30 | over budget | — |
| a12_r035_g04 | gmsh | a12_roots (2) | 0.035 | 0.40 | 143690 | 8 |
| a12_r04_g035 | gmsh | a12_roots (2) | 0.04 | 0.35 | 146165 | 7 |
| a12_r02 | gmsh | a12_roots (2) | 0.02 | 0.45 | 143816 | 9 |
| a12_r03_g06 | gmsh | a12_roots (2) | 0.03 | 0.60 | 136322 | 5 |
| a12_r025_g05 | gmsh | a12_roots (2) | 0.025 | 0.50 | 140441 | 3 |
| a12_r02_g06 | gmsh | a12_roots (2) | 0.02 | 0.60 | 137454 | 4 |
| fiveA12_r03 | gmsh | five_fillet_tori_a12 (8) | 0.03 | 0.45 | over budget | — |
| fiveA12_r03_g035 | gmsh | five_fillet_tori_a12 (8) | 0.03 | 0.35 | over budget | — |
| fiveA12_r025_g04 | gmsh | five_fillet_tori_a12 (8) | 0.025 | 0.40 | over budget | — |
| fiveA12_r04 | gmsh | five_fillet_tori_a12 (8) | 0.04 | 0.45 | 145731 | 3 |
| nine_r03 | gmsh | concave_fillet_tori (9) | 0.03 | 0.45 | over budget | — |
| nine_r03_g035 | gmsh | concave_fillet_tori (9) | 0.03 | 0.35 | over budget | — |
| nine_r025 | gmsh | concave_fillet_tori (9) | 0.025 | 0.45 | over budget | — |
| nine_r04 | gmsh | concave_fillet_tori (9) | 0.04 | 0.45 | 145944 | 4 |
| nine_r035 | gmsh | concave_fillet_tori (9) | 0.035 | 0.45 | 148510 | **2** |
| ninebore_r04 | gmsh | +blind bore (13) | 0.04 | 0.45 | over budget | — |
| netgen_default | **netgen** | concave_fillet_tori (9) | 0.09 | 0.45 | **129936** | **0** |
| netgen_r03 | netgen | concave_fillet_tori (9) | 0.03 | 0.45 | 147561 | 0 |
| netgen_a12_r03 | netgen | a12_roots (2) | 0.03 | 0.45 | 139581 | 0 |
| netgen_ninebore_r03 | netgen | +blind bore (13) | 0.03 | 0.45 | over budget | — |

Every `over budget` row exceeded the unchanged 150000-node cap and was
rejected by the mesh generator before any audit; no cap or gate was relaxed.

## Findings

1. **A12 is the dominant residual.** The A12 ring-groove tori (minor radius
   0.04 mm) produce the 32 A12 exterior cells of the published series.
   Torus-root refinement to 0.04–0.025 mm removes most of them but leaves
   1–5 sliver tetrahedra whose four nodes all sit on the concave torus
   (centroid inside the tube by construction) whenever the local element
   size stays above ≈4× the local concave radius; the count is a
   non-monotone Delaunay topology lottery (0.02 mm root gives 5 A12 cells
   while 0.025 mm gives 3).
2. **The gmsh optimizer cannot reach zero within budget.** The best gmsh
   configuration found (`concave_fillet_tori`, torus root 0.035 mm, growth
   0.45) leaves **2 exterior** at 148510 nodes — the cap edge. Adding the
   blind bore to the guide (needed to kill the two bore-wall slivers, which
   torus refinement never touches) exceeds the cap at any root small enough
   to matter (the bore annulus is ≈175 mm³ and costs ≈340k nodes at a
   0.09 mm wall size).
3. **The Netgen volume optimizer eliminates the pathology entirely.** Its
   topological optimization produces **zero same-surface tetrahedra** on
   every level, so the exterior count is identically zero for any sizing;
   the unchanged series configuration (2/.30, 1.6/.20, 1.2/.13, 1/.09 mm,
   `concave_fillet_tori`, growth 0.45) reports **0 exterior at 129936
   finest nodes** — the first ≤150000-node configuration with 0 exterior in
   the sweep and the closest to the series semantics.
4. **Decision.** The whole-shaft four-mesh series in
   `engineering/generated/feed-fem-torus/gmsh` is rerun with the winning
   configuration: levels, feature, growth, loads, 48 phases, torque
   vertices, gravity and every gate unchanged; only the volume optimizer
   changes to Netgen. The rerun reproduces the Netgen series
   byte-identically (deterministic meshing), 0 exterior same-surface
   centroids and 0 exterior peak centroids at 129936 ≤ 150000 nodes, so the
   fail-closed CAD boundary audit gate passes. The ten regional 5%
   refinement holds remain, and they are the ones already converged by the
   regional submodel record (docs/28). The Netgen series directory is
   untouched; the earlier gmsh-optimizer results remain recorded in
   docs/19, docs/26 and the growth-0.70 record directories.

## Verification

- The rerun of `engineering/generated/feed-fem-torus/gmsh` is byte-identical
  to the untouched `netgen` series on every artifact (report.md, criteria.csv,
  same-surface-cells.csv, regions.csv, basis.csv, mesh-quality.csv,
  surface-audit.csv, peak-cad-states.csv, sizing-faces.csv, all eight VTUs);
  `status.json` differs only by the new `torus_root_mm:0` field.
- `criteria.csv`: `no_exterior_same_surface_centroids,0,0,PASS_SCOPED` and
  `all_reported_peak_centroids_in_CAD,0,0,PASS_SCOPED`; the ten regional
  `REFINE_OR_FAIL` rows are exactly the ten submodel-converged holds.
- Engineering tests: all 27 `ctest -R feed` tests pass, including the new
  feature compositions, the `--torus-root` rejections, the new probe option
  rejections and the pinned boundary-topology regression (which still
  reproduces the historical 136805-node baseline). Full 87-test suite:
  85/87 pass; the two failures are the concurrent collar-contact increment's
  `collar_contact_benchmarks` / `collar_contact_report` ("Unknown OpenCASCADE
  volume with tag 3"), unrelated to this work and untouched here.

## Release state

`numerical_gates_passed` remains false (ten regional holds), `contact_verified`,
`true_threads_verified`, `fatigue_verified` and `fabrication_released` remain
false. No acceptance criterion, load, phase, region or node cap changed; no
failed raw data was deleted. **Not a fabrication, energisation or procurement
release.**
