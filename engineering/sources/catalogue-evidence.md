# Verified catalogue evidence

Research date: 2026-09-11. Numerical values below are transcribed facts, not an assertion that an assembled machine has achieved them. Preserve decimal separators: HIWIN's `7.144 N` means **7144 N**, not 7.144 N.

## AX-01 — selected complete linear axis

- Manufacturer: HIWIN.
- Selected ordering identity: **KK8610P340A1F0**, article **10.00026**.
- [Manufacturer product record](https://www.hiwin.de/en/Products/Precision-axes-%26-precision-systems/Precision-axes-KK-KF/KK/KK8610P340A1F0/p/10.00026).
- [Catalogue drawing, PDF page 19](hiwin-kk-stages.pdf); [rendered drawing](hiwin-kk86-drawing.png). Retrieved from [HIWIN UK / CRD Devices catalogue](https://hiwin.co.uk/wp-content/uploads/2017/05/Data-Sheets-KK-Linear-Stages.pdf).
- The older drawing lists 210 mm physical travel. The current manufacturer page lists **194 mm usable travel and 8 mm reserve at each end**. Use the current usable limit, not both numbers interchangeably.

| Quantity | Value |
|---|---:|
| Rail length / total length | 340 / 440 mm |
| Rail width | 86 mm |
| Carriage top above mounting plane | 46 mm |
| Screw axis above mounting plane | 32 mm |
| Nominal screw diameter / lead | 15 / 10 mm |
| Input shaft diameter / exposed length | 8 h7 mm / **18 mm** beyond the locknut; **not** the 15 mm screw diameter or 28 mm length including the locknut |
| Moving mounting pad | 60 × 75 mm; overall block length including ends 116 mm |
| Carriage attachment | 4 × M6 × 1, 12 mm deep, 46 × 46 mm pitch |
| Base mounting holes | 6 × Ø6.6 through, Ø11 × 6 counterbore; 46 mm transverse pitch; 100 mm longitudinal pitch; first/last at 70 mm from rail ends |
| Rail end thickness before rail-length datum | 13 mm in catalogue overall-length chain |
| Fixed-end nominal coupling cavity | Ø46 mm; F0 locating bore Ø50, +0.05/+0 mm in the drawing |
| F0 attachment | 4 × M5 × 0.8, 10 mm deep, PCD70; separate PCD60 M4 pattern also shown |
| Ballscrew basic dynamic / static rating | 7144 / 12642 N |
| Guide basic dynamic / static rating | 31458 / 50764 N |
| Guide static roll moment / pitch / yaw | 1507 / 622 / 622 Nm |
| Fixed-bearing basic static axial rating | 9240 N |
| Fixed-bearing maximum axial load | **6320 N** |
| Starting torque | 0.150 Nm |
| Repeatability / positioning accuracy / guide parallelism | ±0.003 / 0.025 / 0.015 mm |
| Maximum catalogue speed / acceleration | 0.74 m/s / 15 m/s² |
| Catalogue assembly mass | 5.7 kg |
| Section inertias, catalogue labels Ix / Iy | 74450 / 1134000 mm⁴ |

Manufacturer internal screw-root geometry, individual bearing identities, assembled compliance, nut preload and force-vs-displacement curves are **not** established by these figures. Do not fabricate them for an FEA model. Use the complete manufacturer's rating for the complete unit; use physical stiffness tests where internal compliance is needed.

### HIWIN assembly manual — retrieved 2026-09-12

- [Original KX-05-02-EN-2201-MA manual](hiwin-kk-kf-assembly.pdf), 28 pages, obtained from the selected product's Downloads tab: [manufacturer PDF](https://www.hiwin.de/medias/sys_master/hiwinDocumentMedia/hiwinDocumentMedia/h44/hb8/14712638472222/KX-05-2-EN-2201-MA.pdf?attachment=true).
- Pages14–15: standard installation horizontal within1°; mounting-surface evenness0.03mm per300mm. Lift by the profile, preserve maintenance access and secure against unintended movement.
- Pages15–16, Table5.1: **KK/KF86 rail-to-base M6 mounting screws:10.2Nm**, all mounting holes used; tighten from the centre outward in a spiral sequence and secure against loosening.
- Page18, Table5.2: M3/M4/M5/M6/M8/M10 →1.2/2.9/6.0/12.2/24.9/49.4Nm. Its cited applications are the cover-block adapter and cross-table adapter in sections5.4–5.6. This is **not evidence of a specified torque for the custom H003-to-F0 motor adapter**; do not relabel it as such.
- The moving-load instructions prescribe clean joint faces, spiral tightening and full-travel clearance checks, but do not separately specify the custom holder's preload. Its9kN-per-screw calculation remains a required-preload screen, not a verified torque/preload result.

## DR-01 — selected motor and drive

- [Manufacturer kit 1-CL86T-P85-V41](https://www.omc-stepperonline.com/tp-series-1-axis-closed-loop-stepper-cnc-kit-8-5nm-1203-94oz-in-nema-34-motor-driver-1-cl86t-p85-v41).
- [Actual motor drawing](https://www.omc-stepperonline.com/image/catalog/dimensions/34E1K-85_Dimension.svg).
- [Actual torque-speed curve](https://www.omc-stepperonline.com/image/catalog/torque-curves/34E1K-85_Torque_Curve.svg).
- [Driver manual link exposed by manufacturer](https://www.omc-stepperonline.com/index.php?route=product/product/get_file&file=3155/CL86T-V41_Manual.pdf).
- Curve condition printed on image: **60 V, RMS 5.7 A, 1000 microsteps/revolution**. Do not reuse the curve at 36 V or 48 V. The curve is a catalogue pull-out characteristic, not a guaranteed hot-motor continuous rating.

| Quantity | Value |
|---|---:|
| Motor | 34E1K-85 |
| Holding torque / rated current | 8.5 Nm / 6 A per phase |
| Resistance / inductance | 0.54 Ω / 5.0 mH ±20% |
| Mounting-face square | 85.85 ±0.25 mm (product text rounds to 86 mm) |
| Mounting pitch / holes | 69.58 ±0.25 mm square; 4 × Ø6.5 +0.3/+0 through |
| Motor body / rear section | 112.3 ±1 / 21.8 mm |
| Shaft | Ø14, 0/−0.013 mm; 37 ±1 mm long; 5 mm keyway, 25 ±0.2 mm long |
| Locating boss | Ø73, 0/−0.05 mm; 1.6 ±0.2 mm projection |
| Leads | 270 mm; motor AWG18 ×4, encoder AWG26 ×6 |
| Encoder | 1000 PPR / 4000 quadrature counts per revolution; differential A/B |
| Driver | CL86T-V41, 24–110 VDC or 18–80 VAC, 0–8 A **peak** |
| Logic inputs, manual revision4.1 | 7–20 mA, nominal10 mA; 500 kHz at5 V or200 kHz at24 V |

[Downloaded full motor datasheet](34E1K-85.pdf), drawing dated2021-10-16: rotor inertia3600 g·cm² = **0.000360 kg·m²**; phase resistance0.54 Ω±10% at20°C; maximum shaft radial runout0.025 mm under450 g load; axial runout0.075 mm under920 g load. Ambient0–50°C; maximum80°C rise at standstill/two phases energised; classB130°C. Encoder pinout: DB15 pins1 EA+,2 VCC,3 GND,11 EB+,12 EB−,13 EA−.

[Downloaded driver manual](CL86T-V41.pdf), revision4.1, May2023, takes precedence over the page summary. Maximum reliable operating ambient **40°C**; vertical mounting; ≥30 mm between drives. Set **S1=A** (8 A peak /5.7 A RMS, Ki0/Kp25/Kp25) as the single initial commissioning configuration, not a proven servo tune. SW1–4=ON,ON,ON,OFF for1000 pulses/revolution; SW5=ON; SW6=OFF (closed loop); SW7=OFF (step/direction); SW8=ON (position-reached output). Select24 V input logic. Minimum pulse1 µs, direction lead2 µs; enable-to-direction delay200 ms. ALM/PEND maximum30 VDC/100 mA. Drive enables and alarm outputs are **not safety-rated STO**. Encoder supply+5 V is verified by the manual.

At 1000 commanded pulses/revolution and a 10 mm lead, one command pulse is **0.010 mm**. Encoder count spacing is **0.0025 mm**. Neither value is a guaranteed mechanical positioning accuracy. Peak driver current and RMS test current are not interchangeable.

Observed kit merchandise price: **USD 130.77**, China warehouse. No Tunisia freight quote. Do not turn “in stock” into a guaranteed delivery date. Some website encoder logic-voltage fields are inconsistent and are not an electrical-interface design specification.

## CP-01 — selected coupling

- Manufacturer-configured catalogue item: **R+W MK2/100/50/8/14**, H7 bores, without keyways.
- [Manufacturer datasheet](https://www.rw-america.com/wp-content/uploads/2023/03/RW-DS-PK-miniature-bellows-coupling-MK2-eng-221208.pdf); [local copy](rw-mk2-coupling.pdf).
- **Size100 means10 Nm**, not100 Nm. Bore range5–24 mm; selected50 mm length,40 mm diameter,16 mm fit length per hub.
- Torsional stiffness **9050 Nm/rad**; inertia160 g·cm² = **0.000016 kg·m²**; approximate mass120 g.
- Axial misalignment1 mm; lateral0.2 mm; angular1.5°. These maxima are not simultaneous installation targets. Assembly targets are ≤0.05 mm lateral, ≤0.1° angular, with coupling at its free axial length.
- One M4 clamp screw per hub, **4.5 Nm** tightening torque. The motor hub is tightened before the motor/adapter is inserted; the stage hub is accessed through the existing stage coupling window.
- **R5/R6 axial chain**, from F0 front face toward the rail: stage tip36 mm, exposed shaft36–54 mm; coupling nominally spans3–53 mm; stage hub37–53 mm; motor hub3–19 mm; motor face−15 mm and shaft tip22 mm. The shafts project1 and3 mm beyond their respective16 mm hubs, with14 mm nominal shaft-end separation. Set the stage hub **1.00±0.05 mm from the actual locknut shoulder**, not from an assumed coupling overall length.
- **R6 fasteners:**F0counterboresØ9.5×8.5deep,ISO4762M5×16class10.9withISO7092M5washers(Ø9×1,200HV). The old5.5deep recess places the standard head inside the original motor pilot and is rejected by a dedicated regression. R6 adds8F0screws,8motorM5×25screws and16washers to the two-half assembly. Motor-thread major/minor overlap is permitted only inside exactØ5.002mating cylinders;heads,pilot andall othergeometry remain collision-checked. [Dimensioned A3 drawing](../generated/drawings/H003-R6-adapter.pdf). The15mm axial stack and256-corner fit are unchanged. Its [R6 adapter FEM](../generated/adapter-r6-analysis/report.md) now uses a physically justified support/load boundary — stage-side washer/bolt-seat annuli r∈[2.75,4.5]mm (Ø5.5 hole edge to ISO7092 washer OD9) and the actual85.85mm motor-flange bearing footprint — with six uniform meshes6/4/2.8/2/1.4/1.0mm plus an isolated1.0mm-background→0.7mm-peak-region pair (finest110610nodes): final18.8267MPa,0.000713527mm displacement; displacement refinement0.306163% passes3%, but raw peak refinement14.7581% fails5% (uniform1.4→1.0mm5.52041%). The finest peak lies0.133mm outside the washer-OD seat edge,0.46mm off the stage face, and grows under peak-region refinement (16.05→18.83MPa): an ideal-clamp boundary singularity. This supports a contact/preload investigation, not removal of the failed peak criterion.
- Manufacturer tolerances: motor shaft37±1 mm; coupling free length50−1 mm. STARZ incoming acceptance dimensions, explicitly not supplier guarantees: F0-to-shoulder53.80–54.20 mm; exposed stage shaft17.80–18.20 mm; individual hub fit length15.80–16.20 mm. Adapter thickness15.00±0.02 mm. The256 independent interval corners leave at least0.53 mm motor-side and0.55 mm stage-side full-hub insertion reserve,12.58 mm between shafts, and0.95 mm locknut clearance. Maximum tip projection into the bellows is4.47 mm on the14 mm side and1.45 mm on the8 mm side; inspect this unobstructed volume before assembly. Never force the bellows axially to compensate for a wrong stack.
- R4's19 mm adapter and nominally flush shaft/hub ends are superseded: the shortest motor shaft and shortest coupling left incomplete motor-side engagement. The8 mm bore faces the stage; the14 mm bore faces the motor.
- No claim of personnel-safety torque limiting: this is a shaft coupling, **not** an overload clutch.
- **Assembly operation: remove the supplied motor's5×5×25mm parallel key before inserting the14mm shaft into the keyless MK2 hub.** The original supplier STEP includes the fitted key; its interference with the keyless coupling was detected by the exact solid intersection test. The assembly CAD removes only that removable key, using its original R2.495mm end geometry, and leaves the shaft and keyway unchanged.
- Price not observed; no fabricated USD amount.

### Original coupling CAD access — checked 2026-09-12

- [Current manufacturer MK2 datasheet, June-2026 download path](https://www.rw-america.com/wp-content/uploads/PDFs/RW-Handbuch/Datenblatter_06_2026/EN/MK_Torsionally-stiff-miniature-bellows-couplings/MK2_Datasheet.pdf), [archived PDF](rw-mk2-datasheet-2026.pdf), and [BK/MK installation instructions](rw-bk-mk-installation.pdf) are now archived. They retain the selected size's10Nm rating,40mm OD,16mm hub fit length and5–24mm configurableH7 bores. Neither archive dimensions the free bellows interior or approves the calculated tip projections.
- [R+W exact MK2/100/50 configurator](https://www.rw-america.com/product/mk2/#configurator-MK2_100_50-MK2_100_50): selecting **Open now** presents **Login CAD Download**, with email/password fields. This was reproduced in the browser; no credentials were requested, entered or bypassed.
- The manufacturer's legacy CAD domain redirects to its homepage. Exact MISUMI listings for [8/14](https://uk.misumi-ec.com/vona2/detail/221006398450/?HissuCode=MK2%2F100%2F50%2F8%2F14) and [10/12](https://uk.misumi-ec.com/vona2/detail/221006398450/?HissuCode=MK2%2F100%2F50%2F10%2F12) also require login for CAD download. No original coupling STEP has been acquired.
- **Coupling-interior clearance remains unverified.** Full-hub engagement and shaft-end separation checks are not an internal-bellows clearance check. The nominal open cavity in catalogue-interface CAD must not be used as evidence of an actual Ø28mm internal bore.

## PS-01 — selected motion supply

- [Manufacturer LE-350-60 product](https://www.omc-stepperonline.com/350w-60v-5-8a-115-230v-switching-power-supply-stepper-motor-cnc-router-kits-le-350-60).
- [Original supply specification](LE-350-60.pdf) and [original supplier STEP](LE-350-60.step), captured from the manufacturer's current `3603` document endpoints. The older `3140` endpoint is not the current document link.
- Actual60 V,5.8 A,350 W nominal class;60 ×5.8 = **348 W** continuous arithmetic ceiling used for preliminary loading.
- Input115/230 VAC±20%, switch selected,47–63 Hz. **40 A is cold inrush at230 VAC**, as explicitly stated in the original PDF; it is not continuous mains consumption.
- 215 ×115 ×30 mm; built-in cooling fan; leakage<0.5 mA at230 VAC. Four underside M4 fixings have **3 mm maximum penetration**; side M4 fixings have **5 mm maximum penetration**. Use the original mounting drawing, not the external bounding box, for hole locations.
- Output stability0.5%; linear adjustment≤0.5%; ripple/noise≤1% of output; temperature coefficient≤0.02%/°C. Adjustment54–66 V. Efficiency≥89.5%; rise50 ms and hold20 ms at230 VAC/full load.
- Full rated output0–45°C;80% at−10°C;40% at60°C. Relative humidity20–90%, noncondensing. Short-circuit automatic recovery; overload180–220% for5 s, automatic recovery.
- The sheet's safety-standard wording is **design compliance**, not a third-party certification record. It does not specify an overvoltage trip threshold or permission to absorb regenerated energy. Those two functions remain external circuit requirements.
- Observed merchandise price **USD17.88**, China warehouse; not delivered pricing.
- One separate supply per cutting axis. No uncontrolled paralleling. A bus discharge/overvoltage circuit and its component ratings remain required before electrical release.

## ME-01 — selected measuring wheel

- [Kübler original local catalogue](kuebler-messrad-en.pdf), PDF pages2–4.
- Order structure: `8.0000.3XXX.00XX`; wheel number123 and bore06 unambiguously configure **8.0000.3123.0006**:100 mm nominal circumference,9.5 mm tread,natural rubber60 Shore A,6 mm bore,16 g.
- This is a catalogue configuration, **not** one of the stock codes listed at the bottom of page3. Its exact stock availability and price are not established.
- **Corrected after direct inspection of page4:** free circumference **100 ±0.1 mm**; diameter **31.83 (±0.025) mm**. The previous statement that circumference tolerance was absent was wrong. Its factory scale error is1.0 mm per commanded metre, before contact/loading effects. Do not add the diameter tolerance as a second independent scale error.
- Page4, **No.123 natural rubber**: tread9.5 mm, overall14.5 mm, boreØ6 H7, hubØ16.5 mm, end hubØ9 mm with0.75 mm end land. Radial screw centre2.5 mm from the hub end; callout **M4×6 ISO4029**. Four radial openings are depicted; screw quantity is not separately stated in the No.123 callout. No.113's explicit4×M3×5 callout belongs to the adjacent aluminium/diamond-knurl wheel and must not be transferred.
- Maximum radial and lateral runout0.2 mm. Loaded circumference, pressure/temperature dependence, tread lobing and bond/core geometry are not specified by these dimensions. The [C++ measurement model](../include/measurement_models.hpp) separates the actual catalogue scale tolerance from synthetic phase-calibration benchmarks.
- The previous part identity `05.00.0050.1004` and100 mm ×12 mm geometry are not supported.

## TR-01 — selected feed motor and driver

- Two **24HS40-5004D-E1000** motors, two **CL57T-V41** drives and one shared **LE-350-36** supply, with individually protected star-wired driver branches. Do not reuse the60 V cutting supply for these drives.
- Original [motor PDF](24HS40-5004D-E1000.pdf), [motor STEP](24HS40-5004D-E1000.step), [drive manual](CL57T-V41.pdf), [drive STEP](CL57T-V41.step) and [36 V supply PDF](LE-350-36.pdf) are archived.
- Motor shaftØ10,21±1 mm; registerØ38.1×1.6 mm; **mounting grid50×50 mm**, drawing4×Ø5.1 through. The supplied STEP represents the holes asØ5.0; use the drawing for clearance-hole acceptance. Body length122±1 mm; original CAD rear envelope125 mm includes projecting details. Original CAD also includes cables and connectors outside the motor body.
- Drawing detail: shaftØ10,0/−0.012mm; **D-flat9±0.1mm across the shaft**,15±0.2mm long. RegisterØ38.1±0.05mm; flange7mm; grid50±0.25mm; frame60.3mm maximum. The original STEP's flat lies at nativeY4 fromZ6 to21. **There is no removable feed-motor key.** [Planar-face extraction](../generated/vendor-feed/vendor-planes.csv) confirms this independently. The original motor is used unchanged in feed CAD.
- Original image-based [feed torque PDF](24HS40-5004D-E1000-torque.pdf), SHA256 `d878afab32e23e6936930360e27ea744709eb79b6ecc9ec4e6105cb67f5afbbc`, was captured from the manufacturer's5616 document endpoint. It printsCL57T-V41,36V,RMS4A,2000microsteps. The [cutting-motor torque PDF](34E1K-85-torque.pdf), SHA256 `10887ae109229a7432397a47840e1f10c40bef28b588f6cc60e53b176cbd288c`, was captured from endpoint1087. Raster graphs are not vendor-tabulated numeric data.
- Rated holding torque4 Nm,5 A/phase;0.60 Ω±10%;2.60 mH±20%; rotor inertia0.000108 kg·m². Encoder1000 PPR/4000 quadrature counts. Use the36 V/4 A RMS/2000-pulse torque curve for running torque, not holding torque.
- **Initial drive settings:** S1=4 (5.6 A peak/4 A RMS, Ki0/Kp25/Kp25); SW1–4=OFF,ON,ON,OFF (2000 pulses/revolution); SW5=ON; SW6=OFF; SW7=OFF; **SW8=OFF,1.5 ms filter**; S3=24 V. Both feed drives use the same filter setting. A commanded endpoint must settle and be verified by measuring-wheel feedback; the filter is not a zero-delay position response.
- Actual manual operating range18–50 VDC;0–40°C; logic7–16 mA; minimum pulse1 µs, direction setup2 µs. Vertical mounting and≥30 mm drive spacing.
- Observed standalone motor merchandise priceUSD36.87. Drive and supply USD prices require their current product records; no price is inferred from another kit.

## TR-02 — selected feed rollers

- Eight **MISUMI UMCL40-10-20-MC** rollers: four driven lower and four freely rotating upper rollers, each upper roller fixed to its own bearing-supported rotating shaft. No roller runs directly as an aluminium-on-steel plain bearing.
- [Confirmed manufacturer configuration](https://uk.misumi-ec.com/vona2/detail/110300423260/?HissuCode=UMCL40-10-20-MC); [original dimensional catalogue](misumi-collar-rollers.pdf).
- Outer diameter40 mm, bore10 mm, length20 mm; aluminium5000-series core; bonded natural-colour polyurethane **Shore A50**. MC adds two radial M3 tapped holes90° apart. The configured part was confirmed by the manufacturer's configurator, not constructed from an unverified name.
- The [manufacturer section drawing](misumi-umcl-drawing.png) identifies **dH7**,OD±0.1mm andL±0.2mm. Table thickness **t4 is the radial aluminium collar thickness**, giving an18mm collar OD with the10mm bore; it is not4mm polyurethane thickness. Do not model a32mm core. F110 adds a3.04mm-wide,2.35mm-deep collar keyway opposite the MC holes; this is explicitly a STARZ secondary operation, not a standard MISUMI supplied keyway. The3×4×18mm positive key carries torque; theM3 screws receive no torque-transmission credit.
- The archived EU catalogue instead prints bore+0.01/+0.03. The selected incoming acceptance is10.000–10.030mm, covering both published callouts; the10h6shaft leaves at most0.039mm diametral clearance. F110keyway depth is measured from the actual accepted bore,not from an assumedH7lower limit.
- Manufacturer page displayedEUR19.50 excluding VAT for1–9 pieces. This is retained as source-currency evidence, **not labelled USD**. The machine cost table requires a dated USD conversion or USD quotation. Frankfurt dispatch does not establish Tunisia freight.
- Roller speed at0.50 m/s is238.73 rpm before loaded-radius correction. Roller position is adjusted with wire OD to keep the112 mm wire datum fixed; rubber compression and slip are controlled by the measuring wheels, not an assumed exact40 mm rolling diameter.

## ME-02 — selected measurement encoder and shaft bearings

- Two **Autonics E40S6-1024-6-L-5**:Ø6 shaft,1024 PPR, differentialA/B/Z,5 V±5%,300 kHz,5000 rpm, starting torque≤0.00392 Nm,IP50.
- [Manufacturer model](https://www.autonics.com/us/model/E40S6-1024-6-L-5); [local family drawing](autonics-e40-encoder.pdf), printed pageF-20. From the mounting face: body31 mm rearward; pilot5 mm forward; shaft15 mm beyond the pilot, so **tip20 mm from the mounting face**, not15. ShaftØ6−0.010/−0.015 mm =5.985–5.990 mm. Single D-flat5 mm across over the last10 mm; not opposing flats. PilotØ20,0/−0.02. Front pattern3×M3×0.5 onPCD30,120° apart,5 mm thread depth; bodyØ40.
- SuppliedØ6 coupling:Ø15×22 mm; bore6.0–6.1 mm;4×M3×0.5 radial screws,centres2.8 mm from ends; maximum misalignments0.25 mm parallel,5° angular,0.5 mm axial,not simultaneous installation targets. Its effective hub lengths, screw torque and internal flexure transitions are not dimensioned. The CAD is an explicitly labelled interface envelope,not a machined solid coupling or original supplier STEP.
- The product's public CAD button opens [Autonics' shared Autodesk viewer](https://autonics5.autodesk360.com/g/shares/SH30dd5QT870c25f12fceace0012a2039885). Its title identifies800/75PPR,24V family models,not the selected1024PPR,5V model. It is not relabelled as exact selected-part CAD. No account, vendor form or purchase was used.
- Four **SKF626-2Z**, normal clearance, support the two measuring-wheel shafts independently of the encoders. [SKF original specification archived through RS](skf-626-2z.pdf):6×19×6 mm,C2340 N,C0=950 N,Pu40 N;8.8 g; corner radius≤0.3 mm; shaft abutmentØ8.4–11 mm; housing abutment≤Ø16.6 mm.
- **Current manufacturer product table, checked2026-09-12:** [SKF626-2Z](https://www.skf.com/group/products/rolling-bearings/ball-bearings/deep-groove-ball-bearings/productid-626-2Z) specifies bore deviation−0.007/0 mm, OD−0.008/0 mm, width−0.060/0 mm, **P6 dimensions with tighter width**, and **P5 geometrical tolerances**. Inner assembled radial runout4 µm, inner side-face/bore runout7 µm, inner axial runout7 µm; outer radial6 µm, outside perpendicularity4 µm, outer axial8 µm. Current mass8.5 g and minimum-load factor0.03 supersede the2020 archive's8.8 g/0.025 for new calculations. Load ratings and6×19×6 dimensions agree. These class/size limits are actual product data, not generic Normal-bearing limits.
- The current SKF CAD panel explicitly requires login; no account or login was attempted. Its Download PDF control did not produce a downloadable document in this browser attempt; the new numeric table is transcribed below, not presented as a newly archived PDF.
- The encoder's suppliedØ6 coupling transmits only measuring torque. The independent bearings carry wheel spring force; it is not applied to an unsupported encoder shaft.
- CN minimum2 µm leaves0.5 µm under the SKF10 K ring-temperature equation **before** any fit reduction. Full js5 interference must not be accepted without an actual measuring-bearing matching/thermal design. The [measurement report](../generated/measurement-analysis/report.md) exposes this conditional clearance failure rather than approving it.
- [Transcribed numeric interfaces](measurement-interfaces.csv), [catalogue-interface STEP files](../generated/measurement/manifest.csv), [M201 encoder plate drawing](../generated/drawings/M201-R1-encoder-plate.pdf) and [measurement assembly/calibration record](../../docs/13-measuring-interfaces-and-calibration.md) are the current measurement detail. They do not yet constitute the shaft/pressure/guide assembly.
- **Commercial check2026-09-12:** the [DigiKey exact-part search](https://www.digikey.com/en/products?keywords=E40S6-1024-6-L-5), after normal page loading, displayed **0 results** for `E40S6-1024-6-L-5` and suggested the different `E40S6-1024-3-N-5`. That NPN-output part is **not a substitute**. No current price, stock or lead time was observed for the selected line-driver model. The historical `$48.50`, `1864-1042-ND`, `3 days` claims remain withdrawn; reading them from the old BOM is not verification. An unsuccessful search also does not establish global unavailability or an inability to special-order.

## TH-01 — selected electrical-cabinet cooler

- **Rittal SK3303.500**,230 V,1~,50 Hz,e-Comfort controller. [Manufacturer specification](https://www.rittal.com/uk-en/products/PG20231215KLI101/PG20231215KLI103/PRO136367?variantId=3303500), [original installation manual](rittal-3303500-manual.pdf), [family mounting drawings](rittal-3303500-technical.pdf).
- Cooling520 W atL35/L35 and **280 W atL35/L50**,50 Hz. Use280 W for the conservative cabinet heat-balance calculation, not520 W at every outdoor temperature.
- Product dimensional table280×550×210 mm;18.05 kg. The short description on the same page incorrectly repeats140 mm depth. The installation drawing governs the complete mounting envelope and cut-outs.
- Maximum current2.6 A, starting5.1 A; manufacturer pre-fuse10 A. External/internal free airflow315/265 m³/h. External circuitIP34, internalIP54. Operating10–50°C. R-513A charge0.17 kg; do not modify the sealed refrigerant circuit.
- Factory setpoint35°C, switching hysteresis5 K. The manual warns against changing hysteresis without consultation and may increase it after short cycles. **Do not assume the cabinet stays below40°C solely because the display is set to35°C.** Final selection requires heat load, airflow distribution and overtemperature interlock settings in the cabinet design.

## TR-03 — selected belt, coupling, bearings and retention

SKF'scurrent[generalbearing specifications](https://www.skf.com/group/products/rolling-bearings/ball-bearings/deep-groove-ball-bearings/general-bearing-specifications),metricaccordion,table4:for10&lt;d≤18mm,C3radialclearance **11–25µm**,CN3–18µm. For626d6,CN2–13µm. [Selectednumericrows](skf-dgbb-clearance-selected.csv). Valuesare **unmounted,zero measuringload**,not installedclearance. Thetableopensas a modal;itsstandalonecatalogue-tableURLreturned404.

SKF[seattolerances](https://www.skf.com/group/products/rolling-bearings/principles-of-rolling-bearing-selection/bearing-selection-process/bearing-interfaces/seat-tolerances-for-standard-conditions),table1modal:rotatinginnerring,0.05C&lt;P≤0.1C,10&lt;d≤17mm recommends **j5,Ra0.4µm,radialrunoutIT4/2,axialrunoutIT4**. [Selectedfitrows](skf-selected-shaft-fit.csv). The current6001/6002reactionratios areinthisrange;generic h6 isnot theappropriatefit recommendation (theh6row is forstationaryinner-ringload). **R4replaces onlythebearingseatswithmatchedj5**,3–6µmactualinterference;coupling/rollerfitsremainseparate. SKF[tolerancesandresultantfits](https://www.skf.com/group/products/rolling-bearings/principles-of-rolling-bearing-selection/bearing-selection-process/bearing-interfaces/tolerances-and-resultant-fits),table2modal,givesj5−3/+5µmfor10&lt;d≤18. UseactualP6borelimits−7/0,notthatgeneraltable'sNormal−8/0.

[Fitcalculation](../generated/bearing-fit/report.md):256corners,minresidual1.28µm,minouter-ring/housingclearance2.432µm. Thelimitsareconditionalon theexplicittemperature envelope;thermalfieldnotyetverified. SKF[fitreduction](https://www.skf.com/group/products/rolling-bearings/principles-of-rolling-bearing-selection/bearing-selection-process/bearing-execution/selecting-internal-clearance-or-preload/initial-internal-clearance/reduction-interference-fits) isΔ1f1+Δ2f2;themodelusesfactor1.0bound andchecksfreealuminiumhousing separately. SKF[temperaturereduction](https://www.skf.com/group/products/rolling-bearings/principles-of-rolling-bearing-selection/bearing-selection-process/bearing-execution/selecting-internal-clearance-or-preload/initial-internal-clearance/reduction-temp-difference) gives0.012ΔT(d+D)/2µm. Cold-starttransients,notonlysteadystate,mustsatisfythedeclaredlimits.

R3rollerlayout clarification:the original[collar-roller catalogue](misumi-collar-rollers.pdf) identifiesENAW-5052equivalentcollar material andtheMCurethaneaccessclearance **M+1**. ForM3,theouterurethanehole isØ4,not athread throughrubber. TheS108.75wirelane is5.25mmawayfrommidwidth andis boundedagainst theaccessholes. TheR3shaftloads use thisactualcontactstation. NipsmovedtoX75.5/138.0nominal to retaininsulatedgrip on200mmparts during25mmtailbackfeed. These aredesignintegrationchanges,not a substitutedrollerSKU.

- Four **HTPA20S5M250-A-N14**, confirmed in the [manufacturer configurator](https://uk.misumi-ec.com/vona2/detail/110300406030/?HissuCode=HTPA20S5M250-A-N14):20teeth,S5M5mm pitch,25mm belt,TypeA,32mm overall width,27mm nominal usable width,PD31.83mm,tooth OD30.87mm,flange OD36mm,14H7bore. New-JIS keyway5JS9±0.015mm,2.3mm radial depth0/+0.1mm; twoM5×4set screws included. Source merchandise priceEUR39.87 exVAT,1–14pieces. [Catalogue](misumi-s5m-pulleys.pdf). The earlier10mm and12mm bore configurations are not the purchase selection.
- Two **HTBN225S5M-250**, [confirmed record](https://uk.misumi-ec.com/vona2/detail/110302566230/?HissuCode=HTBN225S5M-250):225mm pitch length,45teeth,25mm width,chloroprene/fibreglass/nylon construction. Source priceEUR25.24 exVAT,1–4pieces. Equal20tooth pulleys give62.5mm nominal centres. Required adjustment52.5–64.5mm, not a fixed62.5mm hole pair.
- MISUMI [Table39 power data](misumi-belt-power-2263.pdf):S5M20teeth at10mm reference width.25mm width factor2.84;ten teeth engaged givesKm1.0. [Installation Table32](misumi-belt-installation.pdf):recommended166N initial tension,maximum221N,Y151N. STARZ setting190N,acceptance180–200N;at62.5mm span,1mm deflection force13.8715–15.1215N. Belt pitch-length tolerance±0.41mm and width±0.6mm. [Selection tables](misumi-belt-selection-3.pdf) and [installation drawing](misumi-belt-installation.png) are retained.
- Two **R+W MK2/100/50/10/12**, the same manufacturer-configured10Nm,40OD,50mm free-length,16mm-hub family already documented for the head. Motor D-flat remains unmodified;4.5Nm hub screws. The nominal motor face−35.2mm and coupling−33…17mm chain is relative to the custom shaft rear end0mm. Assembly sets the actual free coupling without axial strain. No unsupported claim of a separate D-flat torque rating is made; the assembly proof record checks slipping independently.
- Eight each **SKF6001-2Z/C3** and **6002-2Z/C3**, confirmed manufacturer products.6001:12×28×8mm,C5400N,C0=2360N,0.0217kg.6002:15×32×9mm,C5850N,C0=2850N,0.031kg. C3 is the selected clearance, notCN. The manufacturer6001/6002 basic dimensional records specify bore deviation−0.007/0mm,width−0.060/0mm;6001OD−0.008/0,6002OD−0.009/0.6001shaft abutment14–16.9mm,housing≤26mm;6002shaft17–20.4mm,housing≤30mm;both fillet≤0.3mm. The catalogue runout classes are not completed-machine runout measurements.
- Eight **SKF KM2**:M15×1,25OD,5mm width,21mm locating face,4×2mm spanner slots,14.6kN axial static rating;useHN2-3spanner. Eight **MB2** new lock washers:15bore,21side face,28OD,1mm thickness,4mm inner tab,13.5mm inside dimension under tab. Turn one outer tab into a nut slot;replace the washer on every reinstallation. The inner tang uses the shaft's4.2mm-wide slot.
- Eight **SEEGER A12**,spring steel,DIN471. [Original manufacturer's2022 catalogue](seeger-catalogue-2022.pdf),PDF pages24–25:1.00mm ring thickness−0.06;groove11.50mm−0.11;groove width minimum1.10;minimum edge margin0.8mm;groove capacity1500N,ring capacity5000N before application factors;ZGA-1pliers. STARZ groove11.45±0.02mm,1.12±0.02mm width. These dimensions do not come from an unverified generic ring image.
- Same catalogue, PDF page131 / printed page129, section8.1.3: a rectangular groove's loaded-side root may have radius up to0.1×ring thickness without noticeably changing seating. R5selectsR0.04±0.01: maximum0.05mm is below0.094mm for the thinnest specifiedA12. Its minimum1.10mm groove retains1.00mm flat width between both maximum-radius tangencies. This manufacturer geometry guidance does not supply a fatigue notch factor for the custom12mm42CrMo4shaft.
- [Dated ECB/Frankfurter conversion](eur-usd-2026-09-11.json):**1EUR=1.1592USD,2026-09-11**. Active BOM conversions retain the exact source amount and date. They are not USD vendor quotations or Tunisia-delivered prices. Freight remains separate; tax/duty is budgetedzero solely under the user's stated sous-douane arrangement.

## MT-01 — F101 blind-hole cutting tap

- **Dormer E412M5**, material ID7441118, SAP86033653: the [actual manufacturer product](https://dormerpramet.com/p/7441118), checked2026-09-12, specifies M5×0.8-6H,DIN371,HSS-E-PM/TiAlN-Top,3flutes,48° spiral,formC2–3pitch lead and **3D blind-hole application**. ShankØ6,square4.9×8,OAL70,LU25,THL8,PHD4.2 mm. The8 mm THL is active cutting length,not the maximum thread depth. [Transcribed values](dormer-e412m5-interfaces.csv).
- Manufacturer P3 alloy-steel starting speeds:24m/min below180HB,19m/min at180–260HB,16m/min at260–360HB. The shaft's minimum650MPa yield does not establish its actual hardness; these are not approved machine-cycle settings.
- [Linked family dimension image](dormer-e412-family-dim.png) shows a male centre but does not dimension its projection. Do not treat the2.4 mm cutting lead as the complete physical nose. [STARZ operation record](../../docs/14-f101-tapping-operation.md) requires measured tip-to-full-form distance≤3.18 mm,including measurement uncertainty,to leave≥0.50 mm cylindrical bottom clearance. This is a design/tool-inspection requirement,not an invented Dormer tolerance or a completed measurement.
- [MRO Supply SKU6081909](https://www.mrosupply.com/cutting-tools/6081909_e412m5_dormer-pramet/) displayedUSD40.55each;2tools=USD81.10merchandise. Dispatch estimate1day is not Tunisia delivery/stock confirmation;free-shipping banner is not Tunisia freight. Its structured table misassigns several fields: use the manufacturer's4.2 mm pre-drill and6 mm shank,not the retailer's48/4.9 entries. No account/contact/purchase was used.
- The retailer-linked [2020 unified catalogue](dormer-unified-catalogue.pdf),657pages,does not contain E412/7441118. It is an archived research source,not selected-tool dimensional authority. Manufacturer URLs use numeric IDs; failure of a guessed `/p/E412M5` URL does not establish that the product is unavailable.
- **Rejected E003M5**, manufacturer [ID5974394](https://dormerpramet.com/p/5974394):2.5D=12.5 mm blind application is too short for the F10114.27 mm maximum full-thread-end coordinate. It is not a BOM alternative.

## MC-01 — selected pressure-carrier and measuring-arm hardware

Checked 2026-09-13 on MISUMI UK manufacturer-direct records (EUR merchandise, ex-VAT, Frankfurt warehouse). Direct URL guesses returned 404; each part was reached from the keyword search `https://uk.misumi-ec.com/vona2/result/?Keyword=<part number>` and confirmed on the configured detail page. Values are transcribed catalogue figures, not measurements on assembled parts.

- Four **LHFRW12** flanged double-length bushings (two per carrier plate F202): shaft Ø12, outer Ø21 (0/−0.016), length 57, flange Ø42 × 6, four Ø4.5 holes with Ø7.5 × 4.1 counterbores on PCD 32, basic dynamic load 657 N, static 1200 N, allowable moment 10.9 N·m, 110 g. EUR 14.58, 1 day. The carrier design uses the flange face up on F202 and the body hanging into the Ø21.01 clearance bore; the 4 × M4 screws FB206 are on the 45° positions of PCD 32.
- Four **PSFJ12-240** hardened linear shafts (posts PC01/PC02 per carrier): Ø12 g6, L 240 (configurable 15–1200 in 1 mm), EN 1.3505 (100Cr6) 58 HRC, hard chrome, perpendicularity 0.2. EUR 11.70, 4 days. The earlier PSFJ12-150 (EUR 6.29) is superseded by the 240 mm length required to clear the posed feed-motor cable and carry the bridge F203.
- Two **SWU17-60** compression springs (SP01, one per carrier at X 108, Y 0): outer Ø17, inner Ø10.5, free length 60, rate 4.09 N/mm, allowable load 147.1 N, allowable deflection 36, 14 g. EUR 3.53, 6 days. Nominal carrier setting delivers 2 × 60 N nip preload with the CAD-estimated carried mass; the adjuster range is ≈34–140 N total and is checked in code, not guaranteed on hardware.
- One **SWU10.5-35** compression spring (SP02, measuring arm at X −241): outer Ø10.5, inner Ø6.0, free length 35, rate 3.27 N/mm, allowable load 68.6 N, allowable deflection 21, 3 g. EUR 1.80, 6 days. Arm lever 25/60 gives 10 N wheel contact force from 24 N spring force at the nominal adjuster setting.

Prices are merchandise only; Tunisia freight, lead time and duty were not quoted, and no purchase or contact was made.

## Original supplier CAD integrity

All five imported original STEP files have valid solid topology. [Geometry extraction](../generated/vendor/vendor-geometry.csv) records actual native bounds and component solids; [cylinder extraction](../generated/vendor/vendor-cylinders.csv) records interface surfaces. Motor bounds include supplied cable/connector shapes and are not body dimensions or cable bend-radius specifications. Geometric centroids are not multi-material motor mass properties.

## Rejected interfaces — not alternative designs

- Linked NEMA23 kit1-CL57Y-S20-V20 actually contains23HS30-5004D-E1000 andCL57Y-V20; it is not the motor/driver previously named in the BOM.
- The old genericSFU1204 stage does not have an identified assembled-stage drawing or complete thrust/moment ratings.
- KK60 has only3120 N maximum fixed-bearing axial load and152 Nm pitch/yaw static moment for one standard block. It is not treated as interchangeable with the selected KK86.
- CL57T-V41 is **18–50 VDC according to its original revision4.1 manual**, not80 V;24/36/48 V are nominal supply choices.
- Mean WellUHP-500-55 adjusts45–58 V per its actual [datasheet](UHP-500-spec.pdf), not60 V. It is not the selected60 V supply.
- R+WBK2/15/59 has49 mm outside diameter per the [manufacturer sheet](rw-bk2-coupling.pdf); do not put it inside the46 mm KK86 coupling cavity.
- JAKOBKSD10 was screened from its [manufacturer drawing](jakob-ksd-coupling.pdf), but `(e−g)/2 = (45−7)/2 = 19 mm` bore length exceeds the18 mm exposed stage shaft. It is **not selected**; no reduced-engagement torque rating is invented.
- The old [HIWIN overview sheet](hiwin-single-axis-catalog.pdf) is an oversized drawing sheet, not a substitute for the complete catalogue and current product record.

## CT-01 — controls and cabinet design inputs

The following records were checked 2026-09-13. They establish catalogue facts for selected components only. They do **not** establish a completed electrical design, an assembled cabinet, a safety-function performance level, regulatory compliance, or permission to energise the machine. The current logical design input is [controls and wiring](../../docs/22-controls-and-wiring.md).

### XPSUAF13AP safety relay

- [Schneider XPSUAF13AP product record](https://www.se.com/ww/en/product/XPSUAF13AP/), [manufacturer-generated product datasheet](../../A700000007186749.pdf), and [Schneider XPSUAF instruction manual EIO0000003465](../../EIO0000003465.01.pdf).
- The device accepts a 24 VAC/DC supply at A1/A2. B2 is the common 0 V reference for 24 V signal circuits; S11/S21 are test/control outputs and S12/S22 are the two safety inputs. Y1/Y2 are the start test/control output and start input. 13-14, 23-24 and 33-34 are three NO safety outputs. Z1 is a **non-safety diagnostic output** and must not be used as a safety signal.
- Application-selector position 1 is documented for monitoring E-stop circuits and electrical guards through S11-S12 and S21-S22 with input dynamisation/signal-interlock monitoring. Start-selector position 3 is a monitored falling-edge start: it requires an at-least-80 ms pulse, starts on the falling edge, and accepts no start pulse until 2500 ms after power-on.
- The published limits include 6 A continuous maximum per output contact, 12 A total thermal current, DC13 3 A at 24 V and a specified external 10 A gG fuse. Maximum stated response to deactivation of a safety input is 20 ms; maximum activation delay is 100 ms.
- The product states it can reach PL e/category 4 and SIL/SILCL 3 under applicable conditions. That is a component capability, **not** a certification or achieved performance level of this machine. The wiring, diagnostics, DC power removal, stop time, fault exclusion and validation evidence remain open.

### Guard-lock device and actuator

- Commercial records for [SLC-F-024-20/20-R4, article 6018200007](https://www.automation24.com/mechanical-switch-with-guard-locking-bernstein-slc-f-024-20-20-r4-6018200007) and [ACF-1, article 3911742391](https://www.automation24.com/actuator-bernstein-acf-1-3911742391). Direct pages observed 2026-09-13 list the SLC external dimensions as 30 × 192 × 44 mm and the ACF-1 dimensions (W × H × D) as 60 × 40 × 52.6 mm; the ACF-1 page also states a 400 mm minimum actuation radius. These are bounding-envelope inputs for [preliminary guard/payoff CAD](../../docs/25-preliminary-guard-and-payoff.md), not manufacturer mounting drawings or a guard approval.
- The selected SLC is a 24 VAC/DC, spring-force locking device: power loss leaves the guard locked. E1/E2 are its locking-magnet terminals, rated 6.7 VA/W at 100% duty. The actuator-detection NC contacts 11-12 and 21-22 are closed whenever the actuator is inserted; guard-lock NC contacts 31-32 and 41-42 are closed only when the actuator is inserted **and locked**. The contacts have direct-opening action, a 5 A thermal rating and specified 4 A gG short-circuit protection.
- The ACF-1 is compatible with the selected lock and has a stated 400 mm minimum actuation radius. Mechanical fit, guard retention, escape/release behaviour and realistic operating use require physical review.
- No authorized E1/E2 release circuit exists. The cabinet record intentionally leaves guard release unresolved until stop-time, contactor opening, DC-bus decay, regenerated-energy and risk-assessment evidence exist.

### Proposed DC power contactors

- [Schneider TeSys Deca LC1D12BD product record](https://www.se.com/ww/en/product/LC1D12BD/) and [manufacturer product datasheet download](https://www.se.com/us/en/product/download-pdf/LC1D12BD?filename=Schneider%20Electric_TeSys-Deca-Contactors_LC1D12BD.pdf).
- Main poles are 1/L1-2/T1, 3/L2-4/T2 and 5/L3-6/T3. A1/A2 are a 24 VDC, 5.4 W maximum coil with integral suppression. Auxiliaries are 13-14 NO and mirror-certified 21-22 NC. The product material lists a power-circuit voltage up to 300 VDC.
- The documented design concept assigns K1/K2/K3 to the two 60 V and one 36 V motion supplies and proposes three main poles in series in each positive DC branch. That is **not a released DC switching application**: no authoritative manufacturer utilisation/current proof has been located for the exact opening duty, and no regeneration, discharge or short-circuit coordination design has been completed.

### XPS output fuse link

- [Schneider DF2CN2 product record](https://www.se.com/ww/en/product/DF2CN2/) (identity corrected from DF2CN10 on 2026-09-15, see below).
- DF2CN2 is a Schneider NFC/Tesys GS cylindrical fuse **link**, not a fuse holder: 10 × 38 mm, gG, 500 VAC, 2 A, without striker. A holder and its mounting, fault-duty and coordination evidence are deliberately unselected.
- **2026-09-15 correction:** the archived XPSUAF13AP datasheet rates the relay safety outputs 5 A DC-1 / 3 A DC-13 at 24 V DC, and the XPS output load is the two LC1D12BD coils (≈0.45 A). A 10 A gG link could not protect 5 A contacts between 5–10 A; DF2CN2 (2 A gG) is the corrected selection recorded in [docs/22 §11.3](../../docs/22-controls-and-wiring.md).
- `C10G` is an Eaton Bussmann low-voltage cylindrical-fuse series. It must not be attributed to Mersen, and it is not the selected Schneider fuse link.

### IF-B01 logical source and TPL COM rail

- The editable [IF-B01 schematic](../../hardware/if-b01/if-b01.kicad_sch),
	[placed/routed PCB](../../hardware/if-b01/if-b01.kicad_pcb),
	[native intent manifest](../../hardware/if-b01/if-b01-native-intent.json),
	[source-review record](../../hardware/if-b01/review/if-b01-native-source-review.md)
	and [draft output package](../../hardware/if-b01/manufacturing/draft-r2-2026-09-13)
	are generated/checked with local KiCad CLI 10.0.6. ERC, native export parity,
	physical DRC and schematic parity currently report zero issues. These source
	checks constrain transcription and layout consistency only; they are not an
	electrical design review, fabrication release, fixture evidence or machine
	qualification result.
- The [TI TPL7407L product record](https://www.ti.com/product/TPL7407L) links
	the selected driver's datasheet. Its documented COM operating recommendation
	is 8.5–40 V, COM is pin 9 and GND is pin 8. U2/U3 therefore use the protected
	nominal 24 V `+24V_TPL_COM` rail through F1, not the 3.3 V logic rail.
- TI documents COM bypassing for fast COM slew/hot-plug conditions; the selected
	C2/C3 input parts are `GRM188R71H104KA93D`, 100 nF / 50 V X7R, from
	`+24V_TPL_COM` to `0V_PELV` at U2/U3. C1 remains the separate 100 nF / 16 V
	3.3 V U1 decoupler. The 50 V substitution removes the prior 16 V nominal-rail
	rating mismatch; it does not establish surge, cable, branch-load, inrush or
	COM-slew performance on physical hardware.

No exact terminal blocks, PE bar, enclosure, glands, cable, ferrules, branch fuses/holders, mains protection or EMC hardware are selected by this evidence record. Those omissions are explicit release holds, not permission to substitute generic parts.

## Live public-page price observations 2026-09-13 (merchandise, not delivered cost)

Direct public-page observations recorded 2026-09-13; all are merchandise prices
without freight, duty, tax or destination terms and are not quotations.

| Item | Page observed | Observed price | Basis |
|---|---|---|---|
| Schneider TeSys Deca LC1D12BD | [Automation24 product page](https://www.automation24.com/main-contactor-schneider-electric-tesys-lc1d12bd) | EUR 70.40 ex VAT plus dispatch; page RRP 108.00 | In stock; dispatch next working day |
| SKF 6001-2Z/C3 | [Simply Bearings product page](https://www.simplybearings.co.uk/products/6001-2z-c3-skf) | GBP 6.32 | Allow 1-2 days per page |
| SKF 6002-2Z/C3 | [Simply Bearings product page](https://www.simplybearings.co.uk/products/6002-2z-c3-skf) | GBP 6.91 | Allow 1-2 days per page |
| SKF 626-2Z | [Simply Bearings product page](https://www.simplybearings.co.uk/products/626-2z-skf) | GBP 4.49 | In-stock listing |
| SKF 626-2Z/C3 | [Simply Bearings product page](https://www.simplybearings.co.uk/products/626-2z-c3-skf) | GBP 4.09 | Comparison observation only; the selected ME-B01 clearance remains CN |
| SKF KM 2 | [Simply Bearings product page](https://www.simplybearings.co.uk/products/km2-skf) | GBP 7.90 | In-stock listing |
| SKF MB 2 | [Simply Bearings product page](https://www.simplybearings.co.uk/products/mb2-skf) | GBP 2.15 | In-stock listing |

### Quality-equivalent brand alternatives observed on the same page family

These are comparisons recorded on the same Simply Bearings search results on
2026-09-13. They are **not substitutions**: any change of brand requires an
approved change record and repeat of every affected check.

| Selected | Alternative observed | GBP | Relation to selected |
|---|---|---|---|
| SKF 6001-2Z/C3 GBP 6.32 | FAG 6001-C-2Z-C3 [page](https://www.simplybearings.co.uk/products/6001-c-2z-c3-2z-c3-fag) | 4.90 | Same size/clearance/shielded deep-groove class; Schaeffler brand |
| SKF 6001-2Z/C3 GBP 6.32 | KOYO 6001-2Z-C3 [page](https://www.simplybearings.co.uk/products/6001-2z-c3-koy) | 4.69 | Same size/clearance/shielded deep-groove class; JTEKT brand |
| SKF 6002-2Z/C3 GBP 6.91 | FAG 6002-C-2Z-C3 [page](https://www.simplybearings.co.uk/products/6002-c-2z-c3-2z-c3-fag) | 5.05 | Same class |
| SKF 6002-2Z/C3 GBP 6.91 | KOYO 6002-2Z-C3 [page](https://www.simplybearings.co.uk/products/6002-2z-c3-koy) | 4.84 | Same class |
| SKF 626-2Z GBP 4.49 | FAG 626-2Z (CN) [page](https://www.simplybearings.co.uk/products/626-2z-fag) | 4.72 | Not lower; listed for completeness |

The FAG and KOYO entries are peer-manufacturer parts with the same dimensional
and clearance designations; equivalence of tolerances, clearance classes and
grain of supply is asserted by the manufacturer designations, not by an
independent physical comparison. Budget/Codex/ZEN rows on the same results
were observed but are not presented as quality equivalents.

### Payoff spindle bearing observation

For the payoff-spindle design input only (not yet selected in a BOM):
SKF 6202-2Z/C3 [Simply Bearings page](https://www.simplybearings.co.uk/products/6202-2z-c3-skf)
GBP 8.88; FAG 6202-C-2Z-C3 [page](https://www.simplybearings.co.uk/products/6202-c-2z-c3-2z-c3-fag)
GBP 4.98. Both observed 2026-09-13.

### Observed absence, 2026-09-13

- HIWIN publishes no online retail price for KK8610P340A1F0/article 10.00026;
  hiwinonline.com resolves to a parked placeholder. A distributor quotation is
  the only observed-price path.
- Automation24's incremental-encoder catalogue was searched and does not list
  the selected Autonics E40S6-1024-6-L-5; its public price remains unobserved.
- Reichelt (de/en) returns zero results for a 10 × 38 mm fuse holder and for
  Wago 282-364; no Reichelt identity is selected.
- omc-stepperonline.com, digikey.com, uk.rs-online.com, mouser.com,
  tme.eu and se.com blocked automated retrieval on 2026-09-13; a blocked or
  rate-limited page is not evidence of price or availability either way.
  The 2026-09-13 Simply Bearings and Automation24 observations above were
  read directly from live pages; earlier automation24/DigiKey/MISUMI records
  in this repository retain their original dates and remain separate.

### Observed absence and additions, 2026-09-16

- Farnell UK search (live, 2026-09-16) returns "couldn't find any products
  matching KK8610P340A1F0"; subsequent automated Farnell queries were
  rate-limited. Alibaba, eBay, TME, RS and Mouser block automated retrieval;
  no price from those channels is claimed.
- **Made-in-China search (live, 2026-09-16): no listing exists for the
  genuine HIWIN KK8610P340A1F0.** The only 340 mm-stroke KK86-class listing
  is a clone, HZMotion TKK8610C-340A1F0
  ([page](https://hzmotion.en.made-in-china.com/product/BpSrFNDcGvhK/China-Hzmotion-Original-Factory-Tkk8610c-340A1f0-Tkk-Low-Profile-Aluminum-Base-Kk-Type-Linear-Actuator-CNC-Machine-Linear-Motion.html))
  at US$800–2,500 FOB (a non-committal range), plus 940 mm-stroke clones
  (Canmeng KK86D20C/D10C, US$50–200). Clones are not selected: they are a
  different precision class than the genuine HIWIN axis and require a
  change record plus a full repeat of every affected check if ever
  considered. The genuine axis stays on the distributor-quotation route.
- **Circlip observation:** Simply Bearings lists a quality-equivalent
  DIN471-class 12 mm external circlip, code `d1400-012`
  ([page](https://www.simplybearings.co.uk/products/d1400-012)), GBP 0.85,
  observed 2026-09-16. Free/un-sprung diameter 11 mm; the selected TR-RR01
  remains the SEEGER A12 identity — using this part requires an approved
  change record and the groove-fit check of the audit record.
- **Fuse-link family observation (2026-09-16):** Conrad lists the whole
  Schneider 10×38 range: DF2CN02 (2 A) EUR 55.99/10, the DF2CA family
  (EUR 55.99–63.99/10) and the DF2BA family, including DF2BA0200 (2 A)
  at EUR 49.99/10
  ([page](https://www.conrad.com/en/p/schneider-electric-df2ba0200-fuse-10-pc-s-2935281.html))
  — a 10 % cheaper same-class 2 A 10×38 link alternative. The DF2FA family
  (22×58 links, EUR 154.99–214.99/10) is a different size class. The
  DIN-rail fuse BASE for 10×38 links is not listed at Conrad; its exact
  identity is to be confirmed from the current Schneider GS/DF2 accessory
  catalogue.
- **Kübler observations (2026-09-16, Conrad):** (a) Kübler measuring-wheel
  accessory "Perambulator" 05.2400.0040.1000.5045
  ([page](https://www.conrad.com/en/p/kubler-group-perambulator-1-pc-s-05-2400-0040-1000-5045-125696.html))
  EUR 395.00 incl VAT, "available soon" — a different wheel to the
  selected 8.0000.3123.0006, showing the Kübler wheel family price class.
  (b) Kübler beam couplings for rotary encoders, e.g. 8.0000.1102.1008
  (Ø10/Ø8)
  ([page](https://www.conrad.com/en/p/kubler-group-8-0000-1102-1008-beam-coupling-rotary-encoder-1102-1-pc-s-198995.html))
  EUR 43.99 incl VAT, in stock — encoder-coupling class reference; these
  are not power couplings and do not substitute the R+W MK2 lines.
- **Priced encoder alternatives (observed 2026-09-16, automation24):**
  the selected Autonics E40S6-1024-6-L-5 has no observed public price.
  Automation24 lists two in-stock ifm magnetic 1024 PPR Ø6 mm shaft
  alternatives: RU3100
  ([page](https://www.automation24.com/incremental-encoder-ifm-electronic-ru3100))
  EUR 185.50 ex VAT (RRP 206.10, −10%), M12 5-pole; RUP500 with synchro
  flange and display
  ([page](https://www.automation24.com/incremental-encoder-ifm-electronic-rup500))
  EUR 255.50 ex VAT (RRP 283.90, −10%), item 102715, M12 8-pole. Both are
  HTL/TTL 50 mA with factory setting 1024. Substitution requires an
  approved change record plus the M201 flange/pilot and IF-B01 input-level
  re-check.
- No public catalogue page publishes a delivered-to-Tunisia (sous-douane)
  USD price for any line; that pricing only exists as a freight-forwarder /
  broker quotation. The consolidated RFQ step for that quotation is
  defined in the master plan §2.2.
## H003 adapter FEM — preloaded elastic-foundation model (2026-09-15)

This section supersedes the FEM summary inside the R6-fasteners bullet above; the fastener/geometry facts there are unchanged, while the ideal-clamp FEM numbers (18.8267 MPa, 14.7581%) are replaced by the preloaded-joint run below.

- Model class: **linear elastic-foundation (Winkler) support with explicit preload pressure**. It is not a contact, preload-loss or friction solve; no measured joint stiffness exists for this custom H003-to-F0 joint.
- Preload: **4 × F0 = 8.0 kN** per ISO4762 M5×16 class 10.9 screw, 60% of the 13.35 kN ISO 898 proof load (A_s = 14.2 mm², R_p0.2 = 940 MPa). This is a **required-preload screen, not a torque prediction**: the HIWIN M5 6.0 Nm table entry is explicitly not evidence for this custom joint.
- Preload application: uniform pressure over the counterbore-floor washer seats r ∈ [2.75, 4.5] mm (Ø5.5 hole edge to ISO7092 washer OD 9 mm), pushing the adapter against the stage.
- Foundation stiffness: normal **kn = 50,538.5 N/mm³** per unit area, from the elastic half-space stiffness of one rigid annular punch 2·E·b/(1−ν²) = 2014.41 MN/m per seat (E = 205 GPa, ν = 0.29, b = 4.5 mm) spread over the 39.86 mm² seat annulus; tangential **kt = kn/2.67** (documented steel-interface shear-to-normal rule, unmeasured).
- Results (finest 111,349 nodes, supernodal CHOLMOD direct solver, 150,000-node cap unchanged): peak 219.489 MPa; displacement 0.00863849 mm; displacement refinement 2.15931% (PASS 3%); raw peak refinement 16.9981% (**REFINE** 5%); peak vs yield/3 = 219.489 vs 216.667 MPa (**FAIL**, local seat compression inside the 640.7 MPa Lamé bound).
- Foundation-stiffness sensitivity ×0.5 / ×2 on the identical finest mesh: peak 219.07 → 220.08 MPa (+0.46%); displacement 0.01262 → 0.00667 mm. The raw-peak hold is an ideal uniform-pressure patch-edge artifact, not a spring-calibration uncertainty.
- What remains: a true contact analysis with a measured joint stiffness, or a resolved seat pressure profile; both are outside desk-available evidence. The gate stays REFINE.
