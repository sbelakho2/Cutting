# Machine assembly sequence and interface-fit record — S1-B-ASSY

**Revision:** `S1-B-ASSY-R1-2026-09-15`
**Status:** this is the assembly plan for the internal build. The step-by-step
fabrication path is the [fabrication guide](31-fabrication-guide.md). Every
physical check (torque, measured fit, weld inspection, first movement) is
performed and recorded during the build. Until then, no step is claimed as
performed.

The geometric basis is the 504-occurrence mechanism R4 CAD. Its code checks
verify, at both wire diameters: valid solids for all nine groups, zero common
volume between every guard/payoff pair and every implemented cross-group
pair, spring-deflection windows, nip preload 50–75 N, wheel force 6–19 N,
±10 mm bushing travel, moving-head X clearance, and a collision-free 880 mm
wire envelope. The feed cartridge additionally carries code-checked critical
continuous axial insertion paths
([docs/12 §6](12-feed-cartridge-fabrication.md)). Where a physical fit
requires measured values, the step says so explicitly.

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

P1→P2→P3→P4→P5→P6 must complete before P8. P7 is independent of P1–P6 (it is
a separate floor plate) but must exist before the P8 side panels are fitted
on the wire side. P9 mounts beside or behind the machine and is wired after
P8.

## 2. Phase steps

### P1 — welded frame and bed

What you need: the F304 legs (4× 60×60×3, L760), F302 long rails (2× L1080),
F303 cross rails (2× L440), F305 foot plates (4× 100×100×8), the F301 bed
(EN AW-6082 T651, 640×1120×25), FB306 ISO4762 M10×35 class 8.8 screws,
DIN 929 M10 weld nuts, and 4× Ø13 floor anchors.

1. Weld the legs to the long rails and cross rails, then weld the foot
   plates to the legs. Check: full perimeter welds with no undercut, and the
   four foot pads coplanar ≤1 mm over the 1080×440 rectangle (physical).
2. Bolt the bed to the rails with the M10×35 class 8.8 screws into the
   DIN 929 weld nuts. Check: the top face flat ≤0.05 over the cassette/head
   footprints (physical, post-mount).
3. Anchor the feet with the 4× Ø13 floor anchors (purchase-to-requirement)
   and level the bed to the Z=0 datum within 0.1 mm over the travel
   (physical).

Why these fits matter: the bed is the datum every later stage measures
from. If the bed is not flat and level, the cartridge, head and measuring
module will all carry the error.

If you get stuck: a bed that will not level usually means the floor or the
frame is out, not the bed plate. Check the four foot pads first, then the
floor, before you touch the bed bolts.

### P2 — feed cartridges (2×, off-machine, per docs/12 §6)

Build both cartridges on a bench in the documented order. What you need:
6001-2Z/C3 rear non-locating bearings, 6002-2Z/C3 front locating bearings,
KM2+MB2 retention, UMCL40-10-20-MC collars, HTPA20S5M250-A-N14 pulleys,
HTBN225S5M-250 belts, 5JS9 keys, SEEGER A12 rings, 3m6×20 dowels and
M4×20 screws.

1. Rotor preassembly, with the MB2 tang in the shaft slot before the key.
2. Keyed pulleys and rollers: the UMCL40-10-20-MC collars on the 10h6
   shaft, the HTPA20S5M250-A-N14 pulley, the HTBN225S5M-250 belt, the 5JS9
   keys.
3. SEEGER A12 rings, then the F112 front module with its 3m6×20 dowels and
   M4×20 screws.

The code-checked insertion paths apply: MB2 tang before key, the
preassembled rotor/pulley/front module from +S with the module pins removed,
the rear bearing from −S, and dowel service-punch access. Bearing seats are
j5-matched with 3–6 µm interference to measured bores per
[the fit record](../engineering/generated/bearing-fit/report.md): measure
each bearing, then finish the seat (physical measurement gate, docs/12).
Check belt tensioning and the M5 blind-hole tap depth per
[docs/14](14-f101-tapping-operation.md).

Why these fits matter: the 3–6 µm interference is what keeps the bearing
from creeping on the shaft under load, while the measured (not nominal)
match stops an oversize bore from leaving the seat loose or an undersize one
from cracking the ring.

If you get stuck: a bearing that will not seat means the bore is wrong, so
measure the bore first and never force it. A bearing that slips in means the
seat is undersize and must be remade, not retained with adhesive.

### P3 — pressure carriers and wire guides

1. Bolt the F201 carrier bases (2×) to the bed at the tapped bed pattern.
   The bed carries the H001/F201/M206/M207 pattern (F301 record).
2. Fit the PSFJ12-240 guide posts (Ø12 g6, hard chrome) into the F201 post
   bores (Ø12.01×30 with clamp slits, clearance 0.01 nominal, clamped).
3. Slide each LHFRW12 flanged bushing (Ø12 g6 bore) onto its post. Assemble
   the F202 pressure plates (2× Ø21.01 bushing bores, 4×M4 PCD32, 10 dowel
   holes) with the F203 bridge, F208 spring caps and SWU17-60 springs. Set
   the DIN 913 M8 adjusters so each nip lands in 50–75 N. The CAD solves the
   protrusion; the force is measured at commissioning (physical).
4. Wire guides: F205 holders at X38–47 with F206 brass inserts sized to the
   wire OD + 0.5 clearance, and the 60° lead-in cones facing the wire entry.
   Check: guide insert axis on the wire axis within 0.1 mm (physical).

Why these fits matter: the 0.01 mm clamped clearance on the posts holds the
guides rigid when clamped, and the 50–75 N nip band is what feeds the wire
without crushing or slipping. The OD + 0.5 insert clearance lets both wire
diameters pass without the wire touching anything but the intended contacts.

If you get stuck: if you cannot land in the 50–75 N window, move the DIN 913
adjusters in small steps and re-measure each time. The force comes from
adjuster position, not from stretching the catalogue springs.

### P4 — head, stages, motors, couplings

1. Bolt H001 (180×1040×20) to the bed top at X−90…90. The H002 risers set
   the stage planes at Z40. Datum check: H001 top flat and parallel to the
   bed within the drawing limits (the H003 sheet governs tolerance classes).
2. Build each axis per [docs/39 §5](39-custom-cut-axis.md): mill the H005
   rail seats flat to 0.02/300, bolt the two HGR20 rails at 60 mm pitch,
   align with a gauge bar and torque the M5 screws to 6 Nm. Bolt H006 onto
   the four HGH20CA blocks, mount the DSG16h nut housing in its pocket,
   install the screw through BK12 (fixed, preloaded angular bearings) and
   BF12 (floating), and adjust the nut preload per the supplier setting.
   Then mount both axes on H001/H002, run them in for 1 hour and re-measure
   block preload and repeatability, target ±0.02 mm over the 194 mm travel
   with the DTI. Verify carriage travel parallel to X within the stage
   specification (physical run-in). The HIWIN KK8610C-340A1-F0 (USD 824.72
   per unit, MISUMI VN 2026-09-16) and eBay item 406408632737 (P-grade,
   US$450 snippet) remain recorded fallbacks if supply fails.
3. Fit the H003 motor adapters to the carriages: the F0 register
   Ø49.97±0.01 into the carriage register, 4× M5×16 class 10.9 plus ISO7092
   washers on PCD70.
4. Mount the 34E1K-85 motors: pilot Ø73.10+0.02/0 into the motor Ø73.10
   (0.10 diametral clearance), 4× M5×25 on the 69.58 grid.
5. Couplings (head axes): per the R6 axial stack. Use the actual free-length
   coupling, keep ≥0.53 mm full-hub insertion reserve and ≥12.58 mm
   shaft-end gap, set the hub 1.00±0.05 from the locknut shoulder, and never
   stretch the bellows (docs + H003 drawing). The motor shaft is Ø14, the
   screw input is Ø8 h7 × 18 mm per docs/39. Joint preload/torque: not
   established. The preload record is pending (docs/12, H003 sheet).
6. Blade holders H004 and the blade pair: purchase-to-drawing blades. Edge
   alignment and apex overlap 0.2 mm per machine_spec are a commissioning
   measurement, not an assembly claim.

Why these fits matter: the Ø49.97 register is what locates the adapter on
the carriage, and the M5×16 10.9 joint carries the 8.0 kN preload of the
adapter study. The 0.10 diametral clearance on the motor pilot centres the
motor without binding. The coupling stack numbers stop the bellows from
passing screw thrust into the motor bearings.

If you get stuck: if the bellows look stretched, the stack is wrong. Back
the hubs off, set the free-length coupling with the ≥0.53 mm insertion and
≥12.58 mm end gap, and only then tighten.

### P5 — cassette/head/guide interplay

1. Land both cartridges onto their F201 bases, with the wire contact station
   at S108.75 and the nips at X75.5/138.0 (docs/12 §6).
2. Install the feed motors 24HS40-5004D-E1000 with the belt drive. Verify
   belt alignment on both pulleys (5JS9 keyways, belt 25 mm wide) and the
   minimum engagement per the MISUMI belt records.
3. Close the wire path: payoff (P7) → left guide → front nip → cut plane →
   rear nip → right guide → output. The 880 mm straight wire route and both
   1.2/4.5 mm wire envelopes must pass the mechanism wire checks. They are
   already code-verified; re-verify them physically with a taut wire.

Why this matters: every station in the chain must sit on the same 880 mm
line, or the wire rubs, bends or jams as soon as the feed starts.

If you get stuck: if the taut wire touches something unexpected, check the
station order against the route before you adjust anything. A station that
is bolted in the wrong order looks identical to a bad adjustment.

### P6 — measuring module

1. Bolt the M206 bracket and M207 counter-roller column to the bed pattern
   at the X−205 side. Press the two 626-2Z bearings into M203 (K6 seats) and
   fit the M202 shaft (6 h6, seats 6 j5) with the M204 pivot arm, M209 cap
   and SWU10.5-35 spring. Set the wheel force into 6–19 N (physical spring
   force check).
2. Fit the ifm E60137 wheel (6 mm bore) on the shaft with its radial clamp
   (the hub screw detail is unpublished, confirm at receipt) and the
   supplied encoder coupling. The M201 R2 plate holds the ifm RU3100 (flange
   face on A, 3× M4×12 into the PCD42 taps, 4× M4×6 support screws in the
   M203 counterbores, shaft tip 10 mm from the face).
3. Wheel runout and circumference: 200 mm is the ifm claim with no
   published tolerance. Measure and enter into the calibration
   ([docs/13](13-measuring-interfaces-and-calibration.md), physical).

Why these fits matter: the 6–19 N band is the contact force that makes the
wheel track the wire without slipping, and the measured circumference is
what turns encoder counts into a cut length.

If you get stuck: a jumpy encoder reading is usually runout or the coupling,
so measure the wheel before you touch the firmware.

### P7 — payoff (independent floor assembly)

1. Weld the P004/P005 posts (18 mm S235JR, 70 wide) to P003 (430×560×8)
   with a fixture holding both Ø47 H7 pockets coaxial (≤0.1 mm across posts
   after welding, physical). Weld check as P1.
2. Press the two 6204-2Z/C3 bearings into the pockets from the spool side.
3. Slide the P002 spindle (Ø20 h6) through the P001 welded spool (Ø20.5
   flange bores) and both bearings. Check runout ≤0.010 between seats
   (P002 sheet).
4. Fit the P006 brake disc (Ø20 H7, 6 JS9 keyway) outboard of the right post
   with its key and M6×8 setscrew. Thread the M20×1.5 nut (KM-class plus
   tab washer) and set the axial float so the spool spins freely, 0.1–0.3 mm
   end float (physical).
5. Mount the P007 caliper bracket, fit the pads and the SWU-class adjuster
   spring (purchase-to-requirement) and set the braking tension by trial
   (physical).
6. Anchor P003 through its 4× Ø13 holes. Full-spool mass, spindle
   deflection, brake adjustment and runaway checks are
   [docs/26 §4/§6](26-guard-and-payoff-selection.md) gates.

Why these fits matter: the coaxial pockets keep the spindle true so the
spool runs without wobble, the 0.1–0.3 mm float lets the spool spin freely
without end slop, and the brake is a friction tension brake only. It is not
a safety brake.

If you get stuck: a spool that binds usually has too little end float.
Measure the 0.1–0.3 mm float and confirm the posts are still coaxial after
welding before you blame the bearings.

### P8 — guard set

1. Weld the G106 SLC mounting plate to the door jamb. Drill the SLC pattern
   only from the Bernstein mounting drawing (docs/26 §2).
2. Hang the G105 door (5 mm PC in a 20×20×2 tube frame) on two hinges rated
   ≥15 kg, fit the handle, fit the ACF-1 actuator to the door and the
   SLC-F-024-20/20-R4 body to the plate. Verify insertion depth, alignment
   and locking travel physically.
3. Fit the G101/G102 side panels (DC01 2 mm, Ø30 wire pass-throughs at
   Y0/Z112), the G103 rear and G104 roof, with M6×12 8.8 screws at ≤200 mm
   pitch into rivet nuts, and edge grommets on the apertures.
4. Safety-distance, aperture and lock-release validation per docs/26 §6 and
   the guard/payoff release template are physical gates.

Why these fits matter: the lock body and actuator must engage every time,
so insertion depth and locking travel are physical checks, and the panel
pitch and grommets are what keep the 30 mm apertures as wire clearances,
not finger openings.

If you get stuck: a door that will not lock is an alignment problem. Work
from the Bernstein manual and the archived drawing, and check the actuator
approach and the stop set inside 0.2–2 mm before drilling anything.

### P9 — cabinet and wiring

Per [docs/23](23-cabinet-electrical-assembly.md) and
[docs/22](22-controls-and-wiring.md): enclosure, PE, terminals, drives,
supplies, XPS, contactors, IF-B01, glands and field wiring. The DC duty,
thermal and protection calculations close from the desk (see the 2026-09-15
electrical package in docs/22). The energisation gate and stop-time
measurement remain physical first-article gates.

Why this matters: the desk calculations set the ratings, but only a physical
energisation sequence proves the safety channels, the contactor opening and
the bus decay behave as designed.

If you get stuck: if nothing powers up, go back to the point-to-point record
first. Never substitute a fuse or terminal rating to make a test pass.

## 3. Interface-fit register (source-backed values)

| Pair | Fit / value | Source |
|---|---|---|
| F101 seats ↔ 6001/6002 bores | j5 seat matched 3–6 µm interference to measured bores | bearing-fit report, docs/12 |
| UMCL40 collar ↔ 10 h6 shaft | bore 10.000–10.030; ≤0.039 diametral clearance; key transmits torque | docs/12, catalogue evidence |
| HTPA pulley ↔ 14 h7 shaft | 14 H7 bore + 5 JS9 keyway | selected-mechanism/transport CSV |
| R+W MK2 hubs ↔ motor Ø10 D-flat / Ø12 shaft (feed axes) | hub bores 10/12; axial stack ≥0.53 mm insertion, ≥12.58 mm end gap | engineering/README R6, H003 sheet |
| R+W MK2/100/50/8/14 ↔ screw / motor (head axes) | hub bores 8 H7 / 14 H7; screw input Ø8 h7 × 18 mm; Ø46 coupling service cavity at the fixed end; axial stack ≥0.53 mm insertion, ≥12.58 mm end gap | docs/39, engineering/README R6, H003 sheet |
| HGR20 rails ↔ H005 seats | two rails per axis at 60 mm pitch; seats milled flat 0.02/300; M5 at 6 Nm | docs/39 §5, H005-H006 sheet |
| H006 ↔ HGH20CA blocks | four block lands per carriage; preload per supplier | docs/39 |
| SFU1605 screw ↔ BK12/BF12 supports | end-machined Ø8 h7 × 18 mm input; BK12 fixed and preloaded, BF12 floating; DSG16h housing in the H006 pocket | docs/39 |
| Axis travel and accuracy | travel ≥194 mm usable + 8 mm reserve; C7 ±0.05 mm/300; repeatability ±0.01–0.02 mm class; ±0.1 mm placement budget; ±0.02 mm repeatability target over 194 mm after 1 h run-in | docs/39 |
| H003 pilot ↔ 34E1K motor | Ø73.10+0.02/0 vs Ø73.10: 0.10 diametral clearance | catalogue evidence |
| H003 F0 register ↔ axis carriage | Ø49.97±0.01 into carriage register (H006 reproduces the KK86 interface) | H003 sheet, docs/39 |
| A12 ring ↔ shaft groove | groove 11.45±0.02 vs ring 11.50−0.11; conditional clearance — audited in [docs/28](28-physical-contradiction-audit.md) | docs/15, measurement report |
| KM2 nut ↔ M15×1 thread; MB2 tang ↔ 4.2 slot | turn one outer tab into a slot; replace MB2 every reinstall | docs/12 |
| M202 shaft ↔ 626-2Z bores | 6 h6 shaft, 6 j5 seats, K6 housing pockets | measurement record |
| ifm E60137 wheel ↔ M202 | 6 bore on 6 h6; hub clamp detail pending | catalogue evidence |
| RU3100 encoder ↔ M201 | synchro flange on A; 3×M4×12 PCD42; tip 10 mm | M201 sheet |
| P002 spindle ↔ P001 spool | Ø20 h6 in Ø20.5 bores (0.5 clearance) | docs/26, P002 sheet |
| P002 ↔ 6204 pockets | Ø20 h6 in bearing bores; pockets Ø47 H7 | docs/26, P004–P007 sheet |
| PSFJ12-240 posts ↔ F201 bores | Ø12 g6 in Ø12.01 bores, clamp slits | mechanism spec |
| LHFRW12 ↔ posts | Ø12 g6 bore on Ø12 g6 shaft, floating linear pair | mechanism spec |
| Wire ↔ F206 inserts | insert bore = wire OD+0.5 | mechanism spec |
| Guard SLC pocket ↔ body | 0.5 mm CAD clearance class; drilling per Bernstein | docs/26 |

Every row with a measured value (bearings, encoders, collars, weld
positions) requires the corresponding incoming or in-process inspection
record before the assembly step that consumes it. The blank
[incoming-inspection template](inspection/incoming-inspection-template.csv)
governs. No fit in this table has been physically confirmed.

## 4. Warnings and holds

Torque/preload records, matched bearing measurement, weld inspection,
flatness/runout measurements, spring-force settings, belt tension, blade
alignment, stop-time, risk-assessment sign-off, full-spool weighing and
cycle-time confirmation all require the physical machine. This document
sequences them so that nothing is left to improvisation, but it does not
pretend they are done.
