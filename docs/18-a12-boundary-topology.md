# F101 R6 — A12 boundary topology and stress refinement

2026-09-12. **Head R6 / feed R6 / measurement R1 remain the single physical design. No fabrication, energisation or production release.**

This increment reproduces the [previously identified A12 exterior-centroid cell](17-cad-feature-shaft-refinement.md), adds whole-volume Netgen optimization, and reruns the unchanged shaft/load model. Both A12 root series now pass their local last-refinement checks and the stated finest-background comparison. This is a scoped numerical improvement, **not complete curved-volume conformity or full-shaft stress qualification**.

## Reproduced defect and implemented change

The [deterministic regression](../engineering/tests/test_a12_boundary.cpp) uses the four published vertices of zero-based tetrahedron489924 from the original finest front-root mesh. It independently verifies the actual trimmed torus and its analytic equation:

- All four vertices lie on the nominal R0.04 mm torus; maximum trimmed-face distance3.00812×10⁻¹¹ mm.
- Linear tetrahedral volume1.375758729×10⁻⁸ mm³ is finite and positive.
- Its centroid is **outside** the original solid by0.000281177057 mm, or **0.281177057 µm**, not0.281 mm.
- An inward control point classifies inside the solid. No shaft radius or tolerance was changed to make the regression pass.

The [mesher](../engineering/src/fem.cpp) now accepts `RefinedMeshOptimizer::netgen`; the [study driver](../engineering/src/feed_fem_analysis.cpp) exposes `--mesh-optimizer netgen`. It runs Gmsh's Netgen **volume optimization after the existing linear Delaunay generation**. It changes the mesh topology, not the physical CAD, loads, material or support equations. No tetrahedron is removed from stress evaluation by a filter. The elements remain four-node, constant-strain tetrahedra; this is **not quadratic or exact curvilinear FEM**.

The default `gmsh` mode remains the original method for reproducibility. Its original finest front-root geometry/connectivity and all20 raw/bound regional values reproduce exactly at exported precision. The original all-root geometry also reproduces; maximum raw/bound difference5.00002×10⁻¹⁰ MPa. The implementation does not silently relabel old meshes as optimized.

### New fail-closed geometry checks

Every refined mesh records tetrahedra whose four vertices share one Gmsh CAD surface. Each tetrahedron is counted once even if several surface memberships coincide. Such membership alone does not prove an invalid cell: the exact solid classifier separately tests its centroid.

The driver also classifies the governing **raw and phase-bound** cell centroid for every load class and stress region:40 locations per mesh. Two new zero-count gates supplement, rather than replace, the original criteria:

- `no_exterior_same_surface_centroids`
- `all_reported_peak_centroids_in_CAD`

These gates are evaluated at the finest level, like the original final strength/volume checks. Every level's cell/peak records remain available. Classification tolerance is1×10⁻⁷ mm, a numerical tolerance, not a manufacturing allowance. Unknown CAD classifications abort the analysis rather than becoming passes. All stresses, including exterior-cell values in the baseline reruns, remain in the results and fields.

The mandatory focused-study hold,150000-node budget,5% raw/bound stress-change criterion,3% displacement-change criterion,4% circular-bar compliance limit,12% tensor RMS limit and original strength limits are unchanged. The exact six-coordinate constraint solver and physical supports are unchanged.

## Full-centroid audit: improvement and remaining discrepancy

The [mesh-only audit](../engineering/src/feed_boundary_probe.cpp), with its explicit `--all-centroids` option, checks **every** tetrahedron centroid against the unchanged original solid. It uses the same production mesher, not a duplicate set of mesh settings. These two audits use identical bulk/root0.85/0.009375 mm, background0.20 mm, growth0.70 and front-root CAD feature.

| Diagnostic | Original mesh | Netgen-optimized mesh |
|---|---:|---:|
| Nodes | 136805 | 136210 |
| Tetrahedra / centroids checked | 568908 | 542803 |
| Minimum positive SICN | 0.0292786436496 | 0.1024045705504 |
| Tetrahedra with four nodes on one CAD surface | 142 | 0 |
| Exterior centroids among that subset | 130 | 0 |
| Exterior centroids over **all** cells | **136** | **25** |
| Exterior centroids in the A12 reporting region | **9** | **0** |
| Centroids classified on the boundary | 0 | 0 |

Evidence: [original audit status](../engineering/generated/feed-fem-boundary/centroid-audit-baseline/status.json), [original exterior-centroid list](../engineering/generated/feed-fem-boundary/centroid-audit-baseline/outside-cell-centroids.csv), [optimized audit status](../engineering/generated/feed-fem-boundary/centroid-audit-netgen/status.json), and [all25 remaining exterior centroids](../engineering/generated/feed-fem-boundary/centroid-audit-netgen/outside-cell-centroids.csv).

The25 remaining points comprise12 near the rear shoulder/slot entry,12 near the pulley-key floor and one near the roller key. None is in the A12 band. They are **not deleted or hidden**. Their existence shows why the same-surface topology check and the40 peak-location checks are not complete geometry certification. Even checking every centroid does not establish containment of every point of each flat-faced tetrahedron or a continuous Hausdorff bound. The complete centroid comparison above concerns the **front-root focused mesh**, not every published mesh or the rear-root series.

## Published stress studies

Eight completed numerical studies contain **17 meshes,136 basis rows,340 regional rows,680 raw/bound CAD-peak records and34 governing-case VTUs**. A separate trimmed-face audit covers all340 raw peaks. Two mesh-only full-centroid audits are separate from those17 stress meshes.

Both primary A12 series use the same four bulk/root pairs as their historical counterparts: **1.6/.032;1.2/.020;1/.0125;.85/.009375 mm**. Background0.20 mm, growth0.70 and48 angular phases remain fixed throughout each series. All other roots remain in the complete shaft and retain their own reported regions.

| Study | Optimizer | Levels | Finest nodes / tetrahedra | Remaining gates |
|---|---|---:|---:|---:|
| [A12 front](../engineering/generated/feed-fem-boundary/A12-front/report.md) | Netgen | 4 | 136210 /542803 | 3 |
| [A12 rear](../engineering/generated/feed-fem-boundary/A12-rear/report.md) | Netgen | 4 | 136268 /543158 | 7 |
| [Front finest background](../engineering/generated/feed-fem-boundary/A12-front-background/report.md) | Netgen | 1 | 145293 /581878 | 2 |
| [Rear finest background](../engineering/generated/feed-fem-boundary/A12-rear-background/report.md) | Netgen | 1 | 145366 /582306 | 2 |
| [Front96-phase audit](../engineering/generated/feed-fem-boundary/A12-front-phase/report.md) | Netgen | 1 | 136210 /542803 | 2 |
| [All-root series](../engineering/generated/feed-fem-boundary/all-root-netgen/report.md) | Netgen | 4 | 129744 /565040 | 12 |
| [Historical front-root reproduction](../engineering/generated/feed-fem-boundary/baseline-A12-front/report.md) | Original | 1 | 136805 /568908 | 4 |
| [Historical all-root reproduction](../engineering/generated/feed-fem-boundary/all-root-compatibility/report.md) | Original | 1 | 131909 /603290 | 3 |

### A12 local results

MPa. Last change uses the finer result as denominator. Raw peaks are unsmoothed elemental maxima. Phase bounds enclose unsampled angles for each discrete linear model, not spatial/contact/tolerance errors.

| Root / load | Finest raw peak | Finest phase bound | Raw last change | Bound last change |
|---|---:|---:|---:|---:|
| Front working | 16.1699244824 | 16.2004215636 | 3.797510% | 3.794541% |
| Front fault | 79.0315246074 | 79.0620216886 | 3.839954% | 3.839329% |
| Rear working | 15.4323680574 | 15.4488814314 | 1.486476% | 1.492954% |
| Rear fault | 76.2082916648 | 76.2248050388 | 1.420703% | 1.422030% |

**All eight local checks pass5%.** Both finest primary meshes have zero same-surface tetrahedra and zero exterior classifications among their40 reported raw/bound peak locations. The original front values22.4377916/109.684574 MPa are preserved in the baseline rerun and original publication; they were not replaced by values computed after discarding selected cells.

The front series still holds the mandatory scope gate and the two fault front-bearing-shoulder refinement gates. The rear series additionally holds four M15 raw/bound working/fault refinement gates. These unrefined-background regions have not been declared converged by the A12 result.

### Finest-background and angular checks

The finest root and bulk targets are held fixed while background target decreases **0.20→0.17 mm**, a15% refinement chosen within the existing node budget. This is the stated sensitivity comparison, not the earlier0.13 mm background study and not proof of independence from arbitrarily fine backgrounds.

| Root / load | Background raw MPa | Background bound MPa | Raw change | Bound change |
|---|---:|---:|---:|---:|
| Front working | 15.5905666291 | 15.6183869885 | 3.716080% | 3.726598% |
| Front fault | 76.3012402941 | 76.3290606534 | 3.578296% | 3.580499% |
| Rear working | 15.4965866526 | 15.5122847117 | 0.414405% | 0.408729% |
| Rear fault | 76.5662851135 | 76.5819831725 | 0.467560% | 0.466400% |

All eight comparisons are below5%; maximum145366 nodes. The96-phase front audit uses identical geometry/connectivity to its48-phase counterpart. All20 regional96-phase raw peaks remain inside their respective48-phase bounds; minimum reserve0.0301218234 MPa. This is angular validation, not an additional spatial mesh.

### Whole-shaft reconciliation remains open

The all-root Netgen series reruns the original bulk/root2/.30,1.6/.20,1.2/.13,1/.09 mm sequence. Finest working/fault bounds are79.5960417/165.799134 MPa; working displacement bound0.0206177603 mm. The new geometry gates and final volume/displacement checks pass, but **12 raw/bound regional refinement gates remain unmet**: M15 relief, roller shoulder and roller-key root, each for both load classes and both peak measures.

Passing A12 last changes on this coarser all-root sequence does not establish agreement with the locally refined A12 meshes. Nor does this all-root run transfer the older M15 or pulley-floor focused qualifications to the new mesh topology. The historical16 all-root holds remain part of their own source/mesh record. The eight A12 local passes above are a separate check matrix, not a subtraction from that historical count.

## Artifact verification and limits

- All34 VTUs passed an independent XML parser and all-six-array checks: required arrays exactly once, finite numeric syntax/values, exact lengths, integer in-range connectivity, offsets4n, tetrahedron type10 and nonnegative stress. Independently reconstructed governing-region maxima match every CSV raw peak with **zero difference at printed precision**, against1×10⁻⁷ MPa.
- The [separate CAD peak audit](../engineering/generated/feed-fem-boundary/peak-CAD-audit/peak-geometry.csv) projects all340 raw peaks onto all85 trimmed faces. Its classifications agree with the driver's records for every row.
- All680 raw/bound element indices independently reproduce the exported centroids; maximum coordinate difference4.99995×10⁻¹⁰ mm against1×10⁻⁷ mm. All340 raw locations also match the regional tables. No rendered-contour review is claimed.
- Across all17 stress meshes, maximum145366 nodes; minimum SICN0.0196478551819; maximum node projection5.72522009334×10⁻⁷ mm; maximum facet-centroid projection0.226661715394 mm. These extrema include the original compatibility meshes and the coarse all-root level, not only the finest optimized A12 meshes.
- Maximum relative volume discrepancy0.00609627614811 occurs in the coarse all-root series and exceeds0.5%; it is **not** described as a passed level. The driver retains its original finest-level volume gate. All studies remain numerically held and unreleased.
- Maximum equilibrium, constraint, force-balance and moment-balance relative residuals are1.09931641426×10⁻¹⁰,2.01826835323×10⁻¹⁶,2.76506248983×10⁻¹⁵ and2.77555770486×10⁻¹⁶ respectively.

The targeted six-test integration run passed in90.47 s. Complete final suites passed **52/52 native tests in579.68 s** and **52/52 ASan/UBSan tests in873.81 s**, with no sanitizer diagnostics. The optional complete542803-centroid audit also agrees natively and under sanitizers, including the identical25-row exterior list. See the [verification record and final source fingerprints](verification/a12-boundary-topology.md).

Remaining work includes the25 identified non-A12 exterior centroids, broader mesh/region reconciliation, true threads and final edge breaks, dimensional tolerances, fits/preload/contact, rotating fatigue, actual pressure/friction/jam loads, role-specific torque paths, and the unfinished complete machine mechanisms, electrical/safety integration, sourcing, drawing and assembly package. The ±60 N roller envelope is not an established jam-force limit:5 Nm at20 mm radius could transmit250 N tangentially. The **legacy reference** retains disabled outputs and `NotCommissioned`; that historical model is distinct from the unqualified RP2350 target.