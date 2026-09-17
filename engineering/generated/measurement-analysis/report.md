# Measuring-wheel interfaces and calibration model

Revision **S1-B-MEAS-R2-2026-09-16**.

## Source corrections

Measuring wheel **E60137** (ifm E60137 "U:200 N:06", substituted 2026-09-16 for the unpriced Kübler 8.0000.3123.0006): **200 mm** circumference, **Ø63.66 mm**, Ø6 bore, aluminium wheel with PU surface **90 Shore A**, 64.5 g. ifm publishes no circumference/OD tolerance and no hub/clamping-screw dimensions; the 0.1% (0.2 mm) tolerance and the retained 9.5 mm tread / 14.5 mm overall / Ø16.5 mm hub / Ø9 mm end-land profile stations are assumed placeholders pending the manufacturer drawing at incoming inspection. The four M4×6 radial clamp openings are a retained envelope, not a published ifm screw callout.

Encoder **RU3100** (ifm RU3100, substituted 2026-09-16 for the unpriced Autonics E40S6-1024-6-L-5): body **Ø58.6×63.5 mm** behind the mounting face, solid **Ø6×10 mm** shaft with no flat, tip **10 mm from the mounting face**. Synchro-flange fixing with **3×M4×0.7** tapped 6 mm deep on **PCD42**; the archived drawing front view labels only Ø42 and Ø58.6, so no pilot is modeled and the plate's Ø36.5-class seat bore follows the ifm mounting practice (confirm the centering feature at receipt). 4.75–30 VDC, HTL/TTL push-pull, factory 1024 PPR parameterised to 2048 PPR via IO-Link (programmable 1…10000), 1000 kHz switching frequency, 12000 rpm, starting torque ≤0.01 Nm, M12 5-pole connector.

## Uncalibrated length and bearing gates

The factory circumference alone gives **1.0 mm error per commanded metre** (assumed ±0.2 mm on 200 mm; ifm publishes no tolerance). Adding the previous0.2 runout,0.05 quantisation,0.15 tension and0.25 slip allocations gives **1.65 mm**, not the earlier1.15 mm. Those four terms are allocations, not measured errors; this is a failing uncalibrated budget, not a worst-case physical guarantee. The separate factory diameter tolerance is not treated as an additional independent scale error. The 2048 PPR IO-Link setting gives 0.024414 mm/count (0.0122 mm half-count); the quantisation allocation reduces from0.05 to0.025 mm and still covers the half-count error.

SKF626-2Z CN minimum clearance2 µm leaves only **0.5 µm** with10 K inner-to-outer ring difference, even before a press fit. The chosen CN bearing is not silently accepted with full js5 interference. Its actual matching/thermal design remains open.

At500 mm/s and199.8 mm circumference: 150.1501502 rpm, 5125.125125 A/B cycles/s and 20500.5005 quadrature edges/s. The1000 kHz encoder limit is a pulse-frequency specification; MCU edge capture and electrical receivers are separate work.

## Reversible phase calibration

The mapping is C times signed whole turns plus a strictly increasing one-turn distance table. Negative counts use floor division; reversal, rollover across the8192-phase boundary and int64 endpoint extremes are tested. Differences are computed without subtracting two large floating-point positions or overflowing signed raw counters. The32-interval weighted least-squares fitter rejects rank deficiency and nonmonotone solutions. Complete-turn measurements alone cannot identify phase error.

Counts must be unwrapped and referred to the calibrated Z-index phase. Missing index, changed encoder/wheel mounting, stale samples, lost edges or changed wire/contact conditions invalidate use. This engineering library does not create those hardware assurances or enable motion.

The chosen metrology allocation is **0.025 mm +0.01% of reference travel**. Each supplied validation observation is checked using |predicted-reference| plus the supplied absolute reference-uncertainty bound. This is one error-budget allocation, not total machine accuracy. A statistical expanded uncertainty is not an absolute guarantee; its coverage must be recorded. No repeatability, coverage or traceability is inferred from a CSV.

## SYNTHETIC benchmark — NOT machine calibration

Circumference from the map: **200.14 mm**. Local pitch 0.02435494519–0.0245073595 mm/count. 784 verification observations; 0 allocation failures; max absolute residual **0.0003605584494 mm**, max guarded residual **0.001360558449 mm**. Forward records: yes; reverse: yes.

For the synthetic first-harmonic0.2 mm TIR profile only, the independent linear-interpolation distance bound is **0.0009638285548 mm**. Arbitrary tread lobing, contact deformation, slip and time-varying errors do not inherit this bound. Synthetic files are labelled explicitly and may not be loaded as production calibration.

## Next mechanical interfaces

The wheel/encoder/626 interfaces are transcribed separately from unprovided OEM interiors. Measuring shaft, bearing housings, pressure arm, wire guides and support attachment must be completed before these become an installed module. The supplied encoder coupling still needs its actual hub/flexure engagement and tightening information. No spring load is applied directly to the encoder shaft. No personnel-safety function depends on this calibration.
