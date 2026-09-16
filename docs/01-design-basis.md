# S1-B — active design basis

**12 September 2026.** This document replaces the obsolete S1-A dimensions and process arithmetic. The selected head is R6; feed geometry is R6; measuring interfaces/calibration are R1. The machine is a fabrication-ready design package for the internal S1-B build; energisation and commissioning remain gated by the physical measurements in the master plan. The source definitions are [cutting head](../shared/cutting_head_spec.hpp), [feed drive](../shared/feed_drive_spec.hpp) and [measurement](../shared/measurement_spec.hpp).
## 1. One architecture and component set

One lane; two independent feed cassettes; four closed-loop stepper axes; two independently driven opposing 90° V blades; separate actual-wire measuring wheels; guarded payoff, contained scrap and supported output. Optional crimping is a separate guarded module, not an unguarded attachment to this head.
| Subsystem | Selected hardware |
|---|---|
| Each cutting axis | HIWINKK8610C-340A1-F0,34E1K-85,CL86T-V41,R+WMK2/100/50/8/14 |
| Cutting power | 2×LE-350-60, one isolated 60V branch per drive; not paralleled |
| Each feed cassette | 24HS40-5004D-E1000,CL57T-V41,R+WMK2/100/50/10/12; two driven lower/two passive upper rollers |
| Feed transmission | HTPA20S5M250-A-N14×2,HTBN225S5M-250×1 per cassette |
| Rollers | UMCL40-10-20-MC with documented F110 secondary keyway; 8 total |
| Rotor bearings/retention | SKF6001-2Z/C3+6002-2Z/C3,KM2,MB2,SEEGER A12 per rotor |
| Feed power | 1×LE-350-36, separate protected branches to both drives |
| Each measuring module | ifm E60137 wheel, ifm RU3100 encoder (2026-09-16 substitutions), two SKF 626-2Z; independent wheel-shaft support |
| Cabinet cooling | Rittal SK3303.500; 230V 50Hz; 280W manufacturer L35/L50 capacity |
| Cabinet controls | Initial design input in [controls and wiring](22-controls-and-wiring.md); not an electrical/safety release |

The [active BOM index](../bom/README.md) identifies exact selections and observed prices. Currencies are USD; converted EUR prices are explicitly dated. No delivered total is fabricated. Duty/tax is budgeted zero under the user's stated **sous-douane** premise, not a customs eligibility determination.
Embedded and host software are **Rust, C or C++ only**; current replacement models/CAD/control reference use C++20/Qt. Personnel safety must be independent of MCU/HMI. Selected drives have no STO; software enable is not safety isolation.

## 2. Operating requirements

| Parameter | Requirement, not measured performance |
|---|---|
| Material | Single-conductor stranded copper/PVC, 0.5–2.5 mm² |
| Actual wire OD | 1.2–4.5 mm |
| Finished length | 200–3000 mm; initial qualification 200–1000 mm |
| Stripping | Disabled or 3–25 mm independently at each end |
| Length target | ±(0.20 mm + 0.05% length), ±0.70 mm at 1 m |
| Strip target | ±0.20 mm |
| Feed ceiling | 0.50 m/s; initial controlled commissioning 0.05 m/s |
| Cutting axis ceiling | 12.5 mm/s, 300 mm/s²; 100 pulses/mm, 1250 Hz maximum command rate |
| Feed command | 2000 pulses/rev; shaft ceiling 260 rpm, 8666.7 Hz; loaded radius calibration required |
| Head loads | 2500 N axial, 750 N transverse, 375 N vertical; all signs; double proof |
| Feed structural envelopes | 1 N m running, 2.5 reversing service factor; separate 5 N m fault; belt 480 N radial; roller ±60 N in both planes |
| Room/cabinet | 40°C room design case; selected drives limit 40°C; active cooling/overtemperature integration required |
| Mains | Selected supplies/cooler 230V 50Hz; site protection, PE and wiring still must be completed |

No live wire, solid copper, aluminium, steel, shielded/multicore/coax, silicone, PTFE/XLPE or high-voltage processing is qualified by these selections. No production pieces/hour or endurance claim is inferred from motor resolution. The legacy controller in [software/firmware/firmware_controller.cpp](../software/firmware/firmware_controller.cpp) rejects job start with `NotCommissioned`. The separate production-controller source in [software/machine/](../software/machine/) implements retained jobs and panel start logic for the Pico 2 target, but it has no assembled-machine, safety-function or process-qualification evidence.
## 3. Coordinates and interfaces

Millimetres. X is wire travel, Y is cutting-stroke direction, Z is up. Cut plane X0; wire Y0, Z112. Head datum H001: 180×1040×20, from X−90…90, Y−520…520, Z0…20. H002 risers puts stages at Z40; carriage top Z86. Actual original motor rear planes are approximately Y±604.1, with cable geometry included. **The old 800×700 base and 430 mm-high guard are not the active machine envelope.**
R6 H003: 100×100×15, Ø49.97 F0 register, Ø73.10 motor pilot, PCD70 F0 screws with Ø9.5×8.5 counterbores, 69.58 square M5 motor grid. [A3 adapter drawing](../engineering/generated/drawings/H003-R6-adapter.pdf). Coupling at free 49–50 mm length; set actual stage hub 1.00±0.05 from locknut shoulder. The 256-corner chain retains full hub engagement.

Nominal right-cassette nips X75.5 and138.0;centres62.5,requiredadjustment52.5–64.5. Theleftcassette uses a **proper180°rotation aboutZ**,not a reflection ofpurchasedparts. Motoronfar/outboardshaft;nearshaft isbeltfollower. ForwireOD$d$,lowerrolleraxis$Z=112-20-d/2$,upperaxis$Z=112+20+d/2$. Upperhousings rotate180°abouttheirshaft so theirmountingbridgesfaceup. The[287-partplacement](../engineering/generated/layout/head-and-feed-layout.step) passeshead/feedstaticchecks;allmovinghead/feedpairs have17.5mmXseparation. Lower/upperheightandpressureadjusters are notyetintegrated. Left/rightmotor directionpolarity mustaccountfor theopposedshaftdirections.
The wire contact is at rotor station S108.75, **5.25 mm off the 20 mm roller's midplane**, to avoid the MC access holes. With specified incoming hole/position limits, 4.5 mm wire and ±0.20 mm assembled lane tolerance: 0.55 mm minimum clearance to the access pocket, 2.10 mm to tread edge. [Feed machining/assembly record](12-feed-cartridge-fabrication.md). F112 front bearing module must be removable; MB2 must be installed before the pulley key.

R4bearingseats are matchedj5,3–6µmactualinterference inside−3/+5µmnominalshaftlimits. Housingbores28.008–28.018and32.008–32.019retainouter-ringclearance. [Fit/temperature envelope](../engineering/generated/bearing-fit/report.md) enumerates256corners;itdoesnot replace verificationoftheactual thermalfield ormanufacturinggaugecapability.

Measured wire travel: nominal 200 mm wheel circumference (ifm E60137)/4096 counts = 0.048828125 mm/count. The assumed ±0.2 mm free circumference alone permits1.0 mm error per commanded metre. The corrected uncalibrated1m allocation sum is1.65 mm, not1.15. The wheel's loaded circumference, phase error, wire tension, temperature and wear must be calibrated; motor shaft feedback is not a substitute. At0.50 m/s, nominal measuring count rate10240/s. The [R2 measurement record](13-measuring-interfaces-and-calibration.md) includes the source dimensions, separate5-part encoder face mount, A3 M201 plate drawing, reversible phase-map fitter and explicitly synthetic numerical validation. It does not claim that the shaft/pressure/guide assemblies or physical calibration are complete.
## 4. Process geometry

This is the required sequence, not a claim of commissioned hardware firmware. Start with trimmed stock tip at X0; output cassette empty; blades open. Prime/ejection require the final guide/sensor/takeaway integration and are not validated by commanded roller travel alone.

1. **Lead strip:** advance $s_1$; score using a qualified incision diameter; retract $s_1+3$ while blades retain insulation; open/clear slug. Bare tip X−3. If strip disabled, tip remains X0.
2. **Length:** feed $L+3$ after lead strip, or $L$ if disabled. Tip X$L$. Use actual wire feedback; the downstream wheel is valid only after wire reaches it. Stop/settle before blade motion.
3. **Cut:** stationary feeds; close to qualified cut overlap; open. Piece X0…$L$, stock tip X0.
4. **Tail strip:** withdraw stock $s_2+10$; backfeed piece left $s_2$. Stock tip X$-s_2-10$, piece tail X$-s_2$: 10 mm separation. Score at X0; pull piece right $s_2+3$; open/clear slug. Verify motion with downstream wire encoder.
5. **Discharge:** move piece into supported takeaway; require a part-clear transition after a prior present state. Motor travel after the nips release the part does not prove part translation. Restore stock only after clearance confirmation. Failed clearance quarantines the piece and faults the machine.
6. Pause only at the clear boundary. STOP/fault aborts, invalidate home/prime/approval, and never automatically restarts after recovery.
**Shortest-part arithmetic:** maximum far nip X$75.5+64.5=140$ mm. For $L=200$ sized part, $s_1=s_2=25$, the leading insulated edge after tail backfeed is $200-25-25=150$ mm. Thus **10 mm insulated stock remains beyond the far nip**. The earlier near-nip X120 would have violated this retained-grip requirement; it is withdrawn.

Ideal opposed 90° V geometry has incision diameter $D_i=g/\sqrt2$, where $g$ is apex separation. With 8 mm open apex gap, nominal jaw closure $(8-\sqrt2D_i)/2$. Actual ground edge geometry and copper strand layout control the qualified incision; do not set blade depth from equivalent copper area alone.
## 5. Manufacturing and evidence

Laser/brake for sheet metal; CNC for custom metal datums, holders, shafts and housings; outsourced grinding/heat treatment where required. Printed parts are non-structural guide/service items. Available machinery is not proof that 0.005 mm cylindricity, ground blade seats or heat-treatment hardness are achievable without inspection.

- R4 isolated holder FEM converged for all load signs, gravity and double proof; it does not include assembled blade/stage/joint compliance. R6 adapter and datum convergence remains separate.
- R6 head checks actual motor/fastener intersections, continuous independent jaw sweep and CAD readbacks.
- Feed calculation checks cover catalogue belt capacity, tension, shaft beam, key-bearing areas, retention and wire-lane/grip geometry. Housing, root/contact/fatigue, pressure mechanism and loaded tire behaviour are additional models.
- Feed R6 defines the shaft-end/M15-lead chamfers and the MB2 slot plan, vertical-intersection and floor blends. A [191-section nominal stiffness audit](../engineering/generated/feed-sections/report.md) checks the smaller principal bending inertia against the reduced beam model; it does not replace local stress or thermal-field closure. [Shaft sheet1](../engineering/generated/drawings/F101-R6-shaft.pdf), [detail sheet2](../engineering/generated/drawings/F101-R6-details.pdf).
- The [four-mesh F101 R6 stress study](../engineering/generated/feed-fem/report.md) resolves nominal shaft elasticity with distributed supports, rotating radial loads, balanced transmission torque and actual shaft gravity. Whole-shaft peak/displacement last-refinement criteria pass, but16 regional checks remain unmet. Actual threads, final edge breaks, contact/preload, role-specific reactions and pressure/friction/jam force limits remain qualification work. [Model scope](15-feed-shaft-stress-model.md); [verification](verification/feed-shaft-fem.md).
- Cabinet safety, cooling/airflow, regen/inrush/protection, guard locking, full frame/tooling/wiring and physical commissioning remain explicit deliverables. Historical BOMs and old S1-A coordinates must not be used to fill them by assumption.