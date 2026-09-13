# Komax comparison · evidence, not marketing equivalence

Research date **11 September 2026**. This is a feature survey of relevant product families, not a claim to enumerate every Komax product or proprietary detail. Manufacturer specifications are not independently measured here. Source numbers are used in purchasing and validation documents.

## Published comparison

| Reference | Published range / physical features | Published length figure / transport |
|---|---|---|
| Legacy Kappa 310 [S2] | Stranded 0.02–6 mm², OD ≤10 mm; single-roller drive, motorized closing, 25 kg | Repeat accuracy ±(0.2% + 1 mm), maximum transport 4 m/s |
| Legacy Kappa 330 SH/DH [S3,S4] | Stranded 0.22–35 mm², OD ≤16 mm; double rollers or belts, optional length measurement, interchangeable single/dual tooling; 55/60 kg | Same published family repeat expression; maximum 4 m/s |
| Legacy Kappa 350 [S4] | 2.5–120 mm², OD ≤35 mm, double blade arrangement, measurement/detectors, integrated table, about 270 kg | Same family repeat expression; maximum 4.8 m/s; inappropriate size for most S1 wires |
| Current E300 [S5] | Stranded 0.05–10 mm²; solid only up to 1.5 mm²; belted-roller quick-change feed and programmable pressure | Standard minimum 48 mm; optional/short mode 18 mm remaining insulation; 34 kg |
| Current E400 [S6] | Stranded 0.14–16 mm²; OD 0.8–12.5 mm; symmetric high-precision clamping, belt feed, integrated short-piece kit, 10-inch HMI | Length 23–1,000,000 mm; 42 kg. Web repeat expression omits unit on additive 1.0: obtain written clarification |
| Alpha 530 [S7] | 0.13–6 mm², max OD 5.1 mm; two process modules per side; guide quick-change, steel tray, protected mechanics | 60–65,000 mm, optional 30–60 mm; ±(0.2% + 1 mm); feed up to 9 m/s; ~1.3 tonnes with two crimp modules |
| Alpha 550 G2 [S8] | 0.13–6 mm², optional 10 mm²; 12 m/s belt feed; modular crimp/seal handling | Same reproducibility expression; 60 mm minimum (optional 30). Maximum length conflicts: 12 m webpage vs 65 m brochure; do not silently resolve |
| Mira 230/Q, legacy [S9] | 0.03–8 mm² stripping, OD ≤6.5 mm; 1.5–46 mm strip; Q adds conductor contact monitoring | Typical 0.35 s end-processing cycle; 0.01 mm setting increments **not accuracy**; not reel-to-length output |
| Current B300 [S10] | 0.03–8 mm² stripping; linear opposing blades; Core/Plus/Pro/Ultra; AIC and ACD on higher variants | Typical 0.35 s depends on recipe; adjustment increments are not finished strip accuracy |

Komax announced in August 2024 that Kappa and Mira were discontinued in favor of equivalent Schleuniger products [S1]. Legacy brochures below are manufacturer documents hosted by distributors, not proof of current availability.

At 1 m, ±(0.2% + 1 mm) evaluates to **±3 mm**. The S1 length goal ±0.70 mm would be tighter **on a qualified recipe if achieved**. It does not establish equivalence in wire range, handling, throughput, process capability, uptime, support or service life. Numeric Cpk, MTBF, general blade life and guaranteed 24/7 availability were not published in the reviewed material.

## S1-B status against the published Komax feature set

| Industrial feature | S1-B state (head R6 / feed R6 / measuring R1) |
|---|---|
| Cutting: opposed 90° V blades on rigid axes | Two HIWIN KK86 axes, 34E1K-85 closed-loop motors, R6 adapter/holder CAD and FEM; blade pair is purchase-to-drawing and **not yet sourced or edge-qualified** |
| Wire transport | Two keyed PU-roller cassettes with S5M belt drive; spring-loaded pressure carriers, wire guides, bed and frame integrated in the [507-occurrence R3 CAD](../engineering/generated/mechanism/status.json) with interference and force-window checks at both wire extremes; 15 guard/payoff parts are R2-selected design only; **not fabricated** |
| Actual length feedback | Two Kübler wheel / Autonics encoder modules with M201 plate, spring arm and 626 counter-roller integrated in CAD; calibration data synthetic; [qualification tool](20-physical-qualification.md) exists but **no measurement has been taken** |
| Programmable grip pressure, belt clamping, ACD, AIC, vision, crimping, stacking, marking | **Not implemented**; none is claimed |
| Controls | RP2350 target source includes PIO, watchdog and control-layer supervision; the retained legacy host reference returns `NotCommissioned`. Neither has physical hardware or safety-function validation. |
| Payoff, guards, cabinet, wiring, safety relay | Initial logical cabinet design input exists in [controls and wiring](22-controls-and-wiring.md), plus [preliminary static guard/payoff accommodation](25-preliminary-guard-and-payoff.md); no released cabinet, guard/payoff design, assembly or qualification exists |

No Komax parity, throughput, endurance or accuracy claim is made or supported by the repository. The published Komax length figure is a manufacturer specification; the S1 ±0.70 mm/1 m figure is a design target that has never been measured on hardware.

Q1250 [S11] publishes a 24 × 16 mm field of view, 0.13–6 mm² wire range and up to 18 mm strip. Usable detection region is smaller. Its options inspect visible defects; no universal hidden-nick detection accuracy is published. Q1240 [S12] publishes output impact ≤1% above 200 mm and up to 4% at 60–200 mm; do not transfer those figures to another inspection model.

Alpha 530 output graphs [S7] use FLRY 0.5 mm², 6 bar, 9 m/s, C1370/S1441 modules, CFA active, **ACD/Q1250 inactive**, deposit gripper active. Rates were not tabulated, so no invented graph values appear here. Feed velocity is not pieces/hour. The S1 cycle calculator includes acceleration, reversals, cutting, stripping, cleanup and ejection, but not a validated OEE.

## Primary sources and legacy mirrors

- **S1** [Komax structural optimization / discontinuation, 13 Aug 2024](https://www.komaxgroup.com/en/media-releases/consistent-cost-reduction-and-structural-optimizations-as-the-basis-for-future-profitable-growth)
- **S2** [Original Kappa 310/315/320 brochure, Cablespeed mirror](https://cablespeed.co.uk/wp-content/uploads/2021/10/0361390_2_PRO_Kappa_310_315_320_EN.pdf)
- **S3** [Original Kappa 330 SH/DH brochure, Scanditron mirror](https://wire-harness.scanditron.com/images/pdf/PRO_Kappa_330_V1_E.pdf)
- **S4** [Original Kappa family brochure, Unitek mirror](https://unitek-elektrik.com/Images/Product/5/20160222145945743.pdf)
- **S5** [E300](https://www.komaxgroup.com/en/products/wire-processing/semi-automation-machines/wire-cut-and-strip/cut-and-strip-family-e300)
- **S6** [E400 and linked manufacturer brochure](https://www.komaxgroup.com/en/products/wire-processing/semi-automation-machines/wire-cut-and-strip/cut-and-strip-family-e400)
- **S7** [Alpha 530](https://www.komaxgroup.com/en/products/wire-processing/higher-automation-platforms/automatic-crimping/alpha-530) · [brochure V11](https://eu-central-1-enterprise-euc1.graphassets.com/Avst4xSGfMbQnhS8Fgi62z/cm9iaxg1j61iw07uqdbbpiky2)
- **S8** [Alpha 550 G2](https://www.komaxgroup.com/en/products/wire-processing/higher-automation-platforms/automatic-crimping/alpha-550-g2) · [brochure V4 Jan 2026](https://eu-central-1-enterprise-euc1.graphassets.com/Avst4xSGfMbQnhS8Fgi62z/cmmmm97rl2q5h07wew5jkd2d7)
- **S9** [Original Mira brochure, Cablespeed mirror](https://cablespeed.co.uk/wp-content/uploads/2021/10/0359151_4_PRO_Mira230_Mira230Q_EN.pdf)
- **S10** [B300](https://www.komaxgroup.com/en/products/wire-processing/semi-automation-machines/wire-stripping/wire-stripping-wire-stripping/strip-series-b300) · [B300 quality-control explanation](https://www.komaxgroup.com/en/stories/strip-series-b300-now-features-intelligent-quality-control)
- **S11** [Q1250 scalable](https://www.komaxgroup.com/en/products/testing-and-quality-tools/cable-and-harness-quality-tools/integrated-quality-monitoring/q1250-scalable)
- **S12** [Q1240 manufacturer brochure](https://eu-central-1-enterprise-euc1.graphassets.com/Avst4xSGfMbQnhS8Fgi62z/cm9ib7stb64nv07wbg5w2potg)
- **S13** [ACD](https://www.komaxgroup.com/en/products/testing-and-quality-tools/cable-and-harness-quality-tools/integrated-quality-monitoring/acd) · [AIC](https://www.komaxgroup.com/en/products/testing-and-quality-tools/cable-and-harness-quality-tools/integrated-quality-monitoring/aic)
- **S14** [PreFeeder 1000](https://www.komaxgroup.com/en/products/wire-processing/peripherals-and-marking/wire-handling/prefeeding/prefeeder-1000) · [F1160](https://www.komaxgroup.com/en/products/wire-processing/peripherals-and-marking/wire-handling/prefeeding/f1160)
- **S15** [WireStacker 3150](https://www.komaxgroup.com/en/products/wire-processing/peripherals-and-marking/wire-handling/stacking/wirestacker-3150) · [dps 375](https://www.komaxgroup.com/en/products/wire-processing/peripherals-and-marking/wire-handling/stacking/dps-375)
- **S16** [Delta 220](https://www.komaxgroup.com/en/products/wire-processing/semi-automation-machines/wire-crimping/delta-220) · [UniCrimp 208](https://www.komaxgroup.com/en/products/wire-processing/semi-automation-machines/wire-crimping/unicrimp-208)
- **S17** [C1370 module](https://www.komaxgroup.com/en/products/wire-processing/higher-automation-platforms/process-modules/c1370) · [C1370/C1340 brochure](https://eu-central-1-enterprise-euc1.graphassets.com/Avst4xSGfMbQnhS8Fgi62z/cm9ib5bjo63ae07uqupb7jkdz)
- **S18** [Komax CARE maintenance](https://www.komaxgroup.com/en/services/care-service-agreements/care-maintain)
- **S19** [Pico manufacturer hardware/SDK documentation](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)
- **S20** [StepperOnline closed-loop kits](https://www.omc-stepperonline.com/closed-loop-stepper-kit). The page showed a 1.2 N·m NEMA23 kit at USD 47.75 during research; this does **not** establish delivered cost, sufficient dynamic torque, or the price of the selected higher-margin kit.

Supplier estimates in the BOM are budgeting allowances, not quotations or guaranteed live prices. Public URLs can move. Save purchased part drawings, firmware revisions and dated quotations in the released technical file.