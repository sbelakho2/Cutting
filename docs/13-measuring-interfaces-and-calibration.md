# Measuring interfaces and calibration — R1

## 1. Selected parts and actual drawing dimensions

One module upstream and one downstream. Each retains **Kübler 8.0000.3123.0006**, **Autonics E40S6-1024-6-L-5**, the encoder's supplied Ø6 coupling, and **two SKF 626-2Z** shaft-support bearings. The wheel load belongs on its own bearings, not on the encoder shaft. The mechanism CAD additionally models, per module, one 626 as the fork counter-roller (outer ring runs on the wire underside) and two 626 as the arm pivot bearings: **five 626 per module, ten across the two modules** (ME-B01 quantity 10, 2026-09-16). The CAD models one representative module; the second is its mirror and is not modeled.

| Interface | Current definition | Source |
|---|---|---|
| Wheel circumference | **100 ±0.1 mm**, free wheel | [Kübler catalogue](../engineering/sources/kuebler-messrad-en.pdf), page4, No.123 |
| Wheel OD/tread/overall width | Ø31.83 (±0.025), 9.5, 14.5 mm | Same drawing |
| Wheel bore/hub | Ø6 H7; Ø16.5 hub; Ø9 end land, 0.75 long | Same drawing |
| Wheel screw | **M4×6 ISO4029**, centre 2.5 from hub end | No.123 callout; four openings depicted, screw quantity not separately stated |
| Encoder body and pilot | Ø40×31 rearward from mounting face; Ø19.98–20.00×5 forward | [Autonics drawing](../engineering/sources/autonics-e40-drawing.png) |
| Encoder shaft | Ø5.985–5.990; 15 beyond pilot; **tip20 from mounting face** | Same drawing |
| Encoder flat | One flat, 5 across; last10 mm of shaft | Same drawing; not two opposing flats |
| Encoder fixings | Three M3×0.5, 5 deep, PCD30, 120° apart | Same drawing |
| Supplied coupling | Ø15×22; Ø6.0–6.1 bores; four M3 screws 2.8 from ends | Same drawing; hub depth/flexure interior/torque not dimensioned |
| Bearing | 6×19×6; C2340 N; C0 950 N; CN2–13 µm unmounted | [SKF record](../engineering/sources/skf-626-2z.pdf) and [clearance table](../engineering/sources/skf-dgbb-clearance-selected.csv) |

The adjacent No.113 aluminium wheel uses a different screw callout. The earlier statement that the selected wheel had no published circumference tolerance is corrected. [Numeric transcription](../engineering/sources/measurement-interfaces.csv).

The current [SKF626-2Z product table](https://www.skf.com/group/products/rolling-bearings/ball-bearings/deep-groove-ball-bearings/productid-626-2Z) gives **P6 dimensions/tighter width**, with bore5.993–6.000, OD18.992–19.000 and width5.940–6.000 mm; **P5 geometrical tolerances**, including4 µm inner assembled radial runout. Current mass8.5 g and minimum-load factor0.03 replace the2020 archive's8.8 g/0.025 for subsequent calculations. This does not solve the installed CN clearance/thermal stack.

The individual [interface STEP manifest](../engineering/generated/measurement/manifest.csv) identifies catalogue envelopes. These are **not original supplier CAD**, nor instructions to manufacture a rubber wheel, bearing or coupling. Undimensioned rubber bond/core layers, coupling flexure cuts and encoder cable routing are not fabricated from illustration pixels.

## 2. M201 encoder plate

[A3 machining drawing](../engineering/generated/drawings/M201-R1-encoder-plate.pdf), [plate STEP](../engineering/generated/measurement/step/M201.step), [eight-part encoder mount STEP](../engineering/generated/measurement/encoder-mount.step).

[Blank M201 inspection record](../engineering/generated/measurement/M201-inspection-template.csv): no measurement or acceptance result has been prefilled. Coordinate/form/roughness inspection requires suitable calibrated metrology; arrange external inspection for instruments not available in the workshop.

- Quantity2, EN AW-6082-T6; rough stock50×50×8 mm.
- Finish46.00±0.05 square, thickness6.00±0.02.
- A = encoder mounting face, local Y0. B = central bore axis. C = left edge X−23 viewed from coupling side.
- Central boreØ20.040–20.060 through; cylindricity0.01, perpendicularity0.02 toA. Diametral clearance to the actual encoder pilot is0.040–0.080.
- ThreeØ3.40–3.50 through onPCD30, basic coordinates (X,Z): (0,15), (12.990381,−7.5), (−12.990381,−7.5).
- FourØ4.50–4.60 through at (±18,±18), for the eventual support interface.
- Hole positionØ0.10 toA|B|C. A flatness0.02; opposite face parallelism0.02.
- BoreRa0.8 µm; joint facesRa1.6 µm; othersRa3.2 µm. EdgesC0.20±0.05, including drilled-hole mouths. Chamfers are present in the STEP.
- No coating on locating/joint surfaces.
- Per plate: threeISO4762 M3×10 class8.8 and threeISO7092 M3 washers3.2×6×0.5,200HV. Nominal encoder thread engagement3.5 mm and bottom clearance1.5 mm. These are nominal stack checks, not published fastener-length/thread-depth tolerances.

**Machining and inspection**

1. Square stock and finish the two broad faces. Mark A without damaging it.
2. Finish the central bore perpendicular to A, then drill all seven clearance holes from the same setup.
3. Chamfer and remove burrs without dishing A. Clean away chips.
4. Inspect width, thickness at corners, flatness, bore size/cylindricity, hole size and positions.
5. Record the plate identifier, measured values, instrument uncertainty and material certificate.

**Encoder face assembly**

1. Keep power disconnected; clean the encoder face/pilot and plate.
2. Put the encoder body behind A and pass the pilot through the bore without force.
3. Install the three M3×10 screws and specified washers from the coupling side. Never draw a jammed pilot into the bore with the screws.
4. Check the shaft rotates without rubbing the plate and retain cable service space. Coupling, shaft supports and guard installation follow their future completed assembly record.
5. Final tightening awaits an established encoder thread/preload limit. No unverified torque is substituted from a different component's manual.

This completes the **face-plate geometry**, not the measuring shaft/pressure mechanism or its attachment to the machine. The combined287-part head/feed placement remains unchanged; the encoder mount is a separate8-part export.

## 3. Corrected source-based calculations

- Nominal x4 resolution:100/4096 = **0.0244140625 mm/count**. Resolution is not length accuracy.
- At500 mm/s and the factory minimum99.9 mm circumference: approximately300.3003 rpm,5125.1251 A/B cycles/s and20500.5005 quadrature edges/s. The encoder's300 kHz limit is an A/B pulse-frequency rating, not the MCU input limit.
- Factory scale alone can produce **1.0 mm error per commanded metre**, exceeding the whole±0.70 mm target. Adding the previous0.2 runout,0.05 quantisation,0.15 tension and0.25 slip allocations gives **1.65 mm**, not1.15. Those other terms are allocations, not measured physical bounds.
- Free-wheel circumference/OD tolerances do not establish loaded rolling radius. Do not add both as independent scale uncertainties or assume rubber thermal expansion equals aluminium expansion.
- With CN clearance2 µm and10 K inner/outer ring difference, SKF's thermal equation consumes1.5 µm. The remaining **0.5 µm precedes any press-fit loss**. The measurement bearing fits/thermal field remain an explicit design task; the feed rotor's j5/C3 match is not copied onto these CN bearings.

[Current analytic-only report](../engineering/generated/current-analytical/report.md) and [measurement report](../engineering/generated/measurement-analysis/report.md). Earlier R4/R6 FEM archives retain the analytical assumptions used when run; their1.15 mm uncalibrated budget is superseded. Their actual FEM solutions/convergence results are not rerun or relabelled by this correction.

## 4. Calibration implementation

[C++ model](../engineering/include/measurement_models.hpp), [implementation](../engineering/src/measurement_models.cpp), [tests](../engineering/tests/test_measurement_models.cpp).

For signed unwrapped count $n$, split it by **floor division** into whole turns $q$ and phase $r$, with $0\le r<4096$. The mapping is $x(n)=qC+p(r)$, where $p$ is strictly increasing, $p(0)=0$ and $p(4096)=C$. Distance is the difference of these terms, not a floating-point subtraction of two enormous absolute positions. Negative travel and phase-boundary reversals use the same mapping.

The selected fitting grid has32 intervals of128 counts. Weighted least squares uses supplied reference uncertainties. Rank-deficient data, including complete-turn-only observations, cannot determine the phase map and are rejected. Nonmonotone solutions are rejected instead of clipped. The map loader accepts strictly increasing variable knot counts; the fitter emits the fixed32-interval grid.

CSV formats:

- Map header: `count,travel_mm`; start0,0; end4096,C; all intermediate counts/travel strictly increasing and finite.
- Observation header: `from_count,to_count,reference_mm,reference_uncertainty_mm`; signed64-bit unwrapped counts, signed reference distance and nonnegative absolute uncertainty. Fitting requires strictly positive uncertainty.
- No trailing junk, extra/missing columns, blank rows, NaN, infinity, duplicate knots or out-of-range integer conversion is accepted. CRLF is accepted.

The `starz_measurement_analysis` executable accepts `--report`; an optional `--fit-observations` or `--calibration` requires a separate `--verify-observations` file. It refuses identical fitting/verification paths and protects input files against its own output names. `--require-factory-accuracy` returns2 while the exposed source gates fail. Invalid inputs return1 before creating a pending report; excessive validation residuals return2.

**Without supplied data the report is a labelled synthetic benchmark.** [Synthetic fixtures](../engineering/generated/measurement-analysis/synthetic-fit.csv) and [synthetic validation](../engineering/generated/measurement-analysis/synthetic-validation.csv) are not commissioned calibration. No hardware output or reference-controller approval is enabled by this executable.

## 5. Measurement procedure to qualify later

1. Finish and inspect the bearing-supported wheel, pressure mechanism and guide path. Assign wheel, shaft, encoder and mounting identifiers. Record wire manufacturer/lot, conductor area, OD, contact force, tension, temperature, direction and speed.
2. Use a traceable independent displacement reference. Record its uncertainty and whether it is a bounded error or an expanded statistical uncertainty with stated coverage. A nominal ruler length is not an adequate uncertainty record.
3. Latch the same electrical Z-index/AB-state reference used during calibration. Capture unwrapped quadrature counts and reference displacement synchronously. No synthetic feedback or post-hoc count reset is permitted.
4. Acquire multiple turns and partial-turn spans covering every128-count interval in both directions. Include multiple starting phases and the200,500,1000 and3000 mm operating spans. Record fitting observations separately from later verification acquisitions.
5. Fit the phase map. Complete-turn-only fitting must fail; it estimates scale but leaves phase shape unobservable.
6. Verify on separately acquired, held-out observations, including reversals and repeated starts/stops. For each record require $|x_\text{predicted}-x_\text{reference}|+U_\text{reference}\le0.025\text{ mm}+0.0001|x_\text{reference}|$. This is a **chosen metrology allocation**, not a claim that any physical wheel has passed it or that total finished-wire error is within target.
7. Check every wire/contact/temperature/speed condition intended for one recipe; do not extrapolate a calibration to other pressure, tension, material, wear or thermal states. Inspect direction-dependent hysteresis separately; one reversible map cannot conceal it.
8. Bind the accepted record to the assembly identifiers, Z phase, recipe and qualified operating range. Replacing/moving a wheel, encoder or coupling; losing index/count integrity; abnormal wheel disagreement; or operating outside that range invalidates it.
9. Commission the actual hardware capture, line receivers, timestamping and settling logic before using this model for motion. The retained legacy reference firmware has `NotCommissioned`, disabled hardware outputs,750000 µs heartbeat and no automatic restart; this is not a claim about the separate, unqualified RP2350 target.

The synthetic first-harmonic example has a rigorously checked interpolation bound of approximately0.000964 mm for a distance with two endpoint errors. Arbitrary lobing, rubber indentation, slip and time-varying contact do **not** inherit that bound. The final complete length/strip budget must also include wire extension, cutting/holding displacement, contact/slip, guide path, temperature, quantisation and reference uncertainty without double counting.