# Machine assembly sequence and interface-fit record — S1-B-ASSY

**Revision:** `S1-B-ASSY-R1-2026-09-15`
**Status:** desk assembly plan from the checked CAD and source records. It is
**not** a physical build record: every physical check (torque, measured fit,
weld inspection, first movement) remains open and is routed to the blank
[inspection templates](inspection/). No step below is claimed as performed.

The geometric basis is the 507-occurrence mechanism R3 CAD whose code checks
verify, at both wire diameters: valid solids for all nine groups, zero common
volume between every guard/payoff pair and every implemented cross-group
pair, spring-deflection windows, nip preload 50–75 N, wheel force 6–19 N,
±10 mm bushing travel, moving-head X clearance and a collision-free 880 mm
wire envelope. The feed cartridge additionally carries code-checked critical
continuous axial insertion paths ([docs/12 §6](12-feed-cartridge-fabrication.md)).
Where a physical fit requires measured values, the step says so explicitly.

## 1. Assembly dependency tree

```
P1 frame (welded + bed)                 P9 cabinet/electrical (last, not released)
P2 feed cartridges (2×, per docs/12)      │
P3 pressure carriers + wire guides        │
P4 head + stages + motors + couplings     │
P5 cassette/head/guide interplay          │
P6 measuring module                       │
P7 payoff (floor-standing, separate)      │
P8 guard set + Bernstein lock             │
```

P1→P2→P3→P4→P5→P6 must complete before P8; P7 is independent of P1–P6
(separate floor plate) but must exist before P8 side panels are fitted on the
wire side; P9 mounts beside/behind the machine and is wired after P8.

## 2. Phase steps

### P1 — welded frame and bed

1. Weld F304 legs (4× 60×60×3, L760) to F302 long rails (2× L1080) and F303
   cross rails (2× L440); weld F305 foot plates (4× 100×100×8) to the legs.
   Weld check: full perimeter, no undercut; dimensional check of the four
   foot pads coplanar ≤1 mm over the 1080×440 rectangle (physical).
2. Bolt F301 bed (EN AW-6082 T651, 640×1120×25) to the rails with the
   FB306 ISO4762 M10×35 class 8.8 screws into DIN 929 M10 weld nuts. Top
   face flat ≤0.05 over the cassette/head footprints (physical, post-mount).
3. Anchor feet: 4× Ø13 floor anchors (purchase-to-requirement); level the bed
   to Z=0 datum within 0.1 mm over the travel (physical).

### P2 — feed cartridges (2×, off-machine, per docs/12 §6)

Build both cartridges on a bench in the documented order: rotor preassembly
(6001-2Z/C3 rear non-locating, 6002-2Z/C3 front locating, KM2+MB2 retention
with MB2 tang in the shaft slot before the key), keyed pulleys/rollers
(UMCL40-10-20-MC collars on the 10h6 shaft, HTPA20S5M250-A-N14 pulley,
HTBN225S5M-250 belt, 5JS9 keys), SEEGER A12 rings, F112 front module with
3m6×20 dowels and M4×20 screws. The code-checked insertion paths apply:
MB2 tang before key; preassembled rotor/pulley/front module from +S with
module pins removed; rear bearing from −S; dowel service-punch access.
Bearing seats: j5-matched 3–6 µm interference to measured bores per
[the fit record](../engineering/generated/bearing-fit/report.md) — measure
each bearing, then finish the seat (physical measurement gate, docs/12).
Belt tensioning and the M5 blind-hole tap depth per
[docs/14](14-f101-tapping-operation.md).

### P3 — pressure carriers and wire guides

1. Bolt F201 carrier bases (2×) to the bed at the tapped bed pattern; the
   bed carries the H001/F201/M206/M207 pattern (F301 record).
2. Fit the PSFJ12-240 guide posts (Ø12 g6, hard chrome) into the F201 post
   bores (Ø12.01×30 with clamp slits — clearance 0.01 nominal, clamped).
3. Slide each LHFRW12 flanged bushing (Ø12 g6 bore) onto its post; assemble
   F202 pressure plates (2× Ø21.01 bushing bores, 4×M4 PCD32, 10 dowel holes)
   with the F203 bridge, F208 spring caps and SWU17-60 springs; set the DIN
   913 M8 adjusters so each nip lands in 50–75 N (the CAD solves the
   protrusion; the force is measured at commissioning — physical).
4. Wire guides F205 holders at X38–47 with F206 brass inserts sized to the
   wire OD+0.5 clearance; the 60° lead-in cones face the wire entry.
   Check: guide insert axis on the wire axis within 0.1 mm (physical).

### P4 — head, stages, motors, couplings

1. Bolt H001 (180×1040×20) to the bed top at X−90…90; H002 risers set the
   stage planes at Z40. Datum check: H001 top flat and parallel to the bed
   within the drawing limits (H003 sheet governs tolerance classes).
2. Mount the two HIWIN KK8610P340A1F0 axes to H001/H002; verify carriage
   travel parallel to X within the stage specification (physical run-in).
3. Fit H003 motor adapters to the carriages: F0 register Ø49.97±0.01 into
   the carriage register, 4× M5×16 class 10.9 + ISO7092 washers on PCD70.
4. Mount 34E1K-85 motors: pilot Ø73.10+0.02/0 into the motor Ø73.10
   (0.10 diametral clearance), 4× M5×25 on the 69.58 grid.
5. Couplings (head axes): per the R6 axial stack — actual free-length
   coupling, ≥0.53 mm full-hub insertion reserve, ≥12.58 mm shaft-end gap,
   hub 1.00±0.05 from the locknut shoulder, bellows never stretched
   (docs + H003 drawing). Motor D-flats and Ø12/Ø10 shafts per the archived
   R+W ordering page. Joint preload/torque: **not established** — preload
   record pending (docs/12, H003 sheet).
6. Blade holders H004 and blade pair: purchase-to-drawing blades; edge
   alignment/apex overlap 0.2 mm per machine_spec is a commissioning
   measurement, not an assembly claim.

### P5 — cassette/head/guide interplay

1. Land both cartridges onto their F201 bases; wire contact station S108.75
   with nips at X75.5/138.0 (docs/12 §6).
2. Install feed motors 24HS40-5004D-E1000 with the belt drive; verify belt
   alignment on both pulleys (5JS9 keyways; belt 25 mm wide) and minimum
   engagement per the MISUMI belt records.
3. Close the wire path: payoff (P7) → left guide → front nip → cut plane →
   rear nip → right guide → output. The 880 mm straight wire route and both
   1.2/4.5 mm wire envelopes must pass the mechanism wire checks (already
   code-verified; re-verify physically with a taut wire).

### P6 — measuring module

1. Bolt M206 bracket + M207 counter-roller column to the bed pattern at
   X−205 side; press the two 626-2Z bearings into M203 (K6 seats) and fit
   the M202 shaft (6 h6; seats 6 j5) with the M204 pivot arm, M209 cap and
   SWU10.5-35 spring; set the wheel force into 6–19 N (physical spring
   force check).
2. Fit the Kübler 8.0000.3123.0006 wheel (6 H7 bore) on the shaft with the
   No123 M4×6 set screw and the supplied encoder coupling; M201 plate holds
   the Autonics E40S6 (pilot 19.98–20.00 into the 20 mm encoder pilot bore;
   tip 20 mm; 3× M3×10 + ISO7092 washers).
3. Wheel runout and circumference: 100±0.1 mm is the manufacturer claim;
   measure and enter into the calibration (docs/13 — physical).

### P7 — payoff (independent floor assembly)

1. Weld P004/P005 posts (18 mm S235JR, 70 wide) to P003 (430×560×8) with a
   fixture holding both Ø47 H7 pockets coaxial (≤0.1 mm across posts after
   welding — physical); weld check as P1.
2. Press the two 6204-2Z/C3 bearings into the pockets from the spool side.
3. Slide the P002 spindle (Ø20 h6) through the P001 welded spool (Ø20.5
   flange bores) and both bearings; runout ≤0.010 between seats (P002 sheet).
4. Fit P006 brake disc (Ø20 H7, 6 JS9 keyway) outboard of the right post
   with its key and M6×8 setscrew; thread the M20×1.5 nut (KM-class + tab
   washer) and set axial float so the spool spins freely (0.1–0.3 mm end
   float, physical).
5. Mount P007 caliper bracket; fit pads and the SWU-class adjuster spring
   (purchase-to-requirement) and set braking tension by trial (physical).
6. Anchor P003 through 4× Ø13 holes. Full-spool mass, spindle deflection,
   brake adjustment and runaway checks are
   [docs/26 §4/§6](26-guard-and-payoff-selection.md) gates.

### P8 — guard set

1. Weld the G106 SLC mounting plate to the door jamb; drill the SLC pattern
   **only from the Bernstein mounting drawing** (docs/26 §2).
2. Hang the G105 door (5 mm PC in 20×20×2 tube frame) on two hinges rated
   ≥15 kg; fit handle; fit the ACF-1 actuator to the door and the
   SLC-F-024-20/20-R4 body to the plate; verify insertion depth, alignment
   and locking travel physically.
3. Fit G101/G102 side panels (DC01 2 mm, Ø30 wire pass-throughs at Y0/Z112),
   G103 rear, G104 roof; M6×12 8.8 at ≤200 mm pitch into rivet nuts; edge
   grommets on the apertures.
4. Safety-distance, aperture and lock-release validation per docs/26 §6 and
   the guard/payoff release template — physical gates.

### P9 — cabinet and wiring

Per [docs/23](23-cabinet-electrical-assembly.md) and
[docs/22](22-controls-and-wiring.md): enclosure, PE, terminals, drives,
supplies, XPS, contactors, IF-B01, glands and field wiring. The DC duty,
thermal and protection calculations close from the desk (see the 2026-09-15
electrical package in docs/22); the energisation gate and stop-time
measurement remain physical first-article gates.

## 3. Interface-fit register (source-backed values)

| Pair | Fit / value | Source |
|---|---|---|
| F101 seats ↔ 6001/6002 bores | j5 seat matched 3–6 µm interference to measured bores | bearing-fit report, docs/12 |
| UMCL40 collar ↔ 10 h6 shaft | bore 10.000–10.030; ≤0.039 diametral clearance; key transmits torque | docs/12, catalogue evidence |
| HTPA pulley ↔ 14 h7 shaft | 14 H7 bore + 5 JS9 keyway | selected-mechanism/transport CSV |
| R+W MK2 hubs ↔ motor Ø10 D-flat / Ø12 shaft | hub bores 10/12; axial stack ≥0.53 mm insertion, ≥12.58 mm end gap | engineering/README R6, H003 sheet |
| H003 pilot ↔ 34E1K motor | Ø73.10+0.02/0 vs Ø73.10: 0.10 diametral clearance | catalogue evidence |
| H003 F0 register ↔ KK86 carriage | Ø49.97±0.01 into carriage register | H003 sheet |
| A12 ring ↔ shaft groove | groove 11.45±0.02 vs ring 11.50−0.11; conditional clearance — audited in [docs/28](28-physical-contradiction-audit.md) | docs/15, measurement report |
| KM2 nut ↔ M15×1 thread; MB2 tang ↔ 4.2 slot | turn one outer tab into a slot; replace MB2 every reinstall | docs/12 |
| M202 shaft ↔ 626-2Z bores | 6 h6 shaft, 6 j5 seats, K6 housing pockets | measurement record |
| Kübler wheel ↔ M202 | 6 H7 on 6 h6; set screw torque pending | catalogue evidence |
| E40S6 encoder ↔ M201 | pilot 19.98–20.00 into encoder bore; tip 20 mm | M201 sheet |
| P002 spindle ↔ P001 spool | Ø20 h6 in Ø20.5 bores (0.5 clearance) | docs/26, P002 sheet |
| P002 ↔ 6204 pockets | Ø20 h6 in bearing bores; pockets Ø47 H7 | docs/26, P004–P007 sheet |
| PSFJ12-240 posts ↔ F201 bores | Ø12 g6 in Ø12.01 bores, clamp slits | mechanism spec |
| LHFRW12 ↔ posts | Ø12 g6 bore on Ø12 g6 shaft, floating linear pair | mechanism spec |
| Wire ↔ F206 inserts | insert bore = wire OD+0.5 | mechanism spec |
| Guard SLC pocket ↔ body | 0.5 mm CAD clearance class; drilling per Bernstein | docs/26 |

Every row with a measured value (bearings, encoders, collars, weld
positions) requires the corresponding incoming or in-process inspection
record before the assembly step that consumes it — the blank
[incoming-inspection template](inspection/incoming-inspection-template.csv)
governs. No fit in this table has been physically confirmed.

## 4. Warnings and holds

Torque/preload records, matched bearing measurement, weld inspection,
flatness/runout measurements, spring-force settings, belt tension, blade
alignment, stop-time, risk-assessment sign-off, full-spool weighing and
cycle-time confirmation all require the physical machine. This document
sequences them so that nothing is left to improvisation, but it does not
pretend they are done.
