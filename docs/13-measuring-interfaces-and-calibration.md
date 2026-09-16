# Measuring interfaces and calibration — R2

## 1. Selected parts and actual drawing dimensions

One module upstream and one downstream. Each retains **ifm E60137** (measuring
wheel, U:200 N:06), **ifm RU3100** (encoder), the supplied Ø6 coupling, and
**two SKF 626-2Z** shaft-support bearings. The wheel load belongs on its own
bearings, not on the encoder shaft. The mechanism CAD additionally models,
per module, one 626 as the fork counter-roller (outer ring runs on the wire
underside) and two 626 as the arm pivot bearings: **five 626 per module, ten
across the two modules** (ME-B01 quantity 10, 2026-09-16). The CAD models one
representative module; the second is its mirror and is not modeled.

**Substitution record 2026-09-16** (gates 33/34 closed, letters L3/L4
resolved, [findings](../engineering/sources/measurement-replacement-findings.md)):

- The Kübler 8.0000.3123.0006 wheel and the Autonics E40S6-1024-6-L-5 encoder
  had no public price; both were replaced with priced, in-stock automation24
  equivalents: **ifm E60137** (EUR 57.00 ex VAT, item 102756) and **ifm
  RU3100** (EUR 186.40 ex VAT, item 102717).
- The E60137 is a **200 mm** circumference wheel (Ø63.66): the length
  resolution halves from 0.024414 to **0.048828 mm/count** at the 1024 PPR /
  ×4 quadrature factory setting. The RU3100 resolution is IO-Link
  programmable 1…10000 PPR; if finer resolution is ever required, 2048 PPR
  restores 0.024414 mm/count with no mechanical change.
- The RU3100 shaft is **10 mm long** (Autonics 15 + 5 pilot): the plate
  stays at Y70 and the coupling moved from Y44 to **Y48** so the shaft still
  reaches the coupling clamp; the encoder tip is **10 mm from the mounting
  face**.

| Interface | Current definition | Source |
|---|---|---|
| Wheel circumference | **200 mm**, free wheel; **no manufacturer tolerance published** | [ifm E60137 datasheet](../engineering/sources/ifm-e60137-datasheet-en.txt), E60137-01 EN-GB |
| Wheel OD | **Ø63.66 mm** | Same datasheet |
| Wheel surface/bore | PU (polyurethane) on aluminium, **90 Shore A**; bore Ø6; 64.5 g | Same datasheet |
| Wheel profile stations | 9.5 tread / 14.5 overall / Ø16.5 hub / Ø9 end land; 4×M4×6 radial clamp openings | **Retained envelope**: ifm publishes no hub, width or screw dimensions; confirm at incoming inspection |
| Encoder body | **Ø58.6 × 63.5 mm** rearward from mounting face | [ifm RU3100 datasheet](../engineering/sources/ifm-ru3100-datasheet.pdf) |
| Encoder shaft | **Ø6 × 10 mm**, no flat; tip **10 mm from the mounting face** | Same datasheet (63.5 = 53.5 body + 10 shaft) |
| Encoder fixing | Synchro flange: **3×M4×0.7, 6 mm deep, PCD42**; flange OD 58.6; drawing labels no pilot | Same datasheet drawing (labels Ø42 and Ø58.6 only) |
| Encoder electrics | 4.75–30 VDC, HTL/TTL push-pull, factory 1024 PPR, 1000 kHz, 12000 rpm, ≤0.01 Nm starting torque, M12 5-pole | Same datasheet |
| Supplied coupling | Ø15×22; Ø6.0–6.1 bores; four M3 screws 2.8 from ends | Same drawing; hub depth/flexure interior/torque not dimensioned |
| Bearing | 6×19×6; C2340 N; C0 950 N; CN2–13 µm unmounted | [SKF record](../engineering/sources/skf-626-2z.pdf) and [clearance table](../engineering/sources/skf-dgbb-clearance-selected.csv) |

The current [SKF626-2Z product table](https://www.skf.com/group/products/rolling-bearings/ball-bearings/deep-groove-ball-bearings/productid-626-2Z) gives **P6 dimensions/tighter width**, with bore5.993–6.000, OD18.992–19.000 and width5.940–6.000 mm; **P5 geometrical tolerances**, including4 µm inner assembled radial runout. Current mass8.5 g and minimum-load factor0.03 replace the2020 archive's8.8 g/0.025 for subsequent calculations. This does not solve the installed CN clearance/thermal stack.

The individual [interface STEP manifest](../engineering/generated/measurement/manifest.csv) identifies catalogue envelopes. These are **not original supplier CAD**, nor instructions to manufacture a rubber wheel, bearing or coupling. Undimensioned rubber bond/core layers, coupling flexure cuts and encoder cable routing are not fabricated from illustration pixels. The ifm E60137 hub/clamping-screw geometry is likewise **not invented**: the previous wheel's profile stations and four M4×6 openings are retained as placeholders pending the manufacturer drawing.

## 2. M201 encoder plate

[A3 machining drawing](../engineering/generated/drawings/M201-R2-encoder-plate.pdf), [plate STEP](../engineering/generated/measurement/step/M201.step), [five-part encoder mount STEP](../engineering/generated/measurement/encoder-mount.step).

[Blank M201 inspection record](../engineering/generated/measurement/M201-inspection-template.csv): no measurement or acceptance result has been prefilled. Coordinate/form/roughness inspection requires suitable calibrated metrology; arrange external inspection for instruments not available in the workshop.

- Quantity2, EN AW-6082-T6; rough stock66×66×8 mm.
- Finish60.00±0.05 square, thickness6.00±0.02.
- A = encoder mounting face, local Y0. B = central bore axis. C = left edge X−30 viewed from coupling side.
- Central seat boreØ36.50–36.55 through (Ø36.5-class synchro-flange seat; the archived drawing labels no pilot — confirm the centering feature at receipt); cylindricity0.01, perpendicularity0.02 toA.
- ThreeØ4.50–4.60 through onPCD42, basic coordinates (X,Z): (0,21), (18.1865,−10.5), (−18.1865,−10.5).
- FourM4×0.7 tapped through at (±18,±18), for the plate support screws.
- Hole positionØ0.10 toA|B|C. A flatness0.02; opposite face parallelism0.02.
- BoreRa0.8 µm; joint facesRa1.6 µm; othersRa3.2 µm. EdgesC0.20±0.05, including drilled-hole mouths. Chamfers are present in the STEP.
- No coating on locating/joint surfaces.
- Per plate: threeISO4762 M4×12 class8.8 into the encoder flange (nominal engagement6.0 mm = the ifm thread depth; no washer) and fourISO4762 M4×6 class8.8 support screws whose heads seat in the three PCD42 and four (±18,±18) Ø7.5×4 counterbores of the M203 housing rear face. These are nominal stack checks, not published fastener-length/thread-depth tolerances.

**Machining and inspection**

1. Square stock and finish the two broad faces. Mark A without damaging it.
2. Finish the central bore perpendicular to A, then drill the three clearance holes and tap the four M4 support holes from the same setup.
3. Chamfer and remove burrs without dishing A. Clean away chips.
4. Inspect width, thickness at corners, flatness, bore size/cylindricity, hole size and positions, tap engagement.
5. Record the plate identifier, measured values, instrument uncertainty and material certificate.

**Encoder face assembly**

1. Keep power disconnected; clean the encoder flange face and plate.
2. Put the encoder flange face on A (shaft passes the seat bore) without force.
3. Install the three M4×12 screws from the coupling side into the flange. Do not exceed the 6 mm thread depth; never draw a misaligned flange in with the screws.
4. Install the four M4×6 support screws from the coupling side with their heads in the M203 counterbores.
5. Check the shaft rotates without rubbing the plate, orient the M12 connector +X and retain cable service space. Coupling, shaft supports and guard installation follow their future completed assembly record.
6. Final tightening awaits an established encoder thread/preload limit. No unverified torque is substituted from a different component's manual.

This completes the **face-plate geometry**, not the measuring shaft/pressure mechanism or its attachment to the machine. The combined287-part head/feed placement remains unchanged; the encoder mount is a separate5-part export.

## 3. Corrected source-based calculations

- Nominal x4 resolution:200/4096 = **0.048828125 mm/count** (was0.0244140625 with the 100 mm wheel). Resolution is not length accuracy.
- At500 mm/s and the assumed minimum199.8 mm circumference (0.1% proportional; ifm publishes none): approximately150.1502 rpm,2562.5626 A/B cycles/s and10250.2503 quadrature edges/s. The encoder's1000 kHz limit is an A/B pulse-frequency rating, not the MCU input limit.
- Factory scale alone can produce **1.0 mm error per commanded metre** (assumed ±0.2 mm on200 mm), exceeding the whole±0.70 mm target. Adding the previous0.2 runout,0.05 quantisation,0.15 tension and0.25 slip allocations gives **1.65 mm**, not1.15. Those other terms are allocations, not measured physical bounds.
- Free-wheel circumference/OD tolerances do not establish loaded rolling radius. Do not add both as independent scale uncertainties or assume rubber thermal expansion equals aluminium expansion.
- With CN clearance2 µm and10 K inner/outer ring difference, SKF's thermal equation consumes1.5 µm. The remaining **0.5 µm precedes any press-fit loss**. The measurement bearing fits/thermal field remain an explicit design task; the feed rotor's j5/C3 match is not copied onto these CN bearings.

[Current analytic-only report](../engineering/generated/current-analytical/report.md) and [measurement report](../engineering/generated/measurement-analysis/report.md). Earlier R4/R6 FEM archives retain the analytical assumptions used when run; their1.15 mm uncalibrated budget is superseded. Their actual FEM solutions/convergence results are not rerun or relabelled by this correction.

## 4. Calibration implementation

[C++ model](../engineering/include/measurement_models.hpp), [implementation](../engineering/src/measurement_models.cpp), [tests](../engineering/tests/test_measurement_models.cpp).

For signed unwrapped count $n$, split it by **floor division** into whole turns $q$ and phase $r$, with $0\le r<4096$. The mapping is $x(n)=qC+p(r)$, where $p$ is strictly increasing, $p(0)=0$ and $p(4096)=C$. Distance is the difference of these terms, not a floating-point subtraction of two enormous absolute positions. Negative travel and phase-boundary reversals use the same mapping. $C$ is the calibrated circumference; the factory nominal is now **200 mm** per turn, so the nominal scale is 20480 counts per metre (was 40960).

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
9. Commission the actual hardware capture, line receivers, timestamping and settling logic before using this model for motion. The retained legacy reference firmware has `NotCommissioned`, disabled hardware outputs,750000 µs heartbeat and no automatic restart; this is not a claim about the separate, unqualified RP2350 target. **J8/J9 receiver review CLOSED 2026-09-16:** the RU3100 must be parameterised to **TTL mode** (IO-Link; the factory setting is HTL 50 mA); its single-ended A/B outputs drive the AM26LV32EIDR + inputs with the − inputs tied to a 2.5 V mid-bias, and the four 120 Ω RS-422 terminators are replaced by the bias network (IF-B01 board change). The TTL swing stays inside the AM26LV32E −7…+7 V common-mode range (docs/22 §6/§7, hardware/if-b01/README).

The synthetic first-harmonic example has a rigorously checked interpolation bound of approximately0.000964 mm for a distance with two endpoint errors. Arbitrary lobing, rubber indentation, slip and time-varying contact do **not** inherit that bound. The final complete length/strip budget must also include wire extension, cutting/holding displacement, contact/slip, guide path, temperature, quantisation and reference uncertainty without double counting.
