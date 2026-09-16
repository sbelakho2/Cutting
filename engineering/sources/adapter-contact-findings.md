# H003 adapter F0 seat: resolved-pressure elastic-foundation model (contact-gate rerun)

2026-09-16. Record of the R6 adapter FEM raw-peak gate closure attempt. Generated
report: [generated/adapter-r6-analysis/report.md](../generated/adapter-r6-analysis/report.md).

## Model class implemented

The previous model applied the F0 preload as a **uniform pressure patch** on each
counterbore-floor washer seat r∈[2.75,4.5]mm. The patch's r=4.5mm traction
discontinuity is an ideal-load edge singularity: the raw peak sat at the
washer-OD edge and grew under peak-region refinement (182.11→219.49MPa across the
1.0mm-background→0.7mm pair), so the 5% raw-peak gate could not close.

The rerun replaces the uniform patch with a **resolved-pressure
elastic-foundation model** (linear, no contact/preload-loss/friction solve):

- **Resolved seat pressure** (applied on each counterbore-floor seat,
  r∈[2.75,4.5]mm): the quartic bump
  `p(r) = p_peak·(1−s²)²`, `s = (r−3.625)/0.875`,
  continuous with continuous slope and **zero at both annulus edges** — the
  resolved thin-washer class: a thin ISO7092 M5 washer (Ø9×1mm, E=205GPa) over
  the counterbore floor flexes and transmits pressure that decays to zero at
  its edges. Peak `p_peak = p0·15/8 = 376.327MPa` at r=3.625mm; the bump's
  annulus integral is exactly 8/15 of the uniform value (the odd r-term
  integrates to zero), so the mean `p0 = F0/A_seat = 200.708MPa` and the
  **per-seat resultant stay exactly 8.0kN** (60% of the 13.35kN ISO898
  class10.9 M5 proof load — a required-preload screen, not a torque
  prediction).
- **Resolved foundation stiffness** (stage-side seat annuli, same profile —
  action–reaction through the washer): `kn(r) = kn_peak·(1−s²)²`,
  `kn_peak = 94,759.7N/mm³` per unit area at mid-radius,
  `kt = kn/2.67 = 35,490.5N/mm³` (documented steel-interface shear-to-normal
  rule). The per-seat total normal stiffness is preserved at the elastic
  half-space calibration of one rigid annular punch
  `2·E·b/(1−ν²) = 2014.41MN/m` (E=205GPa, ν=0.29, b=4.5mm) spread over the
  39.86mm² annulus.

Implementation: optional `weight` profiles were added to `LoadPatch` and
`FoundationPatch` (`engineering/include/fem.hpp`, `engineering/src/fem.cpp`);
the weight multiplies the face area tributary before normalisation, so the
traction/stiffness vary smoothly over the patch while the load resultant,
equilibrium checks, artifact schemas, the 150,000-node direct-solver cap and
all gates are unchanged. Both torque signs, unchanged ±8.5Nm/150N resultants
on the actual 85.85mm motor-flange bearing footprint, unchanged geometry and
loads.

## Results (full r6 rerun)

Six uniform meshes 6/4/2.8/2/1.4/1.0mm plus the isolated local-refinement pair
(1.0mm background → 0.7mm within 8mm of the raw peak; finest level 110,580
nodes) and the foundation-stiffness sensitivity ×0.5/×2 on the finest mesh.

- The ideal pressure-patch edge singularity is **gone**: the pair peak
  `285.987MPa` sits on the counterbore floor at r=3.91mm from the F0 seat axis
  (0.59mm *inside* the washer-OD edge), no longer at a patch edge; the
  stiffness sensitivity moves the peak only +0.17% (285.54→286.02MPa), so the
  result is not a spring-calibration uncertainty.
- The gate pair still measures **18.4573% raw-peak change** (5% criterion →
  **REFINE**) and **4.9551% displacement change** (3% criterion → **REFINE**).
  Cause identified quantitatively: the resolved seat features (the surface
  compression layer under the profile peak, ≈0.83·p_peak, and the floor-shelf
  deflection at the Ø5.5 hole edge) live on the 1.75mm-wide seat annulus, the
  same order as the pair's 1.0mm background element. The pair now records
  feature-scale discretisation of a smooth field, not convergence of a
  singularity; the four symmetric seats show ±10–20% peak scatter at 1.0mm
  because the profile is sampled by single elements.
- **Seat-scale focused pair** (diagnostic; does not move the gate pair): the
  same resolved-pressure global model refined to 0.15mm within 3mm of the peak
  seat (background) and 0.1mm within 2mm (focused, 0.15mm shell), 1.5mm bulk,
  53,068→64,944 nodes: raw peak **321.67→312.71MPa, last-change 2.86% ≤5%**.
  The resolved model's peak converges at the feature's own scale and is
  bounded at ≈313MPa — a real preload seat compression (surface von Mises
  ≈0.83×p_peak), not a mesh singularity.
- Peak stress screen: 285.987MPa (pair level) vs yield/3=216.667MPa →
  **FAIL**, reported without waiving: local preload seat compression below the
  650MPa certified yield; the earlier uniform-patch 219.489MPa understated the
  seat compression because the patch edge dominated before the resolved model.

## Verdict

The raw-peak gate rows remain **REFINE** (never relaxed). The numerical
singularity is closed: with the resolved pressure and resolved foundation
profiles the peak converges (2.86% at 0.15→0.1mm seat scale), and the gate
pair's 18.5% is fully explained by the seat feature scale versus the pair's
1.0mm background element. What remains is physical, not numerical:

1. the resolved profile shape and the rigid-punch foundation stiffness are
   calibrated models, not measurements — a measured washer/joint contact
   stiffness (or a two-body ISO7092 washer contact solve) is the remaining
   justified step before the adapter row may be relied on for release;
2. the seat compression ≈313MPa is below the certified yield but above the
   blanket yield/3 screen; a seat-local acceptance (clamp compression inside
   the Lamé bound 640.7MPa, below 650MPa yield) is an open engineering
   decision, not a mesh question.

## Files changed

- `engineering/include/fem.hpp`, `engineering/src/fem.cpp` — resolved-profile
  weights on `LoadPatch`/`FoundationPatch` (validated, uniform default).
- `engineering/src/analysis.cpp` — resolved quartic seat profile on the four
  F0 preload patches and the stage-side foundation (including the stiffness
  sensitivity sweep), seat-scale focused pair, updated report prose.
- `engineering/generated/adapter-r6-analysis/*` — regenerated (report.md,
  checks.csv, status.json, fem/*; new `fem/seat-scale-refinement.csv` and two
  seat-scale VTUs; criteria.csv schema unchanged).
- `docs/24-production-and-release-plan.md` §9 adapter row and
  `engineering/README.md` adapter paragraph updated.
