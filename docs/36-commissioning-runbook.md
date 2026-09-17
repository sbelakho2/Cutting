# Commissioning runbook: physical closeout of every gate, in order

**Revision:** `S1-B-RB-R1-2026-09-16`

This runbook is the single ordered execution plan that closes every physical-only
row of the [gate ledger](33-gate-ledger.md), in sequence. It closes nothing by
itself. A gate closes only when you execute a step and fill its record with a
measured pass. The runbook points to the existing procedures and record
templates instead of copying their text, and no value written here overrides
the governing document it cites.

## How to use this runbook

1. Work through phases 0 to 10 in order. A missing record blocks every
   dependent step. docs/31 §7 carries the warning, and docs/23 §3 says a blank
   cell is an open hold, not a pass.
2. Every step has five fields. **What you need** lists the instruments and
   sizes. **What you do** describes the actions in plain words. **What you
   check** gives the number to meet. **Where you write it down** names the
   record. **If it fails** tells you the most common failure and the quickest
   fix. Where the governing record already defines the method, the step cites
   that record instead of repeating it.
3. The desk torque and preload values ([ledger §2.2/§2.4](33-gate-ledger.md),
   [docs/31 §7](31-fabrication-guide.md)) are applied in phase 4 and recorded
   in the build record. They are desk conversions, not assembly authorisations.
4. Phases 0 to 4 assemble the machine. Phase 5 closes the electrical and
   safety gates before energisation. Phases 6 to 9 commission motion, payoff,
   guard and thermal behaviour. Phase 10 is the
   [30-piece Cpk qualification](20-physical-qualification.md). No first article
   runs before phases 0 to 9 are recorded.
5. The templates you will write into are the
   [incoming inspection](inspection/incoming-inspection-template.csv), the
   [production build record](inspection/production-build-record-template.csv),
   the [F101 R6 inspection](inspection/F101-R6.csv), the
   [M201 inspection](../engineering/generated/measurement/M201-inspection-template.csv),
   the [guard/payoff release](inspection/guard-payoff-release-template.csv) and
   the [first article + energisation gate](inspection/first-article-and-energisation-gate-template.csv).

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

## Phase 0: Incoming inspection

Everything arrives before anything is machined or assembled. This phase feeds
the [incoming-inspection template](inspection/incoming-inspection-template.csv)
(gate 27) and the physical halves of gates 1, 13, 15 and 33.

### P0-1 Unpack, identify, certificate

- **What you need:** the exact identities in [docs/31 §1](31-fabrication-guide.md)
  and the sourcing lists, plus a counting sheet.
- **What you do:** unpack and verify every ordered row against the exact
  identities in docs/31 §1 and the sourcing lists. Collect the certificates or
  CofC and the lot and serial numbers.
- **What you check:** identity, quantity, lot/serial and certificate are
  present for every row. Any deviation opens a nonconformance row.
- **Where you write it down:** [incoming-inspection-template.csv](inspection/incoming-inspection-template.csv).
- **If it fails:** a missing certificate or serial number is the usual gap, so
  open the nonconformance row before you accept the part.

### P0-2 F110 UMCL40-10-20-MC collar bore (gate 1 physical)

- **What you need:** a calibrated three-point bore gauge or bore micrometer.
- **What you do:** measure the incoming bore before any keyway machining
  (docs/12:111–113).
- **What you check:** bore 10.000–10.030 mm. The keyway (3.04 ± 0.01 wide,
  2.35 ± 0.025 from the actual bore) is cut after acceptance, not assumed.
- **Where you write it down:** the incoming template. The keyway machining
  result goes in later, in P4.
- **If it fails:** the bore is usually recorded after the keyway exists, so
  stop and measure it before any keyway machining.

### P0-3 Bearing bores measured for matched fitting (gate 10 physical)

- **What you need:** a bore gauge or calibrated comparator at 20 °C, with a
  combined diameter uncertainty target of ≤ 0.5 µm for the matched seats.
- **What you do:** measure every 6001-2Z/C3 and 6002-2Z/C3 bore (plus OD and
  width), the 626-2Z and the 6204-2Z/C3. Assign permanent IDs for
  match-marking.
- **What you check:** the recorded values feed the
  [matching-limits](../engineering/generated/bearing-fit/matching-limits.csv)
  and the [256-corner fit record](../engineering/generated/bearing-fit/report.md).
  No seat may be finished to nominal j5 without a measured bore (docs/12:36).
- **Where you write it down:** the incoming template, then the
  matching-limits.csv workflow.
- **If it fails:** finishing a seat before measuring its bore is the classic
  mistake, so always start from the measured bore value.

### P0-4 ifm E60137 wheel 200 mm circumference claim (gate 33 physical)

- **What you need:** a traceable calibrated reference for the roll/tape method
  of [docs/13 §5](13-measuring-interfaces-and-calibration.md). Record the
  reference identity and uncertainty.
- **What you do:** verify the manufacturer circumference claim at receipt.
  The part is the ifm E60137 "U:200 N:06", Ø63.66 mm, 200 mm circumference,
  and ifm publishes no tolerance. Also confirm the hub and clamping-screw
  detail.
- **What you check:** 200 mm, with an assumed ±0.2 mm acceptance band of
  199.8–200.2 until a manufacturer tolerance exists.
- **Where you write it down:** the incoming template. Keep the result, because
  P6-7 uses it as an input.
- **If it fails:** the hub clamping-screw detail is often missing from the
  paperwork, so confirm it now or the P4-3 clamp torque stays a placeholder.

### P0-5 Blade pair T001 (gate 13 physical)

- **What you need:** the drawing and your dimensional and visual checking kit.
- **What you do:** receive the purchase-to-drawing blade pair and verify it
  against the drawing. Store it uninstalled.
- **What you check:** the pair is drawing-conformant. Installation is blocked
  until P6-8, the loaded clearance plus edge qualification (docs/24 §4.4).
- **Where you write it down:** the incoming template.
- **If it fails:** the pair usually gets fitted too early, so keep it off the
  machine until P6-8 is recorded.

### P0-6 Keys and fasteners identity (gate 5 confirmation)

- **What you need:** a micrometer and thread gauges.
- **What you do:** measure the received F109 keys and pulley keys, fasteners
  and washers.
- **What you check:** F109 3×4×18 (width 2.975–3.000, height 3.950–4.000,
  length 17.90–18.00). DIN 6885-A 5×5×28 pulley key. Fastener classes per
  docs/12 §3/§5.
- **Where you write it down:** the incoming template.
- **If it fails:** an undersize key width is the usual catch, so check the
  2.975–3.000 mm band before you rely on the key.

### P0-7 Electrical/controls identities and revisions

- **What you need:** the controlled records and
  [docs/22 §2–§3, §11](22-controls-and-wiring.md).
- **What you do:** verify the XPSUAF13AP, the 3SU1851 E-stop, the
  SLC-F-024-20/20-R4 with ACF-1, the LC1D12BD ×3, the PSUs, the ifm RU3100
  encoders, the CL57T/CL86T drives, the IF-B01 board, the fused
  terminals/links (ESKA 522620 2 A / 522617 1 A in 3046032) and the incoming
  iC60N + iID against docs/22 §2–§3, §11. Archive all manuals, including the
  Bernstein installation manual 0800000865.
- **What you check:** an exact match. Stop if any identity or revision differs
  (docs/23 §1).
- **Where you write it down:** the incoming template.
- **If it fails:** a revision mismatch is the common stop, so check the
  revision against the controlled record before you accept the part.

---

## Phase 1: Matched bearing measurement and seat finishing

This phase closes the physical half of the bearing-fit gate (gate 10) and the
626 thermal/matching decision (gate 29). The governing requirements are
docs/12 §2.

### P1-1 Measure and match seats to assigned bores

- **What you need:** a calibrated micrometer, taking two angular positions ×
  three axial locations per seat (docs/12:150).
- **What you do:** measure each assigned bearing bore, then finish its shaft
  seat to the intersection of the j5 limits and 3–6 µm diametral interference.
- **What you check:** the rear 12j5 seat at 11.997–12.005 **and** 3–6 µm
  interference with the assigned 6001 bore. The front 15j5 seat at
  14.997–15.005 **and** 3–6 µm with the assigned 6002 bore. With combined
  U ≤ 0.5 µm you accept measured interference 3.5–5.5 µm (guard-banded).
- **Where you write it down:** [F101-R6.csv](inspection/F101-R6.csv) rows
  `rear_bearing_seat`, `front_bearing_seat`,
  `measured_interference_guardband`, plus matching-limits.csv.
- **If it fails:** skipping the measured bore and finishing to nominal j5 is
  the classic mistake, so always start from the assigned bore value.

### P1-2 Seat form and finish

- **What you need:** form metrology (cylindricity and runout to the common
  axis A) and a profilometer.
- **What you do:** finish both seats in a common setup. Outsource the
  cylindrical finishing if the shop cannot demonstrate the diameters and
  runout.
- **What you check:** cylindricity ≤ 0.001 mm. Total radial runout ≤ 0.002 mm
  to A. Ra ≤ 0.4 µm (docs/12:34).
- **Where you write it down:** F101-R6.csv rows `seat_cylindricity`,
  `seat_total_runout`, `bearing_Ra`.
- **If it fails:** runout to axis A usually fails first when the seats are not
  cut in one setup, so keep the common setup.

### P1-3 Match-mark and protect

- **What you need:** permanent match marks for the assigned pairs and spares.
- **What you do:** match-mark the assigned pairs and spares. Do not use
  Loctite retainer (docs/12:36). Do not copy the j5/C3 feed-rotor match onto
  the 626 CN measurement bearings (docs/13:67).
- **What you check:** the pairs and spares are marked, no retainer is used,
  and the 626 CN bearings carry no j5/C3 match.
- **Where you write it down:** the F101-R6.csv `identity` row.
- **If it fails:** copying the j5/C3 match onto the 626 CN bearings is the
  common error, so mark them separately.

### P1-4 Housing bores: clearance fit confirmation (gate 10 physical)

- **What you need:** a bore gauge.
- **What you do:** measure the F112 front bore and the F102 rear bore with the
  pair clamped.
- **What you check:** F112 Ø32.008–32.019, which gives 8–28 µm diametral
  clearance to the 6002 OD with no housing interference. The record governs,
  ledger §2.5. F102 Ø28.008–28.018.
- **Where you write it down:** the build record.
- **If it fails:** a housing interference is the common miss, so check the
  clearance to the actual OD before you clamp.

### P1-5 626 CN thermal screen (gate 29 physical)

- **What you need:** the installed 626 clearance, measured before wheel
  assembly, per docs/13:67.
- **What you do:** measure the installed 626 clearance before wheel assembly
  and decide the thermal/matching treatment or a substitution design.
- **What you check:** the CN + 10 K ΔT stack leaves 0.5 µm before any
  press-fit loss (docs/13:67). Resolve this by measurement or design. Do not
  accept it by assumption.
- **Where you write it down:** the build record.
- **If it fails:** accepting the clearance by assumption is the failure mode,
  so measure it before the wheel goes on.

---

## Phase 2: Machining/finishing checks (F101, H003, M201)

This phase closes gates 8 (true thread roots), 9 (edge breaks), 27 (build
records) and the M201 part of gate 28.

### P2-1 F101 full dimensional inspection

- **What you need:** the per-row method in the template, which is micrometer,
  gauges, optical profile and datum-referenced rotation + DTI.
- **What you do:** complete every row of [F101-R6.csv](inspection/F101-R6.csv)
  against the two A3 sheets (docs/12 §2, §7).
- **What you check:** all template rows pass. Key runouts ≤ 0.010 mm to A.
  B-face perpendicularity ≤ 0.004 mm. Key-seat and MB2-slot geometry per
  docs/12 §2.
- **Where you write it down:** F101-R6.csv.
- **If it fails:** a skipped row is the common gap, so work through the
  template row by row and do not leave any empty.

### P2-2 M5 blind hole: true thread roots (gate 8)

- **What you need:** optical tool measurement of tip-to-first-full-form **a**
  with uncertainty, a thread depth gauge and 6H GO/NO-GO gauges. The gauge's
  own lead is not counted.
- **What you do:** execute the [docs/14](14-f101-tapping-operation.md)
  operation. Accept the E412M5 physical tip. Drill Ø4.20+0.03/0 to
  18.00 ± 0.05 cylindrical depth plus a 118° ± 2° point. Prove the same-lot
  coupon. Then tap.
- **What you check:** guarded a ≤ 3.18 mm. At least 0.50 mm cylindrical bottom
  clearance at the deepest permitted insertion, with no drill-point credit.
  14.00–14.05 mm full-form thread after the actual C0.20 entry. 6H GO/NO-GO
  pass (docs/14 §2).
- **Where you write it down:** F101-R6.csv rows `M5_*` plus the tool
  acceptance record.
- **If it fails:** counting the drill point as clearance is the common error,
  so the 0.50 mm must come from cylindrical depth only.

### P2-3 M15×1 thread: true thread roots (gate 8)

- **What you need:** 6g GO/NO-GO gauges and a full-profile length measurement.
- **What you do:** verify the KM2/MB2 retention thread.
- **What you check:** full-profile thread over S65.10–71.30. Entry
  C0.50 ± 0.02. Ø13.50 ± 0.02 runout relief with R0.20 roots. No overrun into
  the bearing seat (docs/12 §2).
- **Where you write it down:** F101-R6.csv rows `M15_*`.
- **If it fails:** overrun into the bearing seat is the usual failure, so
  watch the thread end against the seat land.

### P2-4 Final edge breaks (gate 9)

- **What you need:** optical plus visual inspection.
- **What you do:** controlled 0.05–0.10 mm deburr of the remaining slot
  openings (docs/12:58).
- **What you check:** a 0.05–0.10 mm break. No raised lips. No load-flank
  rounding. Bearing-seat lands untouched.
- **Where you write it down:** the F101-R6.csv `remaining_slot_edges` row plus
  the build record.
- **If it fails:** rounding the load flank during deburr is the common error,
  so keep the tool away from the loaded flank.

### P2-5 H003 motor adapters (×2)

- **What you need:** CMM, bore gauge or micrometer.
- **What you do:** machine and inspect per the H003 R6 sheet (docs/31 §2).
- **What you check:** F0 register Ø49.97 ± 0.01. Pilot Ø73.10 +0.02/0. Faces
  and counterbores per the sheet. 4× M5×16 class 10.9 + ISO7092 on PCD70.
- **Where you write it down:** the H003 sheet record or build record. The
  joint verification follows in P4-2.
- **If it fails:** the F0 register diameter is the tight number, so verify
  Ø49.97 ± 0.01 before assembly.

### P2-6 M201 encoder plates (×2) (gate 28)

- **What you need:** metrology per the M201 template. Send the work out for
  the instruments the workshop does not have.
- **What you do:** machine and inspect per
  [docs/13 §2](13-measuring-interfaces-and-calibration.md).
- **What you check:** central bore Ø20.040–20.060 through. Cylindricity 0.01.
  Perpendicularity 0.02 to A. Hole positions Ø0.10 to A|B|C. Flatness 0.02.
  Ra per the sheet.
- **Where you write it down:** [M201-inspection-template.csv](../engineering/generated/measurement/M201-inspection-template.csv).
- **If it fails:** hole position to A|B|C is the common miss, so datum the
  inspection the way the drawing does.

---

## Phase 3: Weld inspection (posts/base, spool, frame, jamb)

This phase closes the physical weld inspection behind gate 7 and feeds gates
15, 16 and 42. The procedures and fixtures live in
[docs/34](34-weld-and-assembly-procedures.md).

### P3-1 Frame welds (F304/F303/F302/F305)

- **What you need:** a visual check plus a straightedge and DTI on the four
  foot pads.
- **What you do:** weld legs to rails, rails to cross rails and feet to legs.
  Check for full perimeter and no undercut (docs/30 P1).
- **What you check:** full perimeter welds with no undercut. The four foot
  pads coplanar within ≤ 1 mm over the 1080 × 440 rectangle.
- **Where you write it down:** the build record.
- **If it fails:** pad coplanarity over the 1080 × 440 rectangle is the common
  fail, so check the four pads with the straightedge before the frame leaves
  the bench.

### P3-2 Payoff post-to-base weld (P004/P005 → P003)

- **What you need:** the gauge-pin fixture from docs/34 §1, which is the Ø20
  pin through both Ø20.5 bores and the Ø47 plugs in both pockets. You also
  need the fixture re-check after welding.
- **What you do:** weld on the gauge-pin fixture per docs/34 §1.
- **What you check:** coaxiality of the two Ø47 H7 pockets ≤ 0.1 mm.
  Perpendicularity of the spindle axis to the base ≤ 0.15/300. No cracks and
  no undercut.
- **Where you write it down:** the build record.
- **If it fails:** welding off the fixture is the classic error, so keep the
  pins and plugs in place for the whole weld.

### P3-3 Spool weld (P001 flanges → hub tube)

- **What you need:** a dial indicator at Ø400, a bore-to-hub measurement and a
  visual check.
- **What you do:** weld per docs/34 §2, which is a MIG 4 mm fillet in
  alternating quadrants.
- **What you check:** flange runout ≤ 1.0 mm at Ø400. Hub concentricity ≤ 0.5
  to the Ø20.5 flange bores. Mark the heavy point if runout is marginal.
- **Where you write it down:** the build record.
- **If it fails:** welding the quadrants in one pass warps the flange, so
  alternate quadrants and let it cool between passes.

### P3-4 Door-frame and G106 jamb weld (gate 15 physical)

- **What you need:** a diagonal check and a pattern measurement against the
  archived drawing.
- **What you do:** weld the 5 mm G106 plate to the door jamb with 2 × 40 mm
  fillets per side. Mitre and weld the 20×20×2 door frame. Drill the SLC
  pattern **only** from the Bernstein installation drawing p.8 (4×M5/Ø5.4,
  rows 161.6 mm / columns 22 mm), never from the bounding box (docs/34 §3,
  docs/26 §2).
- **What you check:** frame diagonal ≤ 2 mm. Hole coordinates per the archived
  drawing p.8. Door ≤ 15 kg before hinge fit.
- **Where you write it down:** the build record.
- **If it fails:** drilling from the bounding box instead of the p.8 pattern
  is the common error, so open the archived drawing and copy the coordinates
  from it.

### P3-5 Frame resonance verification (gate 45)

- **What you need:** an instrumented hammer or a soft-faced tap plus an
  accelerometer/phone-vibration logger, or a DTI decay check, on the
  assembled P1 frame with the bed bolted (docs/40 §2).
- **What you do:** tap the bed centre and the H005 plate and record the
  dominant ring-down frequencies. Repeat with the drive loads bolted in
  place.
- **What you check:** against the [docs/40](40-resonance-and-structural-audit.md)
  desk predictions — bed mode 245–368 Hz, rail composite 82–123 Hz, cut-axis
  plate 98–352 Hz, frame sway 19–38 Hz. Acceptance: the measured bed mode is
  ≥ 2× away from the 199/200/300 Hz excitation lines and the cut-axis plate
  ≥ 2× away from 500 Hz at the commissioning feed.
- **Where you write it down:** the build record.
- **If it fails:** install the pre-approved contingencies — the 3–5 mm damped
  bed–rail interface pad (with M10×40 screws, per docs/40 §3) for the bed
  mode, or the H005 stiffening rib for the plate mode — and re-measure.

---

## Phase 4: Assembly torque/preload records

This phase applies and records the desk values ([ledger §2.2/§2.4](33-gate-ledger.md),
[docs/31 §7](31-fabrication-guide.md)). It closes gates 1, 2, 4, 6 and the
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

The full table, including M6 at 10 Nm and M8 at 24 Nm, lives in docs/31 §7.

### P4-1 Apply and record each torque (gates 1, 4)

- **What you need:** a calibrated torque wrench. Record the wrench ID,
  uncertainty and date.
- **What you do:** torque every listed joint during assembly, in the
  [docs/12 §6](12-feed-cartridge-fabrication.md) order.
- **What you check:** the values in the table above. Where a supplier value is
  given, that supplier value governs.
- **Where you write it down:** [production-build-record-template.csv](inspection/production-build-record-template.csv).
- **If it fails:** skipping the wrench ID and uncertainty is the common record
  gap, so write them down before you start torquing.

### P4-2 H003 joint preload verification + washer/joint stiffness (gate 2 physical)

- **What you need:** a load cell or instrumented bolt plus a DTI on the joint
  stack.
- **What you do:** run the torque-preload test on spare parts to confirm the
  M5 joint reaches the 8.0 kN F0 target of the adapter study (docs/31 §7).
  Measure the actual washer/joint contact stiffness, or run the two-body
  ISO7092 washer contact solve required by the adapter record.
- **What you check:** the measured preload and joint stiffness are recorded
  and consistent with the resolved-pressure model inputs.
- **Where you write it down:** the build record plus the adapter findings
  reference.
- **If it fails:** the joint not reaching 8.0 kN F0 on the spares is the
  common fail, so test the spares before the real assembly.

### P4-3 Measuring-wheel hub clamp (gate 6)

- **What you need:** a calibrated torque driver.
- **What you do:** clamp the ifm E60137 wheel hub onto the M202 shaft. The hub
  screw detail is unpublished, so confirm it from the manufacturer drawing at
  receipt. 2.0 Nm is the retained M4 cone-point placeholder only.
- **What you check:** the supplier value, or the 2.0 Nm placeholder, is
  recorded.
- **Where you write it down:** the build record.
- **If it fails:** torquing the placeholder without confirming the supplier
  value is the common error, so record which value you used.

### P4-4 Collar clamp identity + torque (gate 1 physical)

- **What you need:** a calibrated torque wrench.
- **What you do:** record the clamp-screw identity and applied torque at the
  F108/M5×16 collar clamping step (docs/12:115, ledger §2.2).
- **What you check:** M5×16 10.9 at 6 Nm. 12.5 mm engagement. No washer beyond
  F108. The M3 set screws are never used as the torque connection.
- **Where you write it down:** the build record.
- **If it fails:** using the M3 set screws as the torque connection is the
  classic error, so torque only the M5×16.

### P4-5 Belt static proof test (gate 1 physical)

- **What you need:** a rigid **non-motor** fixture (docs/12:152), a calibrated
  torque transducer and slip marks.
- **What you do:** prove each belt drive at 5 Nm in each direction, 10 s hold,
  10 cycles. Slip-mark the shaft/key/pulley/hub interfaces. Inspect the belt
  teeth and collar afterwards.
- **What you check:** no relative slip at any marked interface. No belt-teeth
  or collar damage. No motor holding torque used as the reference. No pliers
  on journals.
- **Where you write it down:** the build record.
- **If it fails:** using motor holding torque as the reference is the common
  error, so the test must run on the rigid non-motor fixture.

### P4-6 R+W couplings

- **What you need:** a DTI and insertion marks.
- **What you do:** set each MK2 coupling at free length. Mark both journals
  for complete 16 mm hub insertion. Align before tightening the supports.
- **What you check:** ≤ 0.05 mm radial and ≤ 0.1° angular alignment. Bellows
  never stretched. Hub screws at 4.5 Nm (docs/12:123).
- **Where you write it down:** the build record.
- **If it fails:** tightening the supports before alignment stretches the
  bellows, so align first and tighten second.

### P4-7 Assembly-order compliance + free rotation

- **What you need:** feeler gauges and a DTI per step, plus hand rotation.
- **What you do:** execute docs/12 §6 steps 1–10 in order. F112 off F102. MB2
  before key. Dowels after module insertion. Rear endfloat 0.08–0.50 mm. Front
  outer-ring clearance 0.020–0.110 mm. Belt centres 52.5–64.5 mm. The
  1.000 mm deflection target force is 13.8715–15.1215 N at 62.5 mm.
- **What you check:** every step's criterion is met. Rotation stays free
  through 360° after each retention step. The actual belt span is used in the
  manufacturer formula.
- **Where you write it down:** the build record.
- **If it fails:** skipping the 360° hand rotation after a retention step is
  the common miss, so rotate by hand after every step.

---

## Phase 5: Electrical checks

De-energised first, then the safety-function gates. This phase closes gates
18, 19, 20, 21, 22 (physical), 23, 27 and the EMC part of 41. Energisation of
the machine stays prohibited until this phase's records exist (docs/22 §10,
docs/23 §4).

### P5-1 IF-B01 review, build and bare-board fixture (gate 18)

- **What you need:** the [fixture procedure](../hardware/if-b01/if-b01-test.md).
- **What you do:** complete the human electrical/mechanical review of the
  draft board. Get the fabricator panel/stack-up confirmation. Build the
  bare board. Run the fixture with **only** `if-b01-factory-test.uf2`, with
  J4–J7 disconnected.
- **What you check:** fixture pass. ERC/DRC/parity clean. Reviewer signatures.
- **Where you write it down:** archive the fixture logs plus the FAI template
  rows.
- **If it fails:** running the fixture with J4–J7 connected is the common
  error, so disconnect them before the test.

### P5-2 Cabinet de-energised inspection (gates 21, 27)

- **What you need:** a multimeter for continuity, a torque driver and
  visual/clearance gauges.
- **What you do:** execute the docs/23 §3 table. Point-to-point continuity
  against the approved schematic. Segregation/clearance and screen
  inspection. Terminal torque and marking. Protection-device configuration
  review.
- **What you check:** every docs/23 §3 row is filled and matching the
  schematic. Terminal designations TB24-01…PE-BAR correct.
- **Where you write it down:** the build record plus the docs/23 §3 table.
- **If it fails:** an empty §3 row is the common gap, so fill every row before
  you sign.

### P5-3 PE continuity and insulation (gate 21)

- **What you need:** a low-ohmmeter (4-wire) and an insulation tester at the
  plan's test voltage.
- **What you do:** bond test every PE bond, which means the copper PE bar, the
  M6 bonds and PE ≥ 1.5 mm² per docs/22 §11.4. Insulation test per the
  approved plan under IEC 60204-1 as applied locally.
- **What you check:** PE continuity ≤ **0.1 Ω** per bond. Insulation readings
  within the approved plan's limits. Instrument IDs recorded.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** a single missed bond usually trips the 0.1 Ω limit, so test
  every PE bond, not a sample.

### P5-4 E-stop/guard channels, faults, reset/EDM (gate 23)

- **What you need:** a multimeter plus the approved fault-injection fixtures.
- **What you do:** test each E-stop channel, both guard contact paths (11-12
  and 21-22 interlock, 31-32 and 41-42 locked state), cross-wire faults,
  reset/EDM behaviour and each contactor fault mode per the reviewed test plan
  (docs/22 §8.4).
- **What you check:** opening either channel, contact or wire removes the XPS
  safety outputs. Reset/EDM per the XPS configuration (app function 1,
  start 3). The 20 ms XPS component response is **not** treated as machine
  stop time.
- **Where you write it down:** the FAI template rows.
- **If it fails:** treating the 20 ms component response as stop time is the
  common error, so record the measured stop time separately in P5-7.

### P5-5 K1/K2/K3 DC-duty opening verification (gate 19 physical)

- **What you need:** an oscilloscope on the contactor coil/contact and the
  branch voltage.
- **What you do:** measure the actual opening behaviour of each LC1D12BD
  three-series-pole chain on hardware (docs/22 §8.5).
- **What you check:** the recorded opening response is consistent with the
  qualified catalogue duty (20 A at ≤ 75 V, DC-1/DC-2..5, margins 2.06×/3.45×).
  The critical-current durability note stays an open observation.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** measuring one contactor and assuming the rest is the common
  error, so scope all three chains.

### P5-6 DC-bus decay, prospective fault, inrush (gates 20, 22 physical)

- **What you need:** an isolated HV probe plus a scope, and a clamp meter for
  inrush.
- **What you do:** measure each drive bus decay curve after K1/K2/K3 open,
  with the bleed resistors fitted. Record the actual bus capacitance. Measure
  inrush and prospective short-circuit coordination.
- **What you check:** the measured decay reaches the risk-assessed safe
  voltage. Set T2 to measured decay plus a margin of at least 1 s. The
  proposal is ≈3 s with the 2.2 kΩ/5 W bleeds, τ ≈ 1.1 s. The 0.45 s desk
  estimate is replaced by the measured curve.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** keeping the 0.45 s desk estimate instead of the measured
  curve is the common error, so use the measured decay.

### P5-7 Stop-time measurement (gate 23)

- **What you need:** calibrated time capture (scope/PLC) plus a motion marker.
- **What you do:** measure stop time from E-stop actuation to standstill for
  the worst-case motion state. Repeat with guard channel actuation.
- **What you check:** the measured stop time satisfies the task risk
  assessment, recorded per axis.
- **Where you write it down:** the FAI template.
- **If it fails:** measuring only one motion state is the common gap, so use
  the worst case and record every axis.

### P5-8 Regenerated-energy confirmation (gate 23)

- **What you need:** bus voltage recording during worst-case stops.
- **What you do:** confirm regenerated-energy behaviour on stops against the
  desk analysis (docs/22 §11.2).
- **What you check:** measured stop energy ≤ bus storage. The ≥ 0.18 J desk
  figure is verified with the measured bus C. No bus overvoltage event.
- **Where you write it down:** the build record.
- **If it fails:** verifying the 0.18 J against the desk capacitance instead
  of the measured C is the common error, so use the measured bus capacitance.

### P5-9 EMC assessment (gate 41 physical)

- **What you need:** competent persons for the review and the assessment.
- **What you do:** enclosure/standards review of segregation, shield
  terminations and ducts. Then an EMC assessment on the built cabinet by
  competent persons.
- **What you check:** the review and the assessment are completed on the built
  cabinet and recorded.
- **Where you write it down:** the build record and the FAI template.
- **If it fails:** assessing a cabinet that is not yet built is the common
  error, so assess the finished cabinet.

---

## Phase 6: Motion commissioning

This phase closes gates 11, 12, 13 (physical), 24, 25 (force records), 28 and
29. Energise in stages, per docs/24 §7.

### P6-1 Staged energisation (gate 24)

- **What you need:** a calibrated multimeter or scope. Record every
  instrument.
- **What you do:** under competent oversight, verify the actual voltages,
  polarity, field contacts, encoder direction, drive enable behaviour,
  interlocks, fault response and restart prevention.
- **What you check:** every docs/24 §7.3 item is verified on hardware. No
  extrapolation from host tests.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** extrapolating from host tests instead of verifying on
  hardware is the common error, so work through §7.3 item by item on the
  machine.

### P6-2 Run-in

- **What you need:** a DTI along travel and a visual belt check.
- **What you do:** run in the KK86 carriages and feed drives. Verify belt
  alignment and minimum tooth engagement on both pulleys (docs/30 P4/P5).
- **What you check:** carriage travel parallel to X within the stage
  specification. Belts aligned. Engagement per the MISUMI records.
- **Where you write it down:** the build record.
- **If it fails:** misaligned belts show up as tooth engagement loss, so check
  both pulleys before the run-in.

### P6-3 Wire path (docs/31 §4)

- **What you need:** 1.2 and 4.5 mm wire.
- **What you do:** run the taut-wire check with 1.2 and 4.5 mm wire along the
  route payoff → left guide → front nip → cut plane → rear nip → right guide →
  output.
- **What you check:** zero unintended contact along the 880 mm route for both
  ODs.
- **Where you write it down:** the build record.
- **If it fails:** checking only one wire size is the common gap, so run both
  1.2 and 4.5 mm through the full route.

### P6-4 Nip preload 50–75 N per side (gate 12)

- **What you need:** a calibrated force gauge with U ≤ 1.0 N.
- **What you do:** lift each upper cartridge pair with the force gauge at the
  roller axis until the wire releases. Do both sides and both ODs (docs/20
  step 3).
- **What you check:** 50–75 N per nip, guard-banded by the gauge U.
- **Where you write it down:** the force-records CSV (P10-3 rows) plus the
  build record.
- **If it fails:** lifting off-axis gives a wrong reading, so pull exactly at
  the roller axis.

### P6-5 Measuring-wheel contact force 6–19 N (gates 12, 28)

- **What you need:** a calibrated force gauge with U ≤ 1.0 N.
- **What you do:** measure the wheel force the same way, under the arm hub.
  Do both ODs.
- **What you check:** 6–19 N, guard-banded.
- **Where you write it down:** the force-records CSV plus the build record.
- **If it fails:** measuring one OD only is the common gap, so record both.

### P6-6 ±60 N roller-force envelope (gate 12)

- **What you need:** a force gauge and a motion check, per the qualification
  plan.
- **What you do:** qualify pressure/friction/jam behaviour on the assembled
  cassette across the ±60 N roller-force envelope (docs/verification/feed-shaft-fem.md:116
  context).
- **What you check:** no jam, pressure or friction failure within the
  envelope. Torque alone is not accepted as proof (5 Nm at r = 20 mm is not
  the force limit).
- **Where you write it down:** the build record plus the qualification inputs.
- **If it fails:** accepting torque as proof of force is the common error, so
  measure the force directly.

### P6-7 Encoder direction and calibration (gate 29)

- **What you need:** a traceable displacement reference with recorded
  uncertainty.
- **What you do:** verify encoder direction (docs/24 §7.3). Then execute the
  [docs/13 §5](13-measuring-interfaces-and-calibration.md) calibration, which
  is a traceable independent reference, synchronous capture, all 128-count
  intervals in both directions, fit plus held-out verification, and binding
  to IDs, Z phase and recipe.
- **What you check:** per the held-out record |x_pred − x_ref| + U_ref ≤
  0.025 mm + 0.0001·|x_ref|. Complete-turn-only fitting must fail.
  `--require-factory-accuracy` passes only after this plus the P0-4
  circumference verification. Invalidate on wheel/encoder/coupling change or
  range breach.
- **Where you write it down:** the measurement-analysis outputs plus the build
  record.
- **If it fails:** fitting on the whole turn only is the common error, so use
  the held-out verification and let the complete-turn-only fit fail.

### P6-8 Blade pair loaded clearance + edge qualification (gate 13 physical)

- **What you need:** the docs/24 §4.4 edge qualification procedure.
- **What you do:** verify loaded clearance before installation and qualify the
  edge per docs/24 §4.4. Install only afterwards.
- **What you check:** loaded clearance verified and edge qualified before
  installation.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** installing before the loaded clearance check is the common
  error, so qualify first and fit second.

### P6-9 Rotating fatigue qualification (gate 11)

- **What you need:** an approved rotating multiaxial fatigue plan, executed
  per the plan with cycles and periodic inspection.
- **What you do:** execute the plan on the actual shaft/build (docs/15:124
  requirement).
- **What you check:** per the approved plan. No FEM number releases the
  thread. A demonstrated life must be established for the machine's duty.
- **Where you write it down:** the build record plus the plan archive.
- **If it fails:** releasing on an FEM number instead of a demonstrated life
  is the common error, so run the physical plan on the actual shaft.

---

## Phase 7: Payoff

This phase closes gate 16 and the payoff part of gate 42. The evidence goes on
the [guard/payoff release template](inspection/guard-payoff-release-template.csv).

### P7-1 Full-spool mass (gate 16)

- **What you need:** a calibrated scale.
- **What you do:** weigh the first full spool per wire lot.
- **What you check:** ≤ **30 kg** until the deflection re-check (docs/28
  §2.1). Above that, re-check before use.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** skipping the re-check above 30 kg is the common error, so
  weigh before every use above the limit.

### P7-2 Spindle deflection under load (gate 16)

- **What you need:** a DTI at mid-span.
- **What you do:** measure P002 deflection with the full spool fitted.
- **What you check:** against the desk bounds 0.65 mm / 52 MPa at 30 kg
  (docs/28 §2.1). The C45 certificate governs. Deflection changes payout
  geometry, not length accuracy.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** measuring without the full spool fitted is the common
  error, so fit the spool first.

### P7-3 Brake adjustment trial (gate 16)

- **What you need:** the M8 adjuster and the docs/26 §3 procedure.
- **What you do:** set the P007 pad pressure with the M8 adjuster to the
  working payout tension (docs/26 §3). Record the setting.
- **What you check:** stable payout tension by trial. The brake is **not** a
  safety or holding brake, so record this in the template.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** treating the brake as a safety brake is the common error,
  so record that it is neither a safety nor a holding brake.

### P7-4 Runaway/overspeed measurement (gate 16)

- **What you need:** a tachometer or high-speed capture.
- **What you do:** measure runaway and overspeed behaviour with the brake set
  (docs/26 §6.3).
- **What you check:** recorded runaway/backlash behaviour. The sign-off row is
  completed.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** leaving the sign-off row empty is the common gap, so
  complete it after the measurement.

### P7-5 Spool float and rotation

- **What you need:** the P3-2 and P3-3 weld rows, ready to attach.
- **What you do:** confirm 0.1–0.3 mm end float and free rotation. Confirm the
  P3-2/P3-3 weld rows are attached (docs/30 P7).
- **What you check:** the 0.1–0.3 mm end float and free rotation are
  confirmed, and the weld rows are attached.
- **Where you write it down:** the build record.
- **If it fails:** the missing weld rows block the sign-off, so attach them
  before you record the float.

---

## Phase 8: Guard

This phase closes gates 14, 15, 17, 22 (physical) and the guard part of 42.
The evidence goes on the
[guard/payoff release template](inspection/guard-payoff-release-template.csv).

### P8-1 SLC/ACF-1 physical fit and locking travel (gate 15 physical)

- **What you need:** feeler gauges and a DTI on the actuator window.
- **What you do:** mount the SLC body on G106 and the ACF-1 on the door.
  Verify insertion depth, alignment and locking travel (docs/26 §2, §6.2).
- **What you check:** alignment window 29 + 3 mm in X/Y. 2 mm overtravel with
  the stop set inside 0.2–2 mm. No lateral force on the head. The ACF-1
  approach radius ≥ 400 mm is respected.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** lateral force on the head is the common fault, so adjust
  the mount until the head runs free.

### P8-2 Safety-distance reconfirmation (gate 14 physical)

- **What you need:** a dimensional check of the panel-to-hazard distance.
- **What you do:** reconfirm the Ø30 mm aperture reach-through against the
  current EN ISO 13857 edition and the completed risk assessment.
- **What you check:** 345 mm > 120 mm slot reach (≈2.9× desk margin, docs/28
  §2.2). Residual annular opening ≤ 12.75 mm with wire installed. The current
  edition is cited.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** citing an old edition of EN ISO 13857 is the common error,
  so name the current edition in the record.

### P8-3 Task-specific risk-assessment sign-off (gate 14)

- **What you need:** the risk assessment rows and a competent person to sign.
- **What you do:** complete the task-specific risk assessment covering
  cutting, wire release, entanglement, ejection, access, maintenance, loading,
  loss of power and emergency egress. Obtain the competent-person sign-off.
- **What you check:** every hazard row is dispositioned and signed.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** an unsigned hazard row is the common gap, so walk the list
  and get each row signed.

### P8-4 PC door impact and hardware (gate 17)

- **What you need:** the docs/26 §6.6 procedure.
- **What you do:** impact verification of the 5 mm PC front door plus
  hinge/handle inspection (docs/26 §6.6).
- **What you check:** the impact verification and the hinge/handle inspection
  are done and recorded.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** skipping the hinge/handle inspection is the common miss, so
  inspect them together with the door.

### P8-5 E1/E2 release-permission fault injection (gate 22 physical)

- **What you need:** fault-injection gear for the contactor-failure,
  timer-failure and power-loss cases.
- **What you do:** physically validate the R2 + K1–K3 NC chain + T2 on-delay
  design (docs/22 §11.8) by fault injection, using the contactor-failure,
  timer-failure and power-loss cases.
- **What you check:** unlock energises only when XPS outputs are off **and**
  all three mirror contacts report open **and** T2 has elapsed. Power loss
  keeps the lock locked. Until sign-off, access after a trip stays a LOTO
  maintenance procedure.
- **Where you write it down:** the FAI template plus the guard/payoff
  template.
- **If it fails:** unlocking with a contactor still closed is the failure you
  are testing for, so inject each failure case and watch the unlock.

### P8-6 Aperture treatment

- **What you need:** edge grommets and the docs/26 §2 requirement.
- **What you do:** confirm the edge grommets are fitted and the aperture edges
  are burr-free (docs/26 §2).
- **What you check:** grommets fitted and edges burr-free.
- **Where you write it down:** the guard/payoff template.
- **If it fails:** a missing grommet on the aperture edge is the common miss,
  so feel every edge.

---

## Phase 9: Thermal

This phase closes gate 10 (instrumented cold-start/hot-run plus motor-case
temperature) and the cabinet thermal part of docs/22 §11.5.

### P9-1 Instrumented cold-start/hot-run (gate 10)

- **What you need:** calibrated thermocouples on the front and rear bearing
  seats and the housing, with logging during the run.
- **What you do:** instrument the built cartridge and run the cold-start/
  hot-run test of docs/12:38 with the worst-case standstill hold (30 W motor
  loss, docs/28 §2.5).
- **What you check:** measured seat temperature rise within the record's
  explicit 20 K shaft-above-housing envelope. The desk FEM's 29.66 K
  prediction is not met, so the physical measurement is the closure (ledger
  §2.5). The S5M thermal break and rear-path mitigations do not relax the
  number.
- **Where you write it down:** the build record plus the FAI template.
- **If it fails:** relying on the 29.66 K desk prediction is the common error,
  so trust the physical measurement against the 20 K envelope.

### P9-2 Motor-case temperature at rated hold (gate 10)

- **What you need:** a calibrated contact/IR thermometer.
- **What you do:** record the motor-case temperature at rated hold and apply
  the [bearing-fit](../engineering/generated/bearing-fit/report.md) conduction
  calculation (docs/28 §2.5).
- **What you check:** the temperature is recorded at rated hold and the
  conduction calculation is applied.
- **Where you write it down:** the build record.
- **If it fails:** measuring off the rated hold point is the common error, so
  hold the rated condition while you record.

### P9-3 Cabinet temperature mapping

- **What you need:** thermocouple mapping inside the enclosure.
- **What you do:** map cabinet temperature at maximum duty against the ≈152 W
  vs 280 W L35 desk budget (docs/22 §11.5).
- **What you check:** internal temperature within the L35 (or derated) budget.
- **Where you write it down:** the build record.
- **If it fails:** mapping at idle instead of maximum duty is the common
  error, so run the machine at full duty while you map.

---

## Phase 10: First article: 30-piece Cpk qualification

This phase closes gates 25, 26 and 43. The procedure authority is
[docs/20](20-physical-qualification.md).

### P10-1 Generate templates

- **What you need:** the `starz_qualification` tool.
- **What you do:** run `starz_qualification --template <dir>`. The tool writes
  120 length rows and 6 force rows.
- **What you check:** the 120 length rows and 6 force rows are written into
  the directory you chose.
- **Where you write it down:** the generated CSVs.
- **If it fails:** the usual miss is the wrong output directory, so point
  `<dir>` at where you will keep the records.

### P10-2 Machine setup

- **What you need:** the F206 inserts for the OD under test and the
  protrusions in [mechanism status.json](../engineering/generated/mechanism/status.json).
- **What you do:** fit the F206 inserts for the OD under test. Set AJ01/AJ02
  to the protrusions in mechanism status.json. Set M210 so the 626 roller just
  touches the wire.
- **What you check:** the inserts match the OD under test, AJ01/AJ02 match
  status.json, and M210 just touches the wire.
- **Where you write it down:** the build record.
- **If it fails:** setting AJ01/AJ02 from memory instead of status.json is the
  common error, so read the values from the file.

### P10-3 Force rows (gates 12, 28, 25)

- **What you need:** the six force rows measured in P6-4/P6-5, with the gauge
  U.
- **What you do:** enter the six force rows measured in P6-4/P6-5 with the
  gauge U.
- **What you check:** nip 50–75 N, wheel 6–19 N, force gauge U ≤ 1.0 N,
  guard-banded.
- **Where you write it down:** the force-records CSV.
- **If it fails:** entering force rows without the gauge U is the common gap,
  so carry the uncertainty over from P6-4/P6-5.

### P10-4 Length cells (gate 25)

- **What you need:** a calibrated scale or CMM at recorded temperature, 15–30
  °C. The length gauge U ≤ 0.05 mm.
- **What you do:** cut and measure 30 pieces per mandatory cell, which is OD
  1.2 and 4.5 mm × commanded 200 and 1000 mm. Do not discard pieces.
- **What you check:** error ≤ ±(0.05 % + 0.20 mm) after subtracting gauge U.
  **Cpk ≥ 1.33** per cell.
- **Where you write it down:** the length-records CSV.
- **If it fails:** discarding a bad piece breaks the sample, so keep every
  piece in the cell.

### P10-5 Qualify and archive (gate 25)

- **What you need:** the `starz_qualification` tool and the raw CSVs.
- **What you do:** run `starz_qualification --lengths <csv> --forces <csv>
  --out <dir>`. Archive the output directory with the raw CSVs.
- **What you check:** PASS per cell. `physical_results` is true only for
  non-`SYN-` gauge and sample identities.
- **Where you write it down:** length-cells.csv, force-checks.csv, report.md,
  status.json.
- **If it fails:** mixing SYN- identities into physical_results is the common
  error, so keep synthetic rows out of the physical flag.

### P10-6 First-article + energisation gate (gate 26)

- **What you need:** the evidence references from phases 0–9 and P10-1–P10-5.
- **What you do:** complete the
  [first-article-and-energisation-gate-template.csv](inspection/first-article-and-energisation-gate-template.csv)
  with those evidence references.
- **What you check:** every required evidence row is present with a reviewer
  disposition.
- **Where you write it down:** the FAI template.
- **If it fails:** an evidence row without a reviewer disposition is the
  common gap, so get every row reviewed.

### P10-7 Integration flag flips (gate 43)

- **What you need:** the physical evidence from phases 0–9 and the
  [docs/30](30-machine-assembly-sequence.md) and
  [docs/31](31-fabrication-guide.md) assembly guides.
- **What you do:** assemble per docs/30 and docs/31 and flip mechanism status
  flags **only** with the physical evidence from phases 0–9.
- **What you check:** no flag flips without physical evidence. Raw failed data
  is kept.
- **Where you write it down:** status.json plus the build record.
- **If it fails:** flipping a flag without physical evidence is the common
  error, so check the evidence exists before the flip.

### P10-8 Repository checks (gate 27)

- **What you need:** the repository tools, run from the repo root.
- **What you do:** run `tools/check_production_records.py` and the
  design-integrity checks after all records are entered. Fix any finding.
  Archive.
- **What you check:** the tools pass.
- **Where you write it down:** the tool outputs, archived with the build
  record.
- **If it fails:** a finding left unfixed is the common gap, so fix each
  finding and rerun until the tools pass.

---

## Closeout note

A gate is closed only when its steps above are executed and the named record
contains a measured pass with reviewer disposition. This runbook does not
close, release or relax anything by itself. It is the ordered path to do so
physically.

