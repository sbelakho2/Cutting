# F101 R6 — focused full-shaft refinement

2026-09-12. **Head R6 / feed R6 / measurement R1 remain unchanged. No fabrication, energisation or production release.**

This is the preserved46-test/26-mesh increment. The subsequent [CAD-feature study](17-cad-feature-shaft-refinement.md) adds M15 local/background results and identifies an A12 curved-boundary mesh discrepancy. Its results do not overwrite the studies below.

This increment investigates the six feature families left unresolved by the [original four-mesh study](verification/feed-shaft-fem.md). It adds full-shaft focused refinement, independent mesh-geometry audits and an exact six-coordinate solver specialization. It does not modify the selected parts, shaft geometry, loads, supports, material, strength limits or 150000-node budget.

**Outcome:** neck shoulder, roller shoulder and roller-key root pass their local last-refinement checks and the separate background-refinement comparisons below. A12 groove, M15 relief and pulley-key/MB2 slot retain unresolved local checks. These scoped results do not turn the original16 holds into a new all-root pass.

## One physical model; focused mesh allocation

Every solve uses the complete nominal F101 STEP, all eight load bases, all eight radial-sign corners, six transmission-torque vertices and gravity. No shaft segment is cut out and no guessed submodel boundary displacement is imposed. All ten stress regions and both load classes remain in every regional CSV.

Let $d_a$ be the original all-root sizing guide and $d_f$ the guide for the selected feature. Focused sizing is

$$h(p)=\min\left(h_b,\ h_{bg}+0.45d_a(p),\ h_f+\gamma d_f(p)\right).$$

The original all-root mode is unchanged: $h(p)=\min(h_b,h_f+0.45d_a(p))$. The feature guides reconstruct exactly the original minimum-distance field; the regression checks this on a grid. They are sizing guides, not a substitute for CAD or a certified distance-to-every-root calculation.

The [driver](../engineering/src/feed_fem_analysis.cpp) accepts `--focus-region`, `--background-root` and `--focus-growth` in addition to its original arguments. Both bulk and focused root sizes must strictly decrease within a series. Background size stays fixed within that series. Focused minimum size can reach0.005 mm; the all-root minimum remains0.02 mm. The node limit remains150000. Smaller permitted input is not proof of an adequately resolved root.

Every focused report has a mandatory `focused_study_requires_background_and_all_root_verification` hold. Therefore `--require-converged` cannot report a focused study as full-shaft spatial convergence merely because its non-refined regions happen to change little. No raw-peak gate is removed, no percentile replaces a maximum, and every original5% stress/3% displacement criterion remains.

## Published refinement series

All primary series use48 phases and a0.20 mm background-root target. Sizes below are bulk/root in mm, in execution order.

| Feature and report | Mesh sequence | Growth $\gamma$ | Finest nodes / tetrahedra |
|---|---|---:|---:|
| [A12 groove](../engineering/generated/feed-fem-focused/A12-groove/report.md) | 1.2/.03; 1.0/.02; .9/.015 | .70 | 138057 / 570261 |
| [M15 relief](../engineering/generated/feed-fem-focused/M15-relief/report.md) | 1.6/.128; 1.25/.08; 1.0/.05; .85/.0333333333333 | .45 | 143887 / 617446 |
| [Neck shoulder](../engineering/generated/feed-fem-focused/neck-shoulder/report.md) | 1.6/.16; 1.25/.10; 1.0/.0625; .85/.04 | .45 | 112291 / 479646 |
| [Pulley key / MB2 slot](../engineering/generated/feed-fem-focused/pulley-key/report.md) | 1.6/.128; 1.25/.08; 1.0/.05 | .45 | 130811 / 670747 |
| [Roller key](../engineering/generated/feed-fem-focused/roller-key/report.md) | 1.6/.128; 1.25/.08; 1.0/.05 | .45 | 93512 / 468568 |
| [Roller shoulder](../engineering/generated/feed-fem-focused/roller-shoulder/report.md) | 1.6/.128; 1.25/.08; 1.0/.05; .85/.0333333333333 | .45 | 88348 / 386573 |

The A12 studies allocate elements around the0.04 mm nominal groove roots. Earlier diagnostic requests at .85/.016 with growth.45 and .85/.0133333333333 with growth.70 produced161344 and165426 nodes respectively; both were rejected before solving. The budget was not increased. The accepted .015 mm level is25% smaller than the preceding .02 mm target, not a nearly identical mesh chosen to hide a change. Its working result only narrowly passes5%, while its fault result still fails.

### Finest named-region results

Stress values are MPa. “Bound” covers unsampled rotation for that discrete mesh, **not spatial error or omitted physical effects**. Last change is $|a-b|/|b|$ with $b$ the finer result. These are the selected feature's peaks, not the full-shaft maximum printed in the report summary.

| Feature / load | Sampled raw peak | Phase bound | Raw last change | Bound last change |
|---|---:|---:|---:|---:|
| A12 working | 16.4332462 | 16.4515830 | 4.988915% | 4.921099% |
| A12 fault | 81.1136200 | 81.1319569 | **5.827147%** | **5.813206%** |
| M15 working | 62.2833457 | 65.5140740 | 3.783379% | 3.707738% |
| M15 fault | 114.202632 | 117.433360 | **5.640769%** | **5.870292%** |
| Neck working | 26.8135387 | 28.4642794 | 3.543544% | 3.625045% |
| Neck fault | 52.1565681 | 53.7070438 | 1.848999% | 1.814587% |
| Pulley key/slot working | 44.2577304 | 46.9114933 | **10.370285%** | **10.187668%** |
| Pulley key/slot fault | 78.2446773 | 79.8933644 | **10.829084%** | **10.796568%** |
| Roller key working | 46.3445247 | 47.5433414 | 0.530872% | 0.499920% |
| Roller key fault | 165.344646 | 166.543463 | 1.809173% | 1.791158% |
| Roller shoulder working | 46.0080570 | 48.5558098 | 3.784200% | 3.559223% |
| Roller shoulder fault | 110.525696 | 112.498723 | 3.521819% | 3.198477% |

Six feature families × two loads × two peak checks gives24 focused last-change checks:16 scoped passes and8 unmet. This is a different matrix from the original all-root study's16 unmet checks. In particular, further local refinement exposed fault sensitivity at M15 and the pulley slot that was not a failed last-step check in the original sequence. Results are not combined by selecting whichever mesh makes each region pass.

The six complete primary reports retain15,5,9,21,3 and12 total holds respectively in the table's study order, including nonfocused regional checks and the mandatory scope hold. All have `numerical_gates_passed:false` and `fabrication_released:false`.

## Independent background sensitivity

For each locally stable feature, a separate solve retains its finest bulk/root target and reduces every other root's background target from0.20 to0.13 mm. These are single-mesh sensitivity checks, not additional points silently spliced into a differently configured refinement series.

| Audit | Nodes / tetrahedra | Working raw / bound change | Fault raw / bound change |
|---|---:|---:|---:|
| [Neck background](../engineering/generated/feed-fem-focused/neck-background-audit/report.md) | 140666 / 613069 | 1.697125% / 1.529185% | 0.054691% / 0.019712% |
| [Roller-key background](../engineering/generated/feed-fem-focused/roller-key-background-audit/report.md) | 122616 / 601148 | 0.727845% / 0.725212% | 0.774677% / 0.773568% |
| [Roller-shoulder background](../engineering/generated/feed-fem-focused/roller-shoulder-background-audit/report.md) | 119235 / 530662 | 1.714865% / 1.533085% | 2.014182% / 1.928637% |

Each comparison is below the unchanged5% stress-change criterion. This supports those three local results under the stated ideal-support model. It is not proof of insensitivity to every background mesh, physical bearing stiffness, interference fit, preload or contact condition. Single-mesh and focused-scope holds remain active in every audit.

## Geometry, phase and exported fields

- Eleven completed studies: six primary series, three background audits, one [all-root compatibility run](../engineering/generated/feed-fem-focused/all-root-compatibility/report.md) and one [A12 96-phase audit](../engineering/generated/feed-fem-focused/A12-phase-audit/report.md).
-26 meshes,520 regional rows and52 governing-case VTUs. Maximum node count143887. Every exported field passed XML and independent six-array/region-maximum checks; no rendered contour review is claimed.
- All refined tetrahedra have positive signed inverse-condition quality and positive volume. Across these meshes, minimum `min_SICN` is0.0196478551819. This is a validity diagnostic, not a stress-error bound.
- Maximum CAD-node projection distance is5.72522009334×10⁻⁷ mm, below1×10⁻⁶ mm. Maximum facet-centroid projection distance across all levels is0.1672627946 mm; it remains reported rather than hidden by nodal agreement. Projection is to each underlying CAD surface, not a trimmed-surface/continuous Hausdorff proof.
- Every imported CAD volume matches the independent nominal shaft volume within1×10⁻⁹ relative; Gmsh cell volume matches independently assembled FEM volume within the same limit. Finest A12 CAD/FE volume difference is0.158325%; this does not quantify local stress error.
- Eight-basis maximum equilibrium and constraint residuals across all26 meshes are1.4780486172×10⁻¹⁰ and2.07673291596×10⁻¹⁶ respectively. Force/moment checks also remain below1×10⁻⁷.
- The A12 96-phase run uses the exact same .9/.015 mesh as its48-phase parent. All20 regional raw peaks stay below the corresponding48-phase bounds; minimum reserve0.0183368773 MPa. This is an angular check only.

In a focused study, numbered working/fault VTUs show the single sampled case governing the **selected region**, as recorded by `field_governing_region`. They still contain the complete shaft. Their whole-file maximum can exceed the selected-region peak. The independent audit therefore reconstructs element centroids, applies the named region and compares that region's raw-cell maximum with its CSV row. All52 comparisons matched at exported precision.

## Numerical correction exposed by refinement

The102276-node A12 mesh at1.0/.02, growth.70, exposed a false “dependent distributed constraints” rejection in the earlier mixed-scale Schur rank test. The six actual constraints still controlled all six rigid modes. The solver now uses exact rigid-equilibrium reactions and rigid-motion lifting for exactly six coordinates; seven–twelve coordinates retain the previous Schur formulation. [Equations](15-feed-shaft-stress-model.md) and [verification record](verification/local-shaft-refinement.md).

The original1.0/.09 all-root mesh was rerun with the new implementation. All20 regional raw peaks and bounds agree with the published values within5.00002×10⁻¹⁰ MPa; geometry/connectivity are identical at export precision. Caching phase sine/cosine values does not change the sampled angles. The earlier four-mesh/96-phase artifacts remain preserved as evidence for their original source state.

An experimental BAMG curvature-surface route was rejected and removed, not integrated into the stress driver. It encountered excessive recursive1D metric integration; staging ordinary1D meshing produced3477 curve nodes but BAMG surface meshing failed. A topology-partition trial failed its combined partition-count/volume check. No valid stress solution or geometry claim was taken from these attempts. This is not evidence that all Gmsh anisotropic methods are unsupported. The accepted studies use the established scalar-sized tetrahedral path without altering CAD.

## Remaining qualification

Continue A12, M15 and pulley-key/slot spatial convergence and then reconcile an all-root verification strategy. Retain the original whole-shaft and every regional gate. True threads, final edge breaks, tolerances, fit prestress, KM2/M5 preload, key/collar contact, bearing/housing/module compliance, rotating fatigue and actual role-specific load paths remain unqualified.

The stated±60 N roller-force envelope is not an established physical jam-force limit:5 Nm at20 mm radius corresponds to250 N tangential force if transmitted. No change to this numerical envelope has been made or disguised. Pressure/friction/jam qualification and the full machine's thermal, electrical, safety, drawing and assembly work remain separate requirements. The **legacy reference** retains disabled outputs and `NotCommissioned`; that historical model is distinct from the unqualified RP2350 target.