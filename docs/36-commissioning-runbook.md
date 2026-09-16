# Commissioning runbook — physical closeout of every gate, in order

**Revision:** `S1-B-RB-R1-2026-09-16`
**Status:** the single ordered execution plan that closes every PHYSICAL-ONLY
row of the [gate ledger](33-gate-ledger.md) in sequence. Nothing in this
document closes a gate; only an executed step with a filled record does.
This runbook cross-references the existing procedures and record templates —
it deliberately does not duplicate their text, and no value written here
overrides the governing document it cites.

## How to use this runbook

1. Execute phases 0 → 10 in order. A missing RECORD blocks every dependent
   step (docs/31 §7 warning; docs/23 §3: a blank cell is an open hold, not a
   pass).
2. Each step has four fields: **DO** (actions), **MEASURE** (instrument +
   method), **CRITERION** (pass value), **RECORD** (template/file). Where the
   governing record already defines the method, the step cites it instead of
   repeating it.
3. The desk torque/preload values ([ledger §2.2/§2.4](33-gate-ledger.md),
   [docs/31 §7](31-fabrication-guide.md)) are applied in phase 4 and recorded
   in the build record; they are desk conversions, not assembly authorisations.
4. Phases 0–4 assemble the machine; phase 5 closes the electrical/safety gates
   before energisation; phases 6–9 commission motion, payoff, guard and
   thermal behaviour; phase 10 is the [30-piece Cpk qualification](20-physical-qualification.md).
   No first article runs before phases 0–9 are recorded.
5. The templates used are: [incoming inspection](inspection/incoming-inspection-template.csv),
   [production build record](inspection/production-build-record-template.csv),
   [F101 R6 inspection](inspection/F101-R6.csv),
   [M201 inspection](../engineering/generated/measurement/M201-inspection-template.csv),
   [guard/payoff release](inspection/guard-payoff-release-template.csv),
   [first article + energisation gate](inspection/first-article-and-energisation-gate-template.csv).

### Gate → runbook step map

| Ledger gate(s) | Runbook steps |
|---|---|
| 1 (key/collar physical) | P0-2, P4-1, P4-4, P4-5 |
| 2 (adapter joint stiffness physical) | P4-2 |
| 4 (M5/M10 torque record) | P4-1 |
| 6 (wheel hub-clamp record) | P4-3 |
| 7 (weld inspection) | P3-1–P3-4 |
| 8 (true thread roots) | P2-2, P2-3 |
| 9 (edge breaks) | P2-4 |
| 10 (bearing preload/thermal) | P1-1–P1-5, P9-1–P9-3 |
| 11 (fatigue) | P6-9 |
| 12 (±60 N roller envelope) | P6-4–P6-6 |
| 13 (blade pair physical) | P0-5, P6-8 |
| 14 (risk assessment + aperture sign-off) | P8-2, P8-3 |
| 15 (SLC/ACF fit + G106 pattern) | P3-4, P8-1 |
| 16 (payoff validation) | P7-1–P7-4 |
| 17 (PC door) | P8-4 |
| 18 (IF-B01 build/fixture) | P5-1 |
| 19 (DC switching duty measured) | P5-5 |
| 20 (fuses/decay coordination) | P5-6 |
| 21 (AC/PE/insulation physical) | P5-2, P5-3 |
| 22 (guard-release physical) | P5-6, P8-5 |
| 23 (safety-function validation) | P5-4, P5-7 |
| 24 (commissioning) | P6-1 |
| 25 (30-piece Cpk) | P10-1–P10-5 |
| 26 (FAI + energisation gate) | P10-6 |
| 27 (fabrication/build-record evidence) | P0, P2, P3, P4, P10-8 |
| 28 (measuring module) | P2-6, P6-5 |
| 29 (calibration + 626 screen) | P1-5, P6-7 |
| 33 (wheel circumference at receipt) | P0-4 |
| 41 (EMC assessment) | P5-9 |
| 42 (guard/payoff flags) | P7, P8 |
| 43 (integration flags) | P0–P9, P10-7 |

---

## Phase 0 — Incoming inspection

Everything arrives before anything is machined or assembled. This phase feeds
the [incoming-inspection template](inspection/incoming-inspection-template.csv)
(gate 27) and the physical halves of gates 1, 13, 15, 33.

### P0-1 Unpack, identify, certificate

- **DO:** unpack and verify every ordered row against the exact identities in
  [docs/31 §1](31-fabrication-guide.md) and the sourcing lists; collect
  certificates/CofC and lot/serial numbers.
- **MEASURE:** visual identity check; counts.
- **CRITERION:** identity, quantity, lot/serial and certificate present for
  every row; any deviation opens a nonconformance row.
- **RECORD:** [incoming-inspection-template.csv](inspection/incoming-inspection-template.csv).

### P0-2 F110 UMCL40-10-20-MC collar bore (gate 1 physical)

- **DO:** measure the incoming bore before any keyway machining (docs/12:111–113).
- **MEASURE:** calibrated three-point bore gauge / bore micrometer.
- **CRITERION:** bore 10.000–10.030 mm; keyway (3.04 ± 0.01 wide, 2.35 ± 0.025
  from the actual bore) is cut after acceptance, not assumed.
- **RECORD:** incoming template; keyway machining result later in P4.

### P0-3 Bearing bores measured for matched fitting (gate 10 physical)

- **DO:** measure every 6001-2Z/C3 and 6002-2Z/C3 bore (plus OD/width),
  626-2Z and 6204-2Z/C3; assign permanent IDs for match-marking.
- **MEASURE:** bore gauge / calibrated comparator at 20 °C; combined diameter
  uncertainty target ≤ 0.5 µm for the matched seats.
- **CRITERION:** recorded values feed the
  [matching-limits](../engineering/generated/bearing-fit/matching-limits.csv)
  and [256-corner fit record](../engineering/generated/bearing-fit/report.md);
  no seat may be finished to nominal j5 without a measured bore (docs/12:36).
- **RECORD:** incoming template + matching-limits.csv workflow.

### P0-4 ifm E60137 wheel 200 mm circumference claim (gate 33 physical)

- **DO:** verify the manufacturer circumference claim at receipt
  (ifm E60137 "U:200 N:06", Ø63.66 mm, 200 mm circumference; ifm publishes
  no tolerance).
- **MEASURE:** traceable calibrated reference (roll/tape method of
  [docs/13 §5](13-measuring-interfaces-and-calibration.md)); record reference
  identity and uncertainty.
- **CRITERION:** 200 mm (assumed ±0.2 mm acceptance 199.8–200.2 until a
  manufacturer tolerance exists); confirm the hub/clamping-screw detail.
- **RECORD:** incoming template; result retained as input to P6-7.

### P0-5 Blade pair T001 (gate 13 physical)

- **DO:** receive the purchase-to-drawing blade pair; verify against the
  drawing; store it uninstalled.
- **MEASURE:** dimensional/visual check vs the drawing.
- **CRITERION:** drawing-conformant; installation blocked until P6-8 (loaded
  clearance + edge qualification, docs/24 §4.4).
- **RECORD:** incoming template.

### P0-6 Keys and fasteners identity (gate 5 confirmation)

- **DO:** measure the received F109 keys and pulley keys, fasteners, washers.
- **MEASURE:** micrometer / thread gauges.
- **CRITERION:** F109 3×4×18 (width 2.975–3.000, height 3.950–4.000, length
  17.90–18.00); DIN 6885-A 5×5×28 pulley key; fastener classes per docs/12 §3/§5.
- **RECORD:** incoming template.

### P0-7 Electrical/controls identities and revisions

- **DO:** verify XPSUAF13AP, 3SU1851 E-stop, SLC-F-024-20/20-R4 + ACF-1,
  LC1D12BD ×3, PSUs, ifm RU3100 encoders, CL57T/CL86T drives, IF-B01 board, fused
  terminals/links (ESKA 522620 2 A / 522617 1 A in 3046032) and the incoming
  iC60N + iID against [docs/22 §2–§3, §11](22-controls-and-wiring.md); archive
  all manuals (incl. the Bernstein installation manual 0800000865).
- **MEASURE:** identity/revision check against the controlled records.
- **CRITERION:** exact match; stop if any identity or revision differs
  (docs/23 §1).
- **RECORD:** incoming template.

---

## Phase 1 — Matched bearing measurement and seat finishing

Closes the physical half of the bearing-fit gate (gate 10) and the 626
thermal/matching decision (gate 29). Governing requirements: docs/12 §2.

### P1-1 Measure and match seats to assigned bores

- **DO:** measure each assigned bearing bore, then finish its shaft seat to
  the intersection of j5 limits and 3–6 µm diametral interference.
- **MEASURE:** calibrated micrometer, two angular positions × three axial
  locations per seat (docs/12:150).
- **CRITERION:** rear 12j5 seat 11.997–12.005 **AND** 3–6 µm interference with
  the assigned 6001 bore; front 15j5 seat 14.997–15.005 **AND** 3–6 µm with
  the assigned 6002 bore; with combined U ≤ 0.5 µm accept measured
  interference 3.5–5.5 µm (guard-banded).
- **RECORD:** [F101-R6.csv](inspection/F101-R6.csv) rows `rear_bearing_seat`,
  `front_bearing_seat`, `measured_interference_guardband` + matching-limits.csv.

### P1-2 Seat form and finish

- **DO:** finish both seats in a common setup; outsource cylindrical
  finishing if the shop cannot demonstrate the diameters and runout.
- **MEASURE:** form metrology (cylindricity, runout to common axis A),
  profilometer.
- **CRITERION:** cylindricity ≤ 0.001 mm; total radial runout ≤ 0.002 mm to
  A; Ra ≤ 0.4 µm (docs/12:34).
- **RECORD:** F101-R6.csv rows `seat_cylindricity`, `seat_total_runout`,
  `bearing_Ra`.

### P1-3 Match-mark and protect

- **DO:** match-mark assigned pairs and spares; no Loctite retainer
  (docs/12:36). Do not copy the j5/C3 feed-rotor match onto the 626 CN
  measurement bearings (docs/13:67).
- **RECORD:** F101-R6.csv `identity` row.

### P1-4 Housing bores — clearance fit confirmation (gate 10 physical)

- **DO:** measure F112 front bore and F102 rear bore with the pair clamped.
- **MEASURE:** bore gauge.
- **CRITERION:** F112 Ø32.008–32.019 (8–28 µm diametral clearance to the
  6002 OD — no housing interference; the record governs, ledger §2.5);
  F102 Ø28.008–28.018.
- **RECORD:** build record.

### P1-5 626 CN thermal screen (gate 29 physical)

- **DO:** measure installed 626 clearance before wheel assembly and decide the
  thermal/matching treatment or a substitution design.
- **CRITERION:** the CN + 10 K ΔT stack leaves 0.5 µm before any press-fit
  loss (docs/13:67) — resolve by measurement or design; do not accept it by
  assumption.
- **RECORD:** build record.

---

## Phase 2 — Machining/finishing checks (F101, H003, M201)

Closes gates 8 (true thread roots), 9 (edge breaks), 27 (build records) and
the M201 part of gate 28.

### P2-1 F101 full dimensional inspection

- **DO:** complete every row of [F101-R6.csv](inspection/F101-R6.csv) against
  the two A3 sheets (docs/12 §2, §7).
- **MEASURE:** per-row method in the template (micrometer, gauges, optical
  profile, datum-referenced rotation + DTI).
- **CRITERION:** all template rows pass; key runouts ≤ 0.010 mm to A, B-face
  perpendicularity ≤ 0.004 mm, key-seat and MB2-slot geometry per docs/12 §2.
- **RECORD:** F101-R6.csv.

### P2-2 M5 blind hole — true thread roots (gate 8)

- **DO:** execute the [docs/14](14-f101-tapping-operation.md) operation:
  E412M5 physical-tip acceptance, Ø4.20+0.03/0 drill to 18.00 ± 0.05
  cylindrical depth + 118° ± 2° point, same-lot coupon proof, then tap.
- **MEASURE:** optical tool measurement of tip-to-first-full-form **a** with
  uncertainty; thread depth gauge + 6H GO/NO-GO (gauge's own lead not
  counted).
- **CRITERION:** guarded a ≤ 3.18 mm; ≥ 0.50 mm cylindrical bottom clearance
  at deepest permitted insertion (no drill-point credit); 14.00–14.05 mm
  full-form thread after the actual C0.20 entry; 6H GO/NO-GO pass
  (docs/14 §2).
- **RECORD:** F101-R6.csv rows `M5_*` + tool acceptance record.

### P2-3 M15×1 thread — true thread roots (gate 8)

- **DO:** verify the KM2/MB2 retention thread.
- **MEASURE:** 6g GO/NO-GO gauges; full-profile length measurement.
- **CRITERION:** full-profile thread over S65.10–71.30; entry C0.50 ± 0.02;
  Ø13.50 ± 0.02 runout relief with R0.20 roots; no overrun into the bearing
  seat (docs/12 §2).
- **RECORD:** F101-R6.csv rows `M15_*`.

### P2-4 Final edge breaks (gate 9)

- **DO:** controlled 0.05–0.10 mm deburr of the remaining slot openings
  (docs/12:58).
- **MEASURE:** optical + visual inspection.
- **CRITERION:** 0.05–0.10 mm break; no raised lips; no load-flank rounding;
  bearing-seat lands untouched.
- **RECORD:** F101-R6.csv `remaining_slot_edges` + build record.

### P2-5 H003 motor adapters (×2)

- **DO:** machine and inspect per the H003 R6 sheet (docs/31 §2).
- **MEASURE:** CMM / bore gauge / micrometer.
- **CRITERION:** F0 register Ø49.97 ± 0.01; pilot Ø73.10 +0.02/0; faces and
  counterbores per the sheet; 4× M5×16 class 10.9 + ISO7092 on PCD70.
- **RECORD:** H003 sheet record / build record (joint verification follows in
  P4-2).

### P2-6 M201 encoder plates (×2) (gate 28)

- **DO:** machine and inspect per [docs/13 §2](13-measuring-interfaces-and-calibration.md).
- **MEASURE:** metrology per the M201 template (external inspection for
  instruments not available in the workshop).
- **CRITERION:** central bore Ø20.040–20.060 through; cylindricity 0.01;
  perpendicularity 0.02 to A; hole positions Ø0.10 to A|B|C; flatness 0.02;
  Ra per the sheet.
- **RECORD:** [M201-inspection-template.csv](../engineering/generated/measurement/M201-inspection-template.csv).

---

## Phase 3 — Weld inspection (posts/base, spool, frame, jamb)

Closes the physical weld inspection behind gate 7 and feeds gates 15, 16, 42.
Procedures and fixtures: [docs/34](34-weld-and-assembly-procedures.md).

### P3-1 Frame welds (F304/F303/F302/F305)

- **DO:** weld legs to rails, rails to cross rails, feet to legs; check
  full perimeter, no undercut (docs/30 P1).
- **MEASURE:** visual + straightedge/DTI on the four foot pads.
- **CRITERION:** full perimeter welds, no undercut; four foot pads coplanar
  ≤ 1 mm over the 1080 × 440 rectangle.
- **RECORD:** build record.

### P3-2 Payoff post-to-base weld (P004/P005 → P003)

- **DO:** weld on the gauge-pin fixture (Ø20 pin through both Ø20.5 bores,
  Ø47 plugs in both pockets) per docs/34 §1.
- **MEASURE:** fixture re-check after welding; visual.
- **CRITERION:** coaxiality of the two Ø47 H7 pockets ≤ 0.1 mm;
  perpendicularity of the spindle axis to the base ≤ 0.15/300; no
  cracks/undercut.
- **RECORD:** build record.

### P3-3 Spool weld (P001 flanges → hub tube)

- **DO:** weld per docs/34 §2 (MIG 4 mm fillet, alternating quadrants).
- **MEASURE:** dial indicator at Ø400; bore-to-hub measurement; visual.
- **CRITERION:** flange runout ≤ 1.0 mm at Ø400; hub concentricity ≤ 0.5 to
  the Ø20.5 flange bores; mark the heavy point if runout is marginal.
- **RECORD:** build record.

### P3-4 Door-frame and G106 jamb weld (gate 15 physical)

- **DO:** weld the 5 mm G106 plate to the door jamb (2 × 40 mm fillets per
  side); mitre/weld the 20×20×2 door frame; drill the SLC pattern **only**
  from the Bernstein installation drawing p.8 (4×M5/Ø5.4, rows 161.6 mm /
  columns 22 mm), never from the bounding box (docs/34 §3, docs/26 §2).
- **MEASURE:** diagonal check; pattern measurement against the archived
  drawing.
- **CRITERION:** frame diagonal ≤ 2 mm; hole coordinates per the archived
  drawing p.8; door ≤ 15 kg before hinge fit.
- **RECORD:** build record.

---

## Phase 4 — Assembly torque/preload records

Applies and records the desk values ([ledger §2.2/§2.4](33-gate-ledger.md),
[docs/31 §7](31-fabrication-guide.md)). Closes gates 1, 2, 4, 6 and the
assembly part of 27.

| Joint | Torque to apply |
|---|---|
| H003 adapter M5×16 / M5×25, class 10.9 | **6 Nm** |
| F108 roller retaining M5×16, class 10.9 | **6 Nm** |
| F112 module M4×20, class 10.9; collar clamp M4 8.8 | **2.8 Nm** |
| Bed F301 M10×35, class 8.8 (into DIN 929) | **47 Nm** |
| Payoff spindle M20 nut | **finger-tight + 1/8 turn** against the tab washer |
| ifm E60137 wheel hub clamp (M4 class, detail unpublished) | **2.0 Nm placeholder** (confirm at receipt) |
| M3 (M3×8 / M3×10, class 8.8) | 1.2 Nm |
| R+W MK2 hub clamp screws | 4.5 Nm per R+W |
| Belt static proof | **5 Nm / 10 s hold / 10 cycles**, each direction |

Full table incl. M6 10 Nm / M8 24 Nm: docs/31 §7.

### P4-1 Apply and record each torque (gates 1, 4)

- **DO:** torque every listed joint during assembly per [docs/12 §6](12-feed-cartridge-fabrication.md)
  order.
- **MEASURE:** calibrated torque wrench; record wrench ID, uncertainty and
  date.
- **CRITERION:** values above; supplier value governs where provided.
- **RECORD:** [production-build-record-template.csv](inspection/production-build-record-template.csv).

### P4-2 H003 joint preload verification + washer/joint stiffness (gate 2 physical)

- **DO:** torque-preload test on spare parts to confirm the M5 joint reaches
  the 8.0 kN F0 target of the adapter study (docs/31 §7); measure the actual
  washer/joint contact stiffness (or run the two-body ISO7092 washer contact
  solve) required by the adapter record.
- **MEASURE:** load cell / instrumented bolt + DTI on the joint stack.
- **CRITERION:** measured preload and joint stiffness recorded and consistent
  with the resolved-pressure model inputs.
- **RECORD:** build record + adapter findings reference.

### P4-3 Measuring-wheel hub clamp (gate 6)

- **DO:** clamp the ifm E60137 wheel hub onto the M202 shaft (hub screw
  detail unpublished — confirm from the manufacturer drawing at receipt;
  2.0 Nm is the retained M4 cone-point placeholder only).
- **MEASURE:** calibrated torque driver.
- **CRITERION:** supplier value, or 2.0 Nm placeholder, recorded.
- **RECORD:** build record.

### P4-4 Collar clamp identity + torque (gate 1 physical)

- **DO:** record the clamp-screw identity and applied torque at the
  F108/M5×16 collar clamping step (docs/12:115, ledger §2.2).
- **CRITERION:** M5×16 10.9 at 6 Nm; 12.5 mm engagement; no washer beyond
  F108; M3 set screws never used as the torque connection.
- **RECORD:** build record.

### P4-5 Belt static proof test (gate 1 physical)

- **DO:** prove each belt drive on a rigid **non-motor** fixture
  (docs/12:152): 5 Nm in each direction, 10 s hold, 10 cycles, with a
  calibrated torque transducer; slip-mark the shaft/key/pulley/hub
  interfaces; inspect belt teeth and collar afterwards.
- **MEASURE:** calibrated torque transducer; slip-mark inspection.
- **CRITERION:** no relative slip at any marked interface; no belt-teeth or
  collar damage; no motor holding torque used as the reference; no pliers on
  journals.
- **RECORD:** build record.

### P4-6 R+W couplings

- **DO:** set each MK2 coupling at free length; mark both journals for
  complete 16 mm hub insertion; align before tightening the supports.
- **MEASURE:** DTI; insertion marks.
- **CRITERION:** ≤ 0.05 mm radial and ≤ 0.1° angular alignment; bellows never
  stretched; hub screws 4.5 Nm (docs/12:123).
- **RECORD:** build record.

### P4-7 Assembly-order compliance + free rotation

- **DO:** execute docs/12 §6 steps 1–10 in order (F112 off F102, MB2 before
  key, dowels after module insertion, rear endfloat 0.08–0.50 mm, front
  outer-ring clearance 0.020–0.110 mm, 52.5–64.5 mm belt centres, 1.000 mm
  deflection target force 13.8715–15.1215 N at 62.5 mm).
- **MEASURE:** feeler/DTI per step; hand rotation 360°.
- **CRITERION:** every step's criterion met; free rotation through 360° after
  each retention step; actual belt span used in the manufacturer formula.
- **RECORD:** build record.

---

## Phase 5 — Electrical checks

De-energised first, then the safety-function gates. Closes gates 18, 19, 20,
21, 22 (physical), 23, 27 and the EMC part of 41. Energisation of the machine
remains prohibited until this phase's records exist (docs/22 §10, docs/23 §4).

### P5-1 IF-B01 review, build and bare-board fixture (gate 18)

- **DO:** complete human electrical/mechanical review of the draft board;
   fabricator panel/stack-up confirmation; bare-board build; run the fixture
   with **only** `if-b01-factory-test.uf2`, J4–J7 disconnected.
- **MEASURE:** per the [fixture procedure](../hardware/if-b01/if-b01-test.md).
- **CRITERION:** fixture pass; ERC/DRC/parity clean; reviewer signatures.
- **RECORD:** archive fixture logs + FAI template rows.

### P5-2 Cabinet de-energised inspection (gates 21, 27)

- **DO:** execute docs/23 §3 table: point-to-point continuity against the
  approved schematic, segregation/clearance and screen inspection, terminal
  torque and marking, protection-device configuration review.
- **MEASURE:** multimeter (continuity), torque driver, visual/clearance
  gauges.
- **CRITERION:** every docs/23 §3 row filled and matching the schematic;
  terminal designations TB24-01…PE-BAR correct.
- **RECORD:** build record + docs/23 §3 table.

### P5-3 PE continuity and insulation (gate 21)

- **DO:** bond test every PE bond (copper PE bar, M6 bonds, PE ≥ 1.5 mm² per
  docs/22 §11.4); insulation test per the approved plan under IEC 60204-1 as
  applied locally.
- **MEASURE:** low-ohmmeter (4-wire); insulation tester at the plan's test
  voltage.
- **CRITERION:** PE continuity ≤ **0.1 Ω** per bond; insulation readings
  within the approved plan's limits; instrument IDs recorded.
- **RECORD:** build record + FAI template.

### P5-4 E-stop/guard channels, faults, reset/EDM (gate 23)

- **DO:** test each E-stop channel, both guard contact paths (11-12/21-22
  interlock; 31-32/41-42 locked state), cross-wire faults, reset/EDM
  behaviour and each contactor fault mode per the reviewed test plan
  (docs/22 §8.4).
- **MEASURE:** multimeter + approved fault-injection fixtures.
- **CRITERION:** opening either channel/contact/wire removes the XPS safety
  outputs; reset/EDM per the XPS configuration (app function 1, start 3);
  the 20 ms XPS component response is **not** treated as machine stop time.
- **RECORD:** FAI template rows.

### P5-5 K1/K2/K3 DC-duty opening verification (gate 19 physical)

- **DO:** measure the actual opening behaviour of each LC1D12BD
  three-series-pole chain on hardware (docs/22 §8.5).
- **MEASURE:** oscilloscope on contactor coil/contact and branch voltage.
- **CRITERION:** recorded opening response consistent with the qualified
  catalogue duty (20 A at ≤ 75 V, DC-1/DC-2..5; margins 2.06×/3.45×); the
  critical-current durability note stays an open observation.
- **RECORD:** build record + FAI template.

### P5-6 DC-bus decay, prospective fault, inrush (gates 20, 22 physical)

- **DO:** measure each drive bus decay curve after K1/K2/K3 open (with the
  bleed resistors fitted); record actual bus capacitance; measure inrush and
  prospective short-circuit coordination.
- **MEASURE:** isolated HV probe + scope; clamp meter for inrush.
- **CRITERION:** measured decay reaches the risk-assessed safe voltage; set
  T2 = measured decay + ≥ 1 s margin (≈3 s proposal with the 2.2 kΩ/5 W
  bleeds, τ ≈ 1.1 s); the 0.45 s desk estimate is replaced by the measured
  curve.
- **RECORD:** build record + FAI template.

### P5-7 Stop-time measurement (gate 23)

- **DO:** measure stop time from E-stop actuation to standstill for the
  worst-case motion state, with guard channel actuation repeated.
- **MEASURE:** calibrated time capture (scope/PLC) + motion marker.
- **CRITERION:** measured stop time satisfies the task risk assessment;
  recorded per axis.
- **RECORD:** FAI template.

### P5-8 Regenerated-energy confirmation (gate 23)

- **DO:** confirm regenerated-energy behaviour on stops against the desk
  analysis (docs/22 §11.2).
- **MEASURE:** bus voltage recording during worst-case stops.
- **CRITERION:** measured stop energy ≤ bus storage (≥ 0.18 J desk figure
  verified with measured bus C); no bus overvoltage event.
- **RECORD:** build record.

### P5-9 EMC assessment (gate 41 physical)

- **DO:** enclosure/standards review of segregation, shield terminations and
  ducts, then an EMC assessment on the built cabinet by competent persons.
- **RECORD:** build record / FAI template.

---

## Phase 6 — Motion commissioning

Closes gates 11, 12, 13 (physical), 24, 25 (force records), 28, 29.
Staged energisation per docs/24 §7.

### P6-1 Staged energisation (gate 24)

- **DO:** under competent oversight: verify actual voltages, polarity, field
  contacts, encoder direction, drive enable behaviour, interlocks, fault
  response and restart prevention.
- **MEASURE:** calibrated multimeter/scope; record every instrument.
- **CRITERION:** every docs/24 §7.3 item verified on hardware; no
  extrapolation from host tests.
- **RECORD:** build record + FAI template.

### P6-2 Run-in

- **DO:** run in the KK86 carriages and feed drives; verify belt alignment
  and minimum tooth engagement on both pulleys (docs/30 P4/P5).
- **MEASURE:** DTI along travel; visual belt check.
- **CRITERION:** carriage travel parallel to X within the stage
  specification; belts aligned; engagement per MISUMI records.
- **RECORD:** build record.

### P6-3 Wire path (docs/31 §4)

- **DO:** taut-wire check with 1.2 and 4.5 mm wire: payoff → left guide →
  front nip → cut plane → rear nip → right guide → output.
- **CRITERION:** zero unintended contact along the 880 mm route for both ODs.
- **RECORD:** build record.

### P6-4 Nip preload 50–75 N per side (gate 12)

- **DO:** lift each upper cartridge pair with a calibrated force gauge at the
  roller axis until the wire releases; both sides, both ODs (docs/20 step 3).
- **MEASURE:** calibrated force gauge, U ≤ 1.0 N.
- **CRITERION:** 50–75 N per nip, guard-banded by gauge U.
- **RECORD:** force-records CSV (P10-3 rows) + build record.

### P6-5 Measuring-wheel contact force 6–19 N (gates 12, 28)

- **DO:** measure the wheel force the same way under the arm hub; both ODs.
- **MEASURE:** calibrated force gauge, U ≤ 1.0 N.
- **CRITERION:** 6–19 N, guard-banded.
- **RECORD:** force-records CSV + build record.

### P6-6 ±60 N roller-force envelope (gate 12)

- **DO:** qualify pressure/friction/jam behaviour on the assembled cassette
  across the ±60 N roller-force envelope (docs/verification/feed-shaft-fem.md:116
  context).
- **MEASURE:** force gauge + motion check per the qualification plan.
- **CRITERION:** no jam, pressure or friction failure within the envelope;
  torque alone is not accepted as proof (5 Nm at r = 20 mm ≠ force limit).
- **RECORD:** build record + qualification inputs.

### P6-7 Encoder direction and calibration (gate 29)

- **DO:** verify encoder direction (docs/24 §7.3), then execute the
  [docs/13 §5](13-measuring-interfaces-and-calibration.md) calibration:
  traceable independent reference, synchronous capture, all 128-count
  intervals in both directions, fit + held-out verification, bind to IDs/Z
  phase/recipe.
- **MEASURE:** traceable displacement reference with recorded uncertainty.
- **CRITERION:** per held-out record |x_pred − x_ref| + U_ref ≤ 0.025 mm +
  0.0001·|x_ref|; complete-turn-only fitting must fail;
  `--require-factory-accuracy` passes only after this + the P0-4
  circumference verification; invalidate on wheel/encoder/coupling change
  or range breach.
- **RECORD:** measurement-analysis outputs + build record.

### P6-8 Blade pair loaded clearance + edge qualification (gate 13 physical)

- **DO:** verify loaded clearance before installation and qualify the edge
  per docs/24 §4.4; install only afterwards.
- **RECORD:** build record + FAI template.

### P6-9 Rotating fatigue qualification (gate 11)

- **DO:** execute an approved rotating multiaxial fatigue plan on the actual
  shaft/build (docs/15:124 requirement).
- **MEASURE:** per the approved plan (cycles, periodic inspection).
- **CRITERION:** per the approved plan; no FEM number releases the thread;
  a demonstrated life must be established for the machine's duty.
- **RECORD:** build record + plan archive.

---

## Phase 7 — Payoff

Closes gates 16 and the payoff part of 42. Evidence on the
[guard/payoff release template](inspection/guard-payoff-release-template.csv).

### P7-1 Full-spool mass (gate 16)

- **DO:** weigh the first full spool per wire lot.
- **MEASURE:** calibrated scale.
- **CRITERION:** ≤ **30 kg** until the deflection re-check (docs/28 §2.1);
  above → re-check before use.
- **RECORD:** guard/payoff template.

### P7-2 Spindle deflection under load (gate 16)

- **DO:** measure P002 deflection with the full spool fitted.
- **MEASURE:** DTI at mid-span.
- **CRITERION:** against the desk bounds 0.65 mm / 52 MPa at 30 kg
  (docs/28 §2.1); C45 certificate governs; deflection changes payout
  geometry, not length accuracy.
- **RECORD:** guard/payoff template.

### P7-3 Brake adjustment trial (gate 16)

- **DO:** set the P007 pad pressure with the M8 adjuster to the working
  payout tension (docs/26 §3); record the setting.
- **CRITERION:** stable payout tension by trial; the brake is **not** a
  safety or holding brake — record this in the template.
- **RECORD:** guard/payoff template.

### P7-4 Runaway/overspeed measurement (gate 16)

- **DO:** measure runaway and overspeed behaviour with the brake set
  (docs/26 §6.3).
- **MEASURE:** tachometer / high-speed capture.
- **CRITERION:** recorded runaway/backlash behaviour; sign-off row completed.
- **RECORD:** guard/payoff template.

### P7-5 Spool float and rotation

- **DO:** confirm 0.1–0.3 mm end float and free rotation; confirm P3-2/P3-3
  weld rows are attached (docs/30 P7).
- **RECORD:** build record.

---

## Phase 8 — Guard

Closes gates 14, 15, 17, 22 (physical) and the guard part of 42. Evidence on
the [guard/payoff release template](inspection/guard-payoff-release-template.csv).

### P8-1 SLC/ACF-1 physical fit and locking travel (gate 15 physical)

- **DO:** mount the SLC body on G106 and the ACF-1 on the door; verify
  insertion depth, alignment and locking travel (docs/26 §2, §6.2).
- **MEASURE:** feeler/DTI on the actuator window.
- **CRITERION:** alignment window 29 + 3 mm in X/Y; 2 mm overtravel with the
  stop set inside 0.2–2 mm; no lateral force on the head; ACF-1 approach
  radius ≥ 400 mm respected.
- **RECORD:** guard/payoff template.

### P8-2 Safety-distance reconfirmation (gate 14 physical)

- **DO:** reconfirm the Ø30 mm aperture reach-through against the current
  EN ISO 13857 edition and the completed risk assessment.
- **MEASURE:** dimensional check of panel-to-hazard distance.
- **CRITERION:** 345 mm > 120 mm slot reach (≈2.9× desk margin, docs/28
  §2.2); residual annular opening ≤ 12.75 mm with wire installed; current
  edition cited.
- **RECORD:** guard/payoff template.

### P8-3 Task-specific risk-assessment sign-off (gate 14)

- **DO:** complete the task-specific risk assessment: cutting, wire release,
  entanglement, ejection, access, maintenance, loading, loss of power,
  emergency egress; obtain competent-person sign-off.
- **CRITERION:** every hazard row dispositioned and signed.
- **RECORD:** guard/payoff template.

### P8-4 PC door impact and hardware (gate 17)

- **DO:** impact verification of the 5 mm PC front door + hinge/handle
  inspection (docs/26 §6.6).
- **RECORD:** guard/payoff template.

### P8-5 E1/E2 release-permission fault injection (gate 22 physical)

- **DO:** physically validate the R2 + K1–K3 NC chain + T2 on-delay design
  (docs/22 §11.8) by fault injection: contactor-failure, timer-failure and
  power-loss cases.
- **CRITERION:** unlock energises only when XPS outputs are off **and** all
  three mirror contacts report open **and** T2 has elapsed; power loss keeps
  the lock locked. Until sign-off, access after a trip stays a LOTO
  maintenance procedure.
- **RECORD:** FAI template + guard/payoff template.

### P8-6 Aperture treatment

- **DO:** confirm edge grommets fitted and aperture edges burr-free
  (docs/26 §2).
- **RECORD:** guard/payoff template.

---

## Phase 9 — Thermal

Closes gate 10 (instrumented cold-start/hot-run + motor-case temperature) and
the cabinet thermal part of docs/22 §11.5.

### P9-1 Instrumented cold-start/hot-run (gate 10)

- **DO:** instrument the built cartridge and run the cold-start/hot-run test
  of docs/12:38 with the worst-case standstill hold (30 W motor loss,
  docs/28 §2.5).
- **MEASURE:** calibrated thermocouples on front/rear bearing seats and
  housing; logging during run.
- **CRITERION:** measured seat temperature rise within the record's explicit
  20 K shaft-above-housing envelope (the desk FEM's 29.66 K prediction is
  not met — the physical measurement is the closure, ledger §2.5); S5M
  thermal break and rear-path mitigations do not relax the number.
- **RECORD:** build record + FAI template.

### P9-2 Motor-case temperature at rated hold (gate 10)

- **DO:** record the motor-case temperature at rated hold and apply the
  [bearing-fit](../engineering/generated/bearing-fit/report.md) conduction
  calculation (docs/28 §2.5).
- **MEASURE:** calibrated contact/IR thermometer.
- **RECORD:** build record.

### P9-3 Cabinet temperature mapping

- **DO:** map cabinet temperature at maximum duty against the ≈152 W vs
  280 W L35 desk budget (docs/22 §11.5).
- **MEASURE:** thermocouple mapping inside the enclosure.
- **CRITERION:** internal temperature within the L35 (or derated) budget.
- **RECORD:** build record.

---

## Phase 10 — First article: 30-piece Cpk qualification

Closes gates 25, 26 and 43. Procedure authority: [docs/20](20-physical-qualification.md).

### P10-1 Generate templates

- **DO:** `starz_qualification --template <dir>` (writes 120 length rows,
  6 force rows).
- **RECORD:** generated CSVs.

### P10-2 Machine setup

- **DO:** fit F206 inserts for the OD under test; set AJ01/AJ02 to the
  protrusions in [mechanism status.json](../engineering/generated/mechanism/status.json);
  set M210 so the 626 roller just touches the wire.
- **RECORD:** build record.

### P10-3 Force rows (gates 12, 28, 25)

- **DO:** enter the six force rows measured in P6-4/P6-5 with gauge U.
- **CRITERION:** nip 50–75 N, wheel 6–19 N, force gauge U ≤ 1.0 N, guard-banded.
- **RECORD:** force-records CSV.

### P10-4 Length cells (gate 25)

- **DO:** cut and measure 30 pieces per mandatory cell: OD 1.2 and 4.5 mm ×
  commanded 200 and 1000 mm; do not discard pieces.
- **MEASURE:** calibrated scale or CMM at recorded temperature, 15–30 °C;
  length gauge U ≤ 0.05 mm.
- **CRITERION:** error ≤ ±(0.05 % + 0.20 mm) after subtracting gauge U;
  **Cpk ≥ 1.33** per cell.
- **RECORD:** length-records CSV.

### P10-5 Qualify and archive (gate 25)

- **DO:** `starz_qualification --lengths <csv> --forces <csv> --out <dir>`;
  archive the output directory with the raw CSVs.
- **CRITERION:** PASS per cell; `physical_results` true only for non-`SYN-`
  gauge and sample identities.
- **RECORD:** length-cells.csv, force-checks.csv, report.md, status.json.

### P10-6 First-article + energisation gate (gate 26)

- **DO:** complete [first-article-and-energisation-gate-template.csv](inspection/first-article-and-energisation-gate-template.csv)
  with the evidence references from phases 0–9 and P10-1–P10-5.
- **CRITERION:** every required evidence row present with reviewer
  disposition.
- **RECORD:** the FAI template.

### P10-7 Integration flag flips (gate 43)

- **DO:** assemble per [docs/30](30-machine-assembly-sequence.md)/[docs/31](31-fabrication-guide.md)
  and flip mechanism status flags **only** with the physical evidence from
  phases 0–9.
- **CRITERION:** no flag flips without physical evidence; keep raw failed
  data.
- **RECORD:** status.json + build record.

### P10-8 Repository checks (gate 27)

- **DO:** run `tools/check_production_records.py` (and the design-integrity
  checks) after all records are entered; fix any finding; archive.
- **CRITERION:** tools pass.
- **RECORD:** tool outputs archived with the build record.

---

## Closeout note

A gate is closed only when its step(s) above are executed and the named
RECORD contains a measured pass with reviewer disposition. This runbook does
not close, release or relax anything by itself; it is the ordered path to do
so physically.
