# Measuring-wheel interfaces and calibration model

Revision **S1-B-MEAS-R1-2026-09-12**.

## Source corrections

Kübler wheel **8.0000.3123.0006**, catalogue pp.3–4: **100 ±0.1 mm** free circumference; Ø31.83 (±0.025) mm, 9.5 mm tread, 14.5 mm overall, Ø6 H7 bore, Ø16.5 mm hub and Ø9 mm end land. The No.123 callout is **M4×6 ISO4029**, not No.113's M3×5. Its section shows holes but does not separately state screw quantity. The rubber section/bond thickness and edge chamfers are not dimensioned. No such dimensions are invented in the interface model.

Autonics **E40S6-1024-6-L-5**: body31 mm behind mounting face, pilot5 mm in front, shaft15 mm beyond pilot. The tip is **20 mm from the mounting face**, not15 mm. Shaft diameter5.985–5.990 mm; single flat5 mm across, last10 mm long; pilot19.98–20.00 mm;3×M3×0.5 on PCD30,5 mm deep. Original public CAD viewer links a shared800/75PPR/24V family model; it is not relabelled as the selected1024PPR/5V unit.

## Uncalibrated length and bearing gates

The factory circumference alone gives **1.0 mm error per commanded metre**. Adding the previous0.2 runout,0.05 quantisation,0.15 tension and0.25 slip allocations gives **1.65 mm**, not the earlier1.15 mm. Those four terms are allocations, not measured errors; this is a failing uncalibrated budget, not a worst-case physical guarantee. The separate factory diameter tolerance is not treated as an additional independent scale error.

SKF626-2Z CN minimum clearance2 µm leaves only **0.5 µm** with10 K inner-to-outer ring difference, even before a press fit. The chosen CN bearing is not silently accepted with full js5 interference. Its actual matching/thermal design remains open.

At500 mm/s and99.9 mm circumference: 300.3003003 rpm, 5125.125125 A/B cycles/s and 20500.5005 quadrature edges/s. The300 kHz encoder limit is a pulse-frequency specification; MCU edge capture and electrical receivers are separate work.

## Reversible phase calibration

The mapping is C times signed whole turns plus a strictly increasing one-turn distance table. Negative counts use floor division; reversal, rollover across the4096-phase boundary and int64 endpoint extremes are tested. Differences are computed without subtracting two large floating-point positions or overflowing signed raw counters. The32-interval weighted least-squares fitter rejects rank deficiency and nonmonotone solutions. Complete-turn measurements alone cannot identify phase error.

Counts must be unwrapped and referred to the calibrated Z-index phase. Missing index, changed encoder/wheel mounting, stale samples, lost edges or changed wire/contact conditions invalidate use. This engineering library does not create those hardware assurances or enable motion.

The chosen metrology allocation is **0.025 mm +0.01% of reference travel**. Each supplied validation observation is checked using |predicted-reference| plus the supplied absolute reference-uncertainty bound. This is one error-budget allocation, not total machine accuracy. A statistical expanded uncertainty is not an absolute guarantee; its coverage must be recorded. No repeatability, coverage or traceability is inferred from a CSV.

## SYNTHETIC benchmark — NOT machine calibration

Circumference from the map: **100.07 mm**. Local pitch 0.02427873803–0.02458356666 mm/count. 392 verification observations; 0 allocation failures; max absolute residual **0.0006502037777 mm**, max guarded residual **0.001650203778 mm**. Forward records: yes; reverse: yes.

For the synthetic first-harmonic0.2 mm TIR profile only, the independent linear-interpolation distance bound is **0.0009638285548 mm**. Arbitrary tread lobing, contact deformation, slip and time-varying errors do not inherit this bound. Synthetic files are labelled explicitly and may not be loaded as production calibration.

## Next mechanical interfaces

The wheel/encoder/626 interfaces are transcribed separately from unprovided OEM interiors. Measuring shaft, bearing housings, pressure arm, wire guides and support attachment must be completed before these become an installed module. The supplied encoder coupling still needs its actual hub/flexure engagement and tightening information. No spring load is applied directly to the encoder shaft. No personnel-safety function depends on this calibration.
