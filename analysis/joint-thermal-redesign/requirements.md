# Adapter joint and shaft thermal redesign requirements (review F07)

**Status:** requirement specification with recomputed demands. The native
redesign + converged re-solves + correlation are the open work; this document
fixes WHAT must be achieved and forbids the known wrong answers.

## 1. Motor-adapter joint (R6 fails; R7 direction fixed)

**Retained failing evidence** (`engineering/generated/adapter-r6-analysis/fem/criteria.csv`):
peak 285.987 MPa vs 216.667 MPa screen (650 MPa material / factor 3); peak
refinement 18.4573 % vs 5 % gate; displacement refinement 4.95506 % vs 3 %
gate. The focused ~313 MPa study also fails. These are model results, not
measurements; they do not improve by changing limits.

**Redesign requirements (the R7 work order):**

1. **Load demand (recomputed, analysis/axis-rating-requirements):** the joint
   transmits the stall thrust selection basis **7539.8 N** (8.5 N.m x1.2
   tolerance / eta 0.85 at 10 mm lead) plus the 5000 N proof case and the
   2500 N / 750 N / 375 N working signs. No candidate may be screened at the
   eta=1 figure alone.
2. **Contact/washer reconstruction:** the R6 screw-head interference with the
   motor pilot (F02/R6 counterbore fix) means the washer/contact path is the
   hot spot. The redesign must include: real washer geometry (HM-W class
   5.3x9x1 reduced-OD), receiver flexibility (never a rigid support), bolt
   preload with scatter (ISO 4762 M5x25 cl.10.9 torque-preload band), and
   friction coefficient bounds at the faying surfaces.
3. **Convergence acceptance (unchanged):** last-pair peak <= 5 %, displacement
   <= 3 %, peak <= 216.667 MPa, with >= 3 systematic refinements and retained
   raw fields.
4. **Correlation/proof:** the P05 proof cases on the physical assembly with
   strain or deflection measurement; the model is not released without it.

**Forbidden shortcuts:** borrowing the HIWIN 6320 N / old guide ratings for the
custom assembly; keeping the 285.987 MPa model and "documenting" it as
acceptable; relaxing the 216.667 MPa screen; ignoring washer/pilot contact.

## 2. Shaft/bearing thermal path (29.6603 K vs 20 K requirement)

**Retained failing evidence** (`engineering/generated/bearing-contact-thermal/status.json`):
29.6603 K shaft rise above housing, `thermal_field_verified:false`,
`physical_validation_complete:false`, `fabrication_released:false`. The stored
30 W motor source case is conditional; it may never be reduced to pass.

**Redesign requirements:**

1. **Heat-path inventory (M22):** motor ohmic/iron losses -> coupling -> shaft;
   bearing friction heat (preload + preload temperature dependency) -> shaft and
   housing; conduction through both bearing supports; convection from housing
   and shaft; radiation at 40 C room. Every path needs identified parameters
   (interface conductances, convection coefficients with justification).
2. **Design levers (each to be modelled, not asserted):** bearing preload
   selection/reduction; housing finning or contact area increase; thermal
   interface improvement at the supports; shaft material/conductivity; duty
   scheduling. Any lever that reduces accuracy/life (e.g. preload) needs its
   own G16/G17 re-check - thermal and fit evidence are coupled.
3. **Coupled fits (G16/G18):** the 3-6 um interference and remaining bearing
   clearance must be re-evaluated at the hot steady state; the cold-fit-only
   conclusion is not a release.
4. **Convergence/uncertainty:** as item 1.3 (5 %/3 % class gates + bounds).
5. **Correlation/proof:** P10 instrumentation on the physical axis (shaft vs
   housing thermocouples at the declared duty); the model is accepted only
   against measured rise.

## 3. What closes F07

- A redesigned adapter model passing the unchanged screen with converged
  refinement and proof correlation.
- A complete coupled thermal model within 20 K rise at the hot duty with
  measured correlation and hot fit/clearance validation.
- Both in native files under `engineering/` with regenerated reports; the
  current conditional failures stay visible until replaced by correlated
  results.
