# F101 R6 — CAD-feature refinement and peak audit

2026-09-12. **One physical design remains head R6 / feed R6 / measurement R1. No fabrication or energisation release.**

**Historical49-test increment.** The subsequent [A12 boundary-topology investigation](18-a12-boundary-topology.md) reproduces the exterior cell, introduces explicit Netgen optimization and reports new local/background results. The numerical tables and original meshes below retain their own provenance; they are not relabeled as optimized or converged.

This increment follows the [26-mesh focused study](16-local-shaft-refinement.md). It relates unresolved stress peaks to the actual nominal CAD faces before reallocating mesh density. No purchased part, machining geometry, material, load, support, strength criterion or node budget changed.

**Results:** M15 passes the local last-refinement, finer-background and angular comparisons below. The pulley key-floor series passes its local last-refinement checks; its background comparison is at the penultimate resolution, not the finest. The broader pulley/slot working peak remains unconverged. Both A12 roots remain unresolved; the front-root investigation identifies a specific curved-boundary mesh discrepancy. None of these separate studies establishes full-shaft convergence.

## Actual CAD, not inferred root dimensions

The [C++ peak audit](../engineering/src/feed_fem_geometry_audit.cpp) enumerates the unchanged shaft's85 faces and projects each reported peak onto every **trimmed** face using OpenCASCADE. It reports the nearest face, nearest torus, distances and solid membership. The [face table](../engineering/generated/feed-fem-cad-features/peak-CAD-audit/faces.csv) and [640 peak-location rows](../engineering/generated/feed-fem-cad-features/peak-CAD-audit/peak-geometry.csv) cover the three historical unresolved studies and all five new primary series. Face numbers identify this nominal B-rep enumeration, not permanent manufacturer IDs.

- A12 roots: major radius5.765 mm, minor radius0.04 mm; torus centres at S18.54 and S19.58. Actual trimmed fillets occupy S18.50–18.54 and S19.58–19.62.
- M15 peaks: near the small washer-slot exit surfaces, not generally on the circumferential relief torus. The earlier finest peak is0.00480505 mm from its nearest trimmed cylinder face, versus0.0335891 mm from the nearest torus.
- Pulley peaks: working peaks migrate to the far key-floor end blend; fault peaks occur along a floor-side blend. The floor radii are0.15 mm. The original broad slot guide allocated fine elements outside these actual blend surfaces as well.

`analytic_axis_defined` in the face table is true only for cylinders and tori. Other rows' zero placeholders are not physical axis/radius measurements. The CAD remains a nominal custom-part definition, including the stated cosmetic thread envelopes; this audit does not authenticate unavailable purchased-part internals.

## CAD-based sizing method

The [feature API](../engineering/include/feed_refinement.hpp) and [implementation](../engineering/src/feed_refinement.cpp) add the optional `--focus-feature` argument to the existing driver. It requires the matching `--focus-region` and records the selected face geometry in each study's sizing table.

| Feature identifier | Region reported and retained in fields | Selected faces |
|---|---|---:|
| `A12_S18_5` | `A12_groove` | 1 torus |
| `A12_S19_62` | `A12_groove` | 1 torus |
| `M15_slot_exit` | `M15_relief` | 7 small pocket blends, clipped axially to S71.4–73.1 |
| `pulley_slot_blends` | `pulley_key_and_MB2_slot` | 28 upper-pocket blends |
| `pulley_key_floor` | `pulley_key_and_MB2_slot` | 4 floor blends: both sides and both ends |

Selection uses actual surface types, radii and bounds, not transient face numbers or a sphere placed at one favorable stress peak. For an analytic face, the sizing distance is the maximum of padded face-bounding-box distance and untrimmed analytic-surface distance reduced by a0.000001 mm numerical allowance (0.001 µm), not a manufacturing tolerance. BSpline junctions use padded bounds only. The allowance is $\epsilon=10^{-6}\,\mathrm{mm}$:

$$d_f(p)=\min_i\max\left(d_{B_i}(p),\max(0,d_{S_i}(p)-\epsilon)\right).$$

For an untrimmed cylinder, $d_S=|r-R|$; for a torus, $d_S=|\sqrt{(r-R)^2+a^2}-\rho|$, with axial coordinate $a$ and distance $r$ from the analytic axis. Each surface-distance and bounding-box-distance term is a lower bound to its trimmed face or explicitly clipped face portion. Their maximum remains a lower bound; the minimum targets the selected set. This is a conservative **sizing guide**, not a spatial stress-error estimator or continuous geometry-conformity certificate.

The full-shaft sizing rule remains

$$h(p)=\min\left(h_b,\ h_{bg}+0.45d_{all}(p),\ h_f+\gamma d_f(p)\right).$$

The default all-root and earlier named-region modes are unchanged. Every solve retains the complete STEP solid, six distributed supports, eight load bases, both load classes, all sign/torque vertices, gravity and all ten stress regions. Separate A12-root allocation does **not** delete the opposite root from the mesh or its stress region. Every focused report retains the mandatory background/all-root scope hold. The150000-node cap,5% raw/bound stress gates,3% displacement gate and original material limits remain unchanged.

## Published studies

Nine completed studies contain **26 meshes,208 basis rows,520 regional rows and52 governing-case VTUs**. Primary series use48 phases and background0.20 mm. Sizes below are bulk/root in mm, in execution order. The report links include each series' complete criteria and field provenance.

| Primary series | Bulk/root sequence | Growth | Finest nodes / tetrahedra | Total holds |
|---|---|---:|---:|---:|
| [A12 rear root](../engineering/generated/feed-fem-cad-features/A12-rear/report.md) | 1.6/.032; 1.2/.020; 1/.0125; .85/.009375 | .70 | 136756 / 568566 | 7 |
| [A12 front root](../engineering/generated/feed-fem-cad-features/A12-front/report.md) | 1.6/.032; 1.2/.020; 1/.0125; .85/.009375 | .70 | 136805 / 568908 | 7 |
| [M15 slot exit](../engineering/generated/feed-fem-cad-features/M15-slot/report.md) | 1.6/.128; 1.2/.08; 1/.05; .85/.03125; .75/.01953125 | .55 | 78131 / 349080 | 5 |
| [Pulley/slot blends](../engineering/generated/feed-fem-cad-features/pulley-blends/report.md) | 1.6/.128; 1.2/.08; 1/.05; .85/.03125 | .65 | 137201 / 572303 | 13 |
| [Pulley key floor](../engineering/generated/feed-fem-cad-features/pulley-floor/report.md) | 1.6/.128; 1.2/.08; 1/.05; .85/.03125; .75/.0205 | .85 | 137503 / 559773 | 3 |

Other studies: [M15 finer background](../engineering/generated/feed-fem-cad-features/M15-background/report.md), [M15 96-phase audit](../engineering/generated/feed-fem-cad-features/M15-phase/report.md), [pulley penultimate-level background](../engineering/generated/feed-fem-cad-features/pulley-background-intermediate/report.md), and [all-root compatibility](../engineering/generated/feed-fem-cad-features/all-root-compatibility/report.md). Single focused audits retain two holds; compatibility retains the three-mesh hold.

### Finest selected-region stresses

MPa. The phase bound encloses unsampled angles for the discrete linear model only. Last change uses the finer value as denominator. These are selected-region values, not necessarily the whole-shaft maximum printed at the end of a run.

| Region / load | Raw peak | Phase bound | Raw last change | Bound last change |
|---|---:|---:|---:|---:|
| A12 rear working | 17.5373870 | 17.5564036 | **6.790487%** | **6.783559%** |
| A12 rear fault | 86.5902033 | 86.6092198 | **6.895461%** | **6.894034%** |
| A12 front working | 22.4377916 | 22.4793716 | **26.614209%** | **26.616155%** |
| A12 front fault | 109.684574 | 109.726154 | **26.569623%** | **26.570038%** |
| M15 working | 61.8033187 | 65.1062032 | 0.085370% | 0.026091% |
| M15 fault | 112.125400 | 115.205328 | 0.272060% | 0.273850% |
| Pulley/slot blends working | 44.0044795 | 46.7180733 | **7.584420%** | **7.612756%** |
| Pulley/slot blends fault | 77.1524654 | 78.8015788 | 3.360667% | 2.942558% |
| Pulley key floor working | 44.8813343 | 47.5963919 | 1.808883% | 1.955660% |
| Pulley key floor fault | 77.7517304 | 79.3713883 | 1.117235% | 1.154404% |

The20 entries in this new selected-feature/load/raw-bound matrix include ten passes and ten unmet checks. This is **not** a subtraction from the historical16 all-root holds or the prior focused matrix. Broader pulley/slot behavior is not qualified by choosing the floor-only series' more favorable last change. All nine study statuses remain numerically held and unreleased.

### Background and phase comparisons

- **M15:** same finest .75/.01953125, growth.55, with background.20→.13:110146 nodes/497589 tetrahedra. Working raw/bound changes0.247836%/0.342256%; fault0.616774%/0.661357%. All below5%. This is a specified background sensitivity comparison, not a bound for all possible meshes or actual contact conditions.
- **Pulley key floor:** same **penultimate** .85/.03125, growth.85, background.20→.13:118526 nodes/507543 tetrahedra. Working raw/bound changes2.448667%/2.755892%; fault1.111671%/1.125530%. This does **not** establish background independence at the finest .75/.0205 level.
- **M15 angular audit:** same78131-node mesh,48→96 phases. All20 regional96-phase raw peaks remain inside their corresponding48-phase bounds; minimum reserve0.0202231474 MPa. Geometry/connectivity hashes match. Angular enclosure does not repair spatial error.
- **All-root compatibility:** original1/.09 mesh remains131909 nodes/603290 tetrahedra. Maximum difference over all20 raw/bound rows is5.00002×10⁻¹⁰ MPa; exported geometry/connectivity are identical. The original four-mesh study is preserved, not reclassified as converged.

## A12 front-root curved-boundary discrepancy

The finest front-root peak is at $(X,S,Z)=(3.99655581152,19.6119262441,4.12200070933)$ mm. The independent CAD classifier places this centroid **outside the exact nominal solid**,0.000281177056 mm from the nearest trimmed torus. This is an identified discrepancy of the linear faceted volume, not a physical measurement or a newly changed shaft surface.

The governing cell in the [working field](../engineering/generated/feed-fem-cad-features/A12-front/working-4.vtu) is zero-based element489924. Its four zero-based nodes are:

| Node | X mm | S mm | Z mm |
|---:|---:|---:|---:|
| 61069 | 3.99299694256 | 19.6132718947 | 4.12743149891 |
| 48757 | 4.00256898574 | 19.6156406177 | 4.12378756916 |
| 52363 | 3.99096699514 | 19.6082212623 | 4.12084560232 |
| 66573 | 3.99969032266 | 19.6105712017 | 4.11593816692 |

All four exported nodes lie on the same analytic torus within3.01×10⁻¹¹ mm when evaluated from the rounded exported coordinates. Connectivity inspection finds two boundary faces with one incident tetrahedron and two interior faces with two incident tetrahedra. Thus accurate surface nodes and positive tetrahedron quality alone do not establish that the intervening flat-faced volume stays inside the curved solid.

The22.4378/109.6846 MPa raw peaks are retained in every table and field. They are **not** removed as outliers, replaced by percentiles, or certified as point stresses in material at that exact nominal CAD location. This finding does not by itself prove the complete cause of the26.6% change or a stress singularity. A12 needs a boundary-faithful discretization/convergence investigation, not another nominally passing surface-node audit.

## Resource-limit decisions

- A12 rear .85/.0078125, growth.70/background.20 produced170423 nodes/694122 tetrahedra and was rejected **before solving**. The accepted .009375 target is a25% reduction from.0125, with38% more nodes, not a nearly identical mesh.
- Pulley floor .75/.01953125, growth.65/background.20 produced151003 nodes/633003 tetrahedra and was likewise rejected. A .0205 target with that growth completed at145912 nodes/610958 tetrahedra. The published floor series uses a separately declared, fixed.85 growth throughout all five levels; it is not spliced into the.65 pilot. Its final target reduction remains34.4%.
- The.65 pilot's accepted finest floor results were44.7632500/47.5316501 MPa working and77.1108497/78.7462688 MPa fault. They are research comparisons, not substituted publication rows or proof of finest-background convergence.
- No node cap, strength limit, raw-peak criterion or dependent-constraint threshold was increased. No rejected mesh became a claimed passed simulation.

## Verification and remaining work

The complete source passed **49/49 native tests in440.53 s** and **49/49 ASan/UBSan tests in707.15 s**, with no sanitizer diagnostics. [Verification, regression coverage and current source fingerprints](verification/cad-feature-shaft-refinement.md).

All52 new VTUs passed XML parsing, six-array numeric/length checks, finite-value checks, integer connectivity, offsets, tetrahedron types, nonnegative stress and independently reconstructed selected-region maxima. Maximum difference from the CSV raw peaks is zero at exported precision. No rendered contour review is claimed.

Across the26 meshes: maximum137503 nodes; minimum signed inverse-condition quality0.0196478551819; maximum surface-node projection6.09405450759×10⁻⁷ mm; maximum facet-centroid projection0.147635738974 mm; maximum CAD/mesh volume difference0.440743%. Maximum equilibrium/constraint residuals1.37486179932×10⁻¹⁰/3.28191319078×10⁻¹⁶. These are scoped numerical checks, not a stress-error certificate; the A12 discrepancy above demonstrates why.

Finish A12 boundary discretization, wider pulley/slot and all-root reconciliation. True helical threads, final edge breaks, tolerances, interference/preload/contact, rotating fatigue and actual roller-force/role envelopes remain separate work. The specified±60 N roller box is not an established jam-force limit;5 Nm at20 mm radius can transmit250 N tangentially. The complete machine mechanisms, thermal/electrical/safety integration, sourcing, drawing/assembly package and physical qualification remain unfinished. The **legacy reference** retains disabled outputs and `NotCommissioned`; that historical model is distinct from the unqualified RP2350 target.