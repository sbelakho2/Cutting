# 28 — F101 R6 feed-shaft regional submodel record

Date: 2026-09-16. Status: desk study; nothing releases fabrication.

## 1. Why a submodel

The torus-focused four-mesh reruns (docs/26) left 13 gmsh / 10 Netgen regional
holds. The held peaks sit at R0.15–0.20 blends (roller-shoulder R0.20 root,
roller key-floor R0.15 perimeter, M15 Ø13.5 runouts, washer-entry blends).
At the global 0.09 mm root these blends carry 2–3 elements across the radius,
and finer full-shaft roots exceed the unchanged 150000-node cap. The peak
locations are stable inside the CAD (all 40 finest peak centroids classify
inside/on the exact solid), so the holds are resolution gaps, not mesh noise.

The submodel resolves each peak region locally: the same loads (transferred as
exact boundary displacements from the global solution) on a window cut from
the exact nominal shaft B-rep, meshed with root sizes the full-shaft budget
cannot afford. The acceptance gate is unchanged: raw-peak last-change ≤ 5%
between two submodel mesh levels with a stable peak, plus the peak centroid
classified inside/on the exact CAD. No four-mesh gate value is altered or
removed; the submodel results are additive columns.

## 2. Method

1. **Global transfer.** The finest torus solutions (`feed-fem-torus/{gmsh,netgen}`
   level-4 VTUs, 132174 / 129936 nodes) are read with the fail-closed VTU field
   reader (`readLinearVtuField`). The eight shaft load bases are re-solved on
   that mesh with the unchanged distributed solver; the stored VTU field is
   reproduced to 1e-7 MPa (max displacement diff 3.4e-13 / 4.9e-13 mm), so the
   reconstruction is exact. Any load case (phase / radial signs / torque
   vertex) is then a linear combination of the eight bases.
2. **Interpolation.** `DisplacementFieldInterpolator`: element-local barycentric
   lookup (exact point-in-tet via the inverse-transpose of the vertex matrix,
   grid-indexed candidates). Points in the micrometre gap between the global
   mesh's flat facets and the true CAD surface use nearest-hull projection
   with a documented 0.05 mm tolerance; the maximum projection gap is recorded
   per run (observed 0.017–0.033 mm). Every point beyond the tolerance is an
   explicit error (the outside-support path). Observed: 0 unsupported points
   in every run.
3. **Window cut.** `cutShaftSlab`: BRepAlgoAPI_Common of the exact shaft B-rep
   with a box spanning exactly S±half (x/z padded). The two cut planes are the
   Dirichlet faces; the retained shaft surface stays traction-free. Windows
   are 11–12 mm long, centred on the four-mesh peak station, cut faces 4.3–6 mm
   from the fillet.
4. **Submodel mesh.** `meshStepRefined` (unchanged library) with the same
   growth-0.45 sizing family as the torus series, bulk 0.5 mm, on the region's
   analytic root guide (`feedShaftRootDistance` for the ring guides; a
   two-sided capsule-perimeter distance for the key floors — see §4). Node
   budget ≤ 150000 (the unchanged cap; solver parity).
5. **Dirichlet + loads.** The cosine / sine / torque-constant combinations of
   the eight bases are interpolated onto the cut nodes (metres) and solved
   with `solveSubmodelCases` (CHOLMOD supernodal, METIS, one factorisation for
   the three cases, residual < 1e-7, force/moment balance < 1e-7). Self-weight
   is re-applied through the same acceleration coefficients as the global
   basis (basis 6/7 × 9.80665 m/s²). No pulley/roller patch acts inside any
   window (all stations outside the cuts).
6. **Peak extraction.** The region membership function (unchanged) is applied
   to submodel element centroids; the 48-phase sweep and the unsampled-angle
   remainder reproduce the sampled raw peak and the angular bound exactly as
   the four-mesh driver.
7. **Convergence gate.** Last-change between the two finest submodel levels
   ≤ 5% on sampled and bound, peak centroid inside/on the CAD, and the peak
   kept inside the refined sector (documented plateau behaviour, §4).
8. **Kt cross-check.** Pilkey shoulder-fillet bending fit (verified range
   2 < h/r ≤ 20; the implemented coefficients reproduce the independent
   Shigley Table A-15-9 value Kt=1.74 at D/d=1.5, r/d=0.1 to 1.7433). Nominal
   bending is a weighted least-squares fit of σ_yy over a clean cross-section
   band, scaled by (R_band/R_net)³ to the net section. Torsion handbook fits
   are not asserted (validity 0.25 ≤ h/r ≤ 4 exceeded); band torsion shear is
   FE-derived and labelled.

## 3. Node budgets (all ≤ 150000)

| Window | Optimizer | Levels | Nodes (per level) |
|---|---|---|---|
| roller_shoulder (S86.1–98.1) | gmsh | 0.02 full / 0.02 sector / 0.01 sector | 102080 / 33392 / 87820 |
| roller_shoulder | netgen | same | 101337 / 37275 / 91231 |
| m15_relief (S66.7–78.7) | gmsh | 0.02 sector / 0.01 sector | 61286 / 140345 |
| m15_relief | netgen | same | 60645 / 139132 |
| pulley key ×2 / roller key | gmsh/netgen | 0.02 sector / 0.0175 sector | see §4 |

## 4. Sizing deviations from the torus series (documented, gate unchanged)

- **Sector restriction.** The full-ring root at 0.02 mm and below exceeds the
  budget on the R0.2 blends (the blend surface alone needs ≈ 700k surface
  nodes at 0.01 mm). Root refinement is confined to the peak azimuth
  ±35–60°; the finest peak must stay inside the sector. Where the full-ring
  0.02 level fits (roller shoulder), it cross-checks the sector: 46.14 vs
  45.35 MPa (gmsh working, 1.7%) and 106.03 vs 105.46 MPa (netgen fault,
  0.5%) — the sector restriction is benign.
- **Two-sided capsule guide.** The published one-sided guide
  (max(0, capsule−fillet)) puts the root size on the whole blend material
  volume (2.3 mm³), which alone meshes to ≈ 300k nodes at 0.02 mm. The
  submodel uses |capsule−fillet| (the root size on the blend-root surface,
  graded inside) — the same zero set as the ring guides use. Peak (surface)
  resolution is unchanged.
- **Finest key-floor root 0.0175 mm.** 0.01 mm on the full perimeter blend
  (≈ 90 mm² tube surface) is budget-infeasible at 150k nodes; the finest key
  level uses 0.0175 mm (measured 129–140k nodes). The roller-shoulder and
  M15 windows do reach 0.01 mm.
- **M15 refine band.** The rear M15 runout torus keeps a 0.1 mm background
  root (the region's governing peak sits on the front runout at all four
  mesh levels; the guard verifies the submodel peak stays in the refined
  band).

## 5. Per-region results

Four-mesh (global 1/0.09) → submodel last-change (the unchanged 5% gate).
"dS/dr/daz" = peak-centroid movement between the two submodel levels; the
azimuthal plateau behaviour is documented below.

| Region | Case | Global sampled/bound MPa | Four-mesh change % (sampled/bound) | Submodel levels | Submodel sampled MPa | Submodel bound MPa | Submodel change % (sampled/bound) | Peak dS/dr/daz | Verdict |
|---|---|---|---|---|---|---|---|---|---|
| roller_shoulder (gmsh) | working 135°, s2, t2 | 39.309 / 41.634 | 10.88 / 11.01 | 0.02 full → 0.01 sector | 46.137 → 45.164 | 48.554 → 47.851 | 0.41 / 0.18 | 0.001 mm / 0.003 mm / 46° plateau | **converged** |
| roller_shoulder (netgen) | fault 45°, s0, t2 | 98.926 / 100.795 | 8.61 / 8.30 | 0.02 full → 0.01 sector | 106.034 → 104.738 | 108.140 → 106.929 | 0.69 / 0.58 | ≈0 / ≈0 / plateau | **converged** |
| M15_relief (gmsh) | working 142.5°, s2, t0 | 59.910 / 63.151 | 6.22 / 6.37 | 0.02 → 0.01 sector | 60.517 → 59.291 | 63.718 → 62.470 | 2.07 / 2.00 | 0.029 / 0.008 / 0.08° | **converged** (slot/groove corner peak, §6) |
| M15_relief (netgen) | working 37.5°, s1, t0 | 59.425 / 62.621 | 7.48 / 7.46 | 0.02 → 0.01 sector | 60.463 → 60.415 | 63.643 → 63.479 | 0.08 / 0.26 | 0.094 / 0.016 / 0.52° | **converged** |
| M15_relief (netgen) | fault 142.5°, s2, t0 | 105.967 / 108.962 | 6.58 / 6.54 | 0.02 → 0.01 sector | 110.528 → 115.130 | 113.418 → 118.167 | 4.00 / 4.02 | 0.006 / 0.002 / 0.08° | **converged** |
| roller_key_root (netgen) | working 315°, s0, t2 | 43.146 / 44.278 | 6.92 / 6.85 | 0.02 → 0.0175 sector | 42.007 → 42.955 | 43.146 → 44.097 | 2.21 / 2.16 | 1.861 mm (S plateau) / 0.010 / 0.30° | **converged** |
| roller_key_root (netgen) | fault 315°, s0, t2 | 153.574 / 154.706 | 8.54 / 8.51 | 0.02 → 0.0175 sector | 145.744 → 147.877 | 146.882 → 149.019 | 1.44 / 1.43 | 1.861 mm (S plateau) / 0.010 / 0.30° | **converged** |
| pulley_key_and_MB2_slot (gmsh) | working 255°, s1, t3 | 38.721 / 41.058 | 5.33 / 5.22 | 0.02 → 0.0175 sector | 37.090 → 37.082 | 39.364 → 39.354 | 0.02 / 0.03 | 0.111 / 0.087 / 0.38° | **converged** (slot-lip peak, see caveat) |
| pulley_key_and_MB2_slot (gmsh) | fault 210°, s5, t0 | 75.131 / 76.749 | 9.64 / 9.62 | 0.02 → 0.0175 sector | 72.242 → 74.447 | 73.765 → 75.999 | 2.96 / 2.94 | 1.657 mm (S plateau) / 0.001 / 0.05° | **converged** |
| roller_key_root (gmsh) | working 315°, s1, t2 | 42.626 / 43.757 | 5.65 / 5.70 | 0.02 → 0.0175 sector | 42.217 → 41.783 | 43.325 → 42.890 | 1.03 / 1.00 | 0.258 mm (S plateau) / 0.004 / 0° | **converged** |
| roller_key_root (gmsh) | fault 315°, s0, t2 | 150.191 / 151.309 | 6.58 / 6.60 | 0.02 → 0.0175 sector | 145.539 → 143.891 | 146.648 → 144.998 | 1.13 / 1.13 | 0.258 mm (S plateau) / 0.004 / 0° | **converged** |

**Azimuthal plateau.** The axisymmetric blend peaks are degenerate mirror
pairs (vm(−σ)=vm(σ)) with a shallow plateau: on the roller shoulder the
sampled peak wanders up to ≈46° in azimuth between levels while the value
changes ≤0.7% and S/radius stay stable to <0.005 mm (azimuth-profile.csv).
The gate's "stable location" is therefore read as: same feature (S and
radius stable, CAD state inside), with the plateau behaviour documented.

**Key-floor S plateau.** The roller key-floor side blend is straight for
18.1 mm; its peak S wanders 1.86 mm along the blend between 0.02 and
0.0175 mm while the value changes 2.2%/1.4% and x/z stay stable to
0.01 mm — the four-mesh series showed the same wander (S99.45–100.05).

**Pulley slot-lip competition.** For the gmsh working pulley case the region
peak sits on the washer-slot lip edge (S59.2, az 21.9°, submodel 37.09 MPa)
competing with the key-floor front blend (four-mesh peak 38.72 MPa at S61.3;
the global field's window-local lip peak is 36.25 MPa). Both features are
refined together in the final runs (the published slot-lip guide terms are
restored in the submodel sizing). The two peaks differ by ≈4%, which is the
inherited global discretization band of the transferred boundary data: the
submodel re-solves the global solution's continuation, not the continuum, so
its regional peak carries the global finest mesh's own error floor. The
submodel's internal last-change (0.02%) closes the resolution gate with this
transfer-error caveat documented.

## 6. Kt cross-check numbers

Reference: W.D. Pilkey, *Peterson's Stress Concentration Factors*, 3rd ed.,
Wiley 2008, Table 3.3 (stepped round bar, shoulder fillet, bending),
Kt = C1 + C2(2h/D) + C3(2h/D)² + C4(2h/D)³, implemented for the published
2 < h/r ≤ 20 range only; the coefficients reproduce the independent Shigley
Table A-15-9 value Kt=1.74 at D/d=1.5, r/d=0.1 to 1.7433. Torsion fits
(Table 4.1, 0.25 ≤ h/r ≤ 4) are NOT asserted here (h/r exceeded); band
torsion shear is FE-derived and labelled.

| Region | Geometry (D/d/r) | h/r | Handbook bending Kt | Nominal bending σ (net section) | Kt-based peak | Submodel peak | Measured Kt | Comment |
|---|---|---|---|---|---|---|---|---|
| roller_shoulder | 12/10/0.2 mm | 5 | 2.586 | 12.37 MPa (band S93.5, r4.94; τ 5.75 MPa) | 32.0 MPa | 45.16 MPa (gmsh working) | 3.65 | bending+torsion estimate √((Kt·σ)²+3τ²)=33.5 MPa; peak combines both |
| roller_shoulder | 12/10/0.2 mm | 5 | 2.586 | 12.54 MPa (τ 26.66 MPa) | 32.4 MPa | 104.74 MPa (netgen fault) | 8.35 (bending ref) | torque-dominated case; band vm 29.93 MPa, submodel/band-vm 3.50; torsional concentration ≈2.3 of the shear |
| M15_relief | 15/13.5/0.2 mm | 3.75 | 2.534 | 11.62 MPa (band S74.5) | 29.4–29.6 MPa | 59.29–115.13 MPa | 5.10–9.87 | **idealisation inapplicable**: the peak is at the sharp washer-slot-wall ∩ groove-surface intersection edge (bounded 3D corner, converged); mid-groove surface σ_yy = 40.2 MPa → runout factor ≈1.6 |
| roller_key_root / pulley key | milled key-floor perimeter blend | — | none citable | — | — | 42.96 / 147.88 (netgen) | — | no closed-form Kt exists for the milled floor perimeter; convergence is the evidence |

The M15 peak is NOT a shoulder-fillet peak: the washer slot's side wall
(x = ±2.1) crosses the Ø13.5 groove surface at z = 6.415 mm, S71.5–73, and
the peak hugs that intersection edge (azimuth profile: a 59.3 MPa spike at
337.5° against 40.1 MPa one bin away). It is a real, bounded, converged
corner concentration of the machined geometry — not mesh noise — but the
Pilkey shoulder-fillet factor does not bound it, and no handbook factor for
this compound corner is asserted. The submodel therefore closes the
resolution gate for M15 with the corner documented, not with a Kt claim.

## 7. Remaining holds

- `no_exterior_same_surface_centroids` (gmsh, 34 cells): a whole-shaft
  geometry-audit gate; a regional submodel cannot re-audit the full-shaft
  mesh. Stays held.
- Any regional row that does not meet the 5% gate in §5 stays held with its
  four-mesh values intact (none of the four-mesh rows are altered anywhere).
- The azimuthal and key-floor S plateaus (§5) are documented behaviour of
  the axisymmetric/straight blends, not gate relaxations; the reported
  submodel values are the region raw peaks on the refined meshes.

## 8. Files

- New: `engineering/include/feed_submodel.hpp`, `engineering/src/feed_submodel.cpp`
  (VTU field reader, displacement interpolator, slab cut, submodel solver,
  Pilkey Kt, capsule distance, nominal band stress),
  `engineering/src/feed_submodel_analysis.cpp` (driver),
  `engineering/src/feed_submodel_profile.cpp` (azimuth profile tool),
  `engineering/tests/test_feed_submodel.cpp`; additive CTest entries
  `feed_submodel_benchmarks` and `feed_submodel_reject_invalid_requests`.
- Generated: `engineering/generated/feed-submodel/{gmsh,netgen}/…` per window
  (submodel.step, results.csv, report.md, azimuth-profile.csv,
  working/fault-submodel.vtu, status.json).
- Updated: the two torus `criteria.csv`/`report.md` files (additive submodel
  rows/sections only), `docs/24` §9 feed row, `engineering/README.md`
  feed-shaft paragraph, this record.
