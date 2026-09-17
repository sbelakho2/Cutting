# Fabrication guide — build the STARZ S1-B, step by step

**Revision:** `S1-B-FAB-R1-2026-09-16`

This is the ordered build manual. Follow the sections in order. Every step
names the part, the operation, the check and where to record the result.
Records go in the blank templates under `docs/inspection/` and
`engineering/generated/qualification/templates/`.

## 1. What you are building

The STARZ S1-B pulls wire of 1.2 mm or 4.5 mm diameter off a floor-standing
payoff spool, feeds it through two spring-loaded cartridges and a set of
guides, measures its length with a running wheel and an encoder, and cuts it
with two opposed 90° V blades mounted on a two-axis head inside a locked
guard. You build the welded steel frame and bed first, then the two feed
cartridges, the pressure carriers and wire guides, the head and its two axes,
the measuring module, the payoff stand, the guard set, and finally the
electrical cabinet. Every fit in this manual is measured, never assumed, and
each check you pass unlocks the next step.

## 2. Tools and measuring gear you need

- A laser or waterjet cutter with a bed big enough for the Ø400 spool
  flanges, the 430×560 payoff base and the 1080 mm frame rails.
- A press brake for the 2 mm DC01 guard panels.
- A CNC mill for the tooling plates: H001 at 180×1040×20, H003 at
  105×105×20, M201 at 66×66×8, and the H005/H006 axis plates.
- A lathe with grinding capability, or a grinding shop you trust, for the
  F101 shafts and the Ø20 h6 ground P002 spindle.
- A MIG welder with ER70S-6 wire for the frame, the payoff and the guard
  jamb.
- A drill press and a tap set from M3 to M10, including M4×0.7 and M5×0.8
  taps and one blind M5 tap, the Dormer E412M5.
- A torque wrench that reads from 1.2 Nm up to 47 Nm.
- A 0–25 mm micrometer, bore gauges that reach Ø6 to Ø73 mm, and gauge pins
  in the same range.
- A dial test indicator (DTI) on a stand, reading to 0.01 mm.
- A digital multimeter and an insulation tester for the cabinet work.

If any of this is missing, stop and get it first. Most of the checks in this
manual cannot be eyeballed.

## 3. Buying

Order from the exact rows below. Where a price is not public, use the
quotation request in [the plan §2.2](24-production-and-release-plan.md).

Phase A, the mechanism. Order this first. Rows marked * have a long lead
time, so order them before anything else.

| Item | Exact order | Qty | Price observed |
|---|---|---|---|
| Axis rail sets * | AX-R01: HGR20 400 mm rail + 2× HGH20CA block set, [Taiyin on made-in-china](https://taiyin6666.en.made-in-china.com/product/ztLrHCpjCsci/China-Hgr20-High-Accuracy-High-Qualit-Linear-Guide-Rail-Set-Hgr20-400mm-Hg20-Linear-Rail-and-Block.html) | 4 sets (2 per axis) | US$8.00–13.00 FOB/set, MOQ 1 |
| Axis screw sets * | AX-S01: SFU1605 C7 500 mm rolled ballscrew + flange nut + DSG16h housing + BK12/BF12 supports, end-machined option, [ALM on made-in-china](https://autolinearmotion.en.made-in-china.com/product/PtaYZwvVEukB/China-Sfu1605-500mm-600mm-800mm-C7-with-End-Machined-1605-Ball-Nut-Housing-Bk-Bf12-End-Support-Coupler-Rolled-Ball-Screw.html) | 2 sets (1 per axis) | US$5.00–10.00 FOB, MOQ 1 |
| Axis plates | H005/H006, own shop, per the [H005-H006 sheet](../engineering/generated/drawings/H005-H006-R1-axis-plates.pdf) | 1 pair per axis | own shop |
| R+W coupling * | MK2/100/50/8/14 (head axes: 8 H7 hub on the Ø8 h7 screw end, 14 H7 hub on the motor) | 2 | quotation required |
| R+W coupling * | MK2/100/50/10/12 (feed axes: motor Ø10 D-flat, shaft Ø12) | 2 | quotation required |
| Cut motor kit | 1-CL86T-P85-V41 (34E1K-85, CL86T-V41, CM2M-18, CE2M, CABLE-PC-1) | 2 | kit contents per catalogue evidence |
| Feed motor | 24HS40-5004D-E1000 | 2 | USD 36.87 (STEPPERONLINE) |
| Feed driver | CL57T-V41 | 2 | shop price at purchase; class fallback EUR173.78 (Amazon.de B0FYPDGC85) |
| 60 V supply | LE-350-60 | 2 | in motor kit / shop |
| 36 V supply | LE-350-36 | 1 | shop price at purchase; equivalent EUR77.04 (Amazon.de B0HDN818SJ) |
| SKF bearing | 6001-2Z/C3 | 8 | GBP 6.32 |
| SKF bearing | 6002-2Z/C3 | 8 | GBP 6.91 |
| SKF bearing | 626-2Z | 4 | GBP 4.49 |
| SKF bearing | 6204-2Z/C3 | 2 | GBP 8.68 |
| SKF lock nut / washer | KM 2 / MB 2 | 8 / 8 | GBP 7.90 / 2.15 |
| SEEGER ring | A12 DIN471 | 8 | quotation |
| MISUMI collar roller | UMCL40-10-20-MC | 8 | EUR 19.50 |
| MISUMI pulley | HTPA20S5M250-A-N14 | 4 | EUR 39.87 |
| MISUMI belt | HTBN225S5M-250 | 2 | EUR 25.24 |
| MISUMI bushing | LHFRW12 | 4 | EUR 14.58 |
| MISUMI post | PSFJ12-240 | 4 | EUR 11.70 |
| MISUMI spring | SWU17-60 | 2 | EUR 3.53 |
| MISUMI spring | SWU10.5-35 | 1 | EUR 1.80 |
| ifm wheel | E60137 (U:200 N:06) | 2 | automation24 EUR 57.00 ex VAT, in stock |
| ifm encoder | RU3100 | 2 | automation24 EUR 186.40 ex VAT, in stock |
| Dormer tap | E412M5 material 7441118 | 1 | per docs/14 |
| Fasteners | ISO4762 M5×16 (8), M5×25 (8), M4×20 (16), M3×8 (32), M4×12 (6), M4×6 (4), M10×35 (8); ISO7092 M5/M4 washers; ISO8734 3m6×20 (16); DIN929 M10 (8); C45 key 5×5×28 (4) | per list | Wegertseder: M5×16 10.9 50pcs EUR6.19 (2540-034); 3m6×20 dowels 10pcs EUR2.73 (1018-078) |

The custom axis in this table replaces the HIWIN KK86, per
[docs/39](39-custom-cut-axis.md). Both axes together cost roughly US$70–115
each, against USD 824.72 for the priced HIWIN C-grade. Order the screw with
the end-machined Ø8 h7 × 18 mm option, or machine that end locally. Never
clamp a raw Ø12 blank in the coupling. If the supplier offers a 10 mm lead
(SFU1610-class) at the same price, take it, because it keeps the 100
pulses/mm firmware map unchanged. If you can only get the 5 mm lead, update
the firmware `steps_per_mm` to 800 and record the choice in the order
confirmation and the firmware map constant. If the component supply fails,
the recorded fallback purchase leads are the HIWIN KK8610C-340A1-F0 at USD
824.72 per unit (VND 21,146,653, MISUMI VN, observed 2026-09-16, lead 23–99
days) and eBay item 406408632737, a genuine P-grade at a US$450 snippet.

Phase B, controls and cabinet. Order this only after §2 of the electrical
record is accepted.

| Item | Exact order | Qty |
|---|---|---|
| Safety relay | XPSUAF13AP | 1 (EUR 133.50 observed) |
| E-stop | 3SU1851-0NB00-2AC2 | 1 (EUR 110.70 observed) |
| Guard lock / actuator | SLC-F-024-20/20-R4 6018200007 / ACF-1 3911742391 | 1 / 1 (EUR 143.50 / 17.90 observed) |
| Contactor | LC1D12BD | 3 (EUR 70.40 observed) |
| Fuse terminals + links | Phoenix 3046032 ×5 (XPS output, +24V_DRVLOGIC, 3 drive-logic feeds) + ESKA 522620 2 A T 5×20 ×1 + ESKA 522617 1 A T 5×20 ×4 | EUR 7.99 / 1.49 / 1.49 observed 2026-09-16 |
| Mains MCB + RCD | Schneider Acti9 iC60N 2P C10 (A9F54210) + iID 2P 25 A 30 mA (A9Z21225) | 1 + 1 (RCD EUR 35.99 observed; MCB price at purchase) |
| 24 V supply | 6EP3332-7SB00-0AX0 | 1 |
| Pico | Raspberry Pi Pico 2 SC1631 | 1 (USD 5.00 observed) |
| Cooler | Rittal SK3303.500 | 1 (quotation) |
| Terminals | Phoenix PT 2,5 (3209510 ×n, 3209523 ×n), UT 4 (3044131 ×n) | per layout |
| Enclosure | Rittal AE ≥800×600×210 (confirm size) | 1 |
| Wire | H07V-K 0.75/1.0/1.5 mm² + ferrules | per §11.4 of controls |
| Glands | 2× M20 EMC metal gland (Lapp/Pflitsch identity confirm) | 2 |
| Pushbuttons/LED | 3SU1152 pair, 3SU1200 buzzer, IE5338 sensor, PLC-RSC-24DC/21 | per selected-controls.csv |

## 4. Making the parts

Make the parts in this order. Each recipe lists the stock, the operations in
order, and the check. Machine each part, measure it, record the measurement,
then move on. A missing record blocks the next dependent step.

**F101 rotor shaft, make 2.** Stock per [docs/12](12-feed-cartridge-fabrication.md).
Turn the shaft, thread it M15×1, cut the keyseats, cut the A12 grooves, and
tap the blind M5 hole with the E412M5. Check: matched j5 bearing seats with
3–6 µm interference to the measured bores, per docs/12 §checks. If the check
fails, measure the bearing bore again and finish the seat to that measured
bore, never to the nominal. A seat that comes out loose must be remade, not
shimmed or glued.

**F102–F112, make 2 of each.** Stock and operations per
[docs/12](12-feed-cartridge-fabrication.md). Mill or turn each part per the
record. Check: per docs/12.

**H001.** Stock 180×1040×20 tooling plate. Mill it flat and tap the bed
pattern. Check: flat ≤0.05 over the cassette and head footprints.

**H002, make 2.** Stock per the [head manifest](../engineering/generated/head/manifest.csv).
Mill the risers. Check: parallel and height per sheet.

**H003, make 2.** Stock 105×105×20. Mill both faces, cut the registers
Ø49.97 and Ø73.10, the counterbores and the 4×M5 holes. Check: per the
[H003 sheet](../engineering/generated/drawings/H003-R6-adapter.pdf).

**H004, make 2.** Stock per the head manifest. Mill the blade holders.
Check: per sheet.

**H005/H006 axis plates, one pair per axis.** Stock 6082-T6 per the
[H005-H006 sheet](../engineering/generated/drawings/H005-H006-R1-axis-plates.pdf).
H005 is the 20×86×440 base: mill the rail seats flat to 0.02/300, the
BK12/BF12 lands and the 6×Ø6.6 grid on 46/100. H006 is the carriage plate
with the 60×75 pad at 46 mm height, the Ø49.97 F0 register, 4×M5 on PCD70,
4×M6 on 46×46, the DSG16h pocket and the four HGH20CA lands. Check: per
sheet.

**M201.** Stock 66×66×8. Mill the plate, the seat bore, the 3 PCD42 holes
and the 4 M4 taps. Check: per the
[M201 sheet](../engineering/generated/drawings/M201-R2-encoder-plate.pdf).

**M202–M210.** Stock per the measurement manifest. Mill or turn per the
record. Check: per [docs/13](13-measuring-interfaces-and-calibration.md).

**F201–F208 and F301–F305.** Stock per the mechanism
[manifest](../engineering/generated/mechanism/manifest.csv). Mill or laser
cut, then weld the frame. Check: per the manifest.

**P002 spindle.** Stock Ø20 ground C45 bar. Cut it to 660 mm and thread
M20×1.5 on both ends. Check: per the
[P002 sheet](../engineering/generated/drawings/P002-R2-payoff-spindle.pdf),
runout ≤0.010 between the bearing seats.

**P001 spool.** Stock 3 mm sheet plus Ø60×2.5 tube. Laser the flanges at
Ø400 and weld the hub through both flanges per
[docs/34](34-weld-and-assembly-procedures.md): MIG 4 mm fillet, alternating
quadrants. Check: flange bore Ø20.5, flange runout ≤1.0 mm at Ø400, hub
concentricity to the Ø20.5 bores ≤0.5.

**P003/P004/P005.** Stock 8/18 mm plate. Laser cut, mill the Ø47 H7 pockets
and the Ø20.5 bores, then weld the posts to the base on the coaxial fixture
per [docs/34](34-weld-and-assembly-procedures.md). Check: per the
[P004–P007 sheet](../engineering/generated/drawings/P004-P007-R2-payoff-plates.pdf),
pockets coaxial ≤0.1 after welding and the spindle axis perpendicular to the
base top ≤0.15/300.

**P006.** Stock 6 mm plate. Laser it at Ø180, bore Ø20 H7, keyway 6 JS9.
Check: per the sheet above.

**P007.** Stock plate. Laser the caliper bracket. Check: per the sheet above.

**G101–G106.** Stock 2 mm DC01, 5 mm PC, S235 5 mm. Laser and brake. For
G106, drill the pattern ONLY from the Bernstein mounting drawing. Check: per
the [G101–G106 sheet](../engineering/generated/drawings/G101-G106-R2-guard-panels.pdf).

## 5. Assembly steps, in this order

### 5.1 Frame and bed

Weld the F304 legs to the F302 long rails and F303 cross rails, then weld
the F305 feet on. Check the weld all round, with no undercut, and check the
four foot pads sit coplanar within 1 mm over the 1080×440 rectangle. If they
do not, shim or adjust before you weld more, and never grind a foot down
without measuring first. Then bolt the F301 bed (EN AW-6082 T651,
640×1120×25) on with the M10×35 class 8.8 screws into the DIN929 M10 weld
nuts. Check the bed top face flat ≤0.05 over the cassette and head
footprints after mounting. Anchor the feet with 4× Ø13 floor anchors and
level the bed to Z=0 within 0.1 mm over the travel.

If you get stuck: a frame that will not level usually has a twisted bench or
an unwelded foot. Put the level on the bench first, then re-check the four
foot pads before you touch the bed bolts.

### 5.2 Feed cartridges (both, on the bench)

Follow the [docs/12 §6 order](12-feed-cartridge-fabrication.md) exactly:
rotor preassembly, then keyed rollers and pulleys, then the front module,
then the rear bearing, then the belt. Use the 6001-2Z/C3 rear non-locating
bearing and the 6002-2Z/C3 front locating bearing, keep the KM2+MB2
retention with the MB2 tang in the shaft slot before the key, fit the
UMCL40-10-20-MC collars on the 10h6 shaft, the HTPA20S5M250-A-N14 pulley,
the HTBN225S5M-250 belt and the 5JS9 keys, then the SEEGER A12 rings and
the F112 front module with its 3m6×20 dowels and M4×20 screws. The
code-checked insertion paths apply: MB2 tang before the key, the
preassembled rotor/pulley/front module in from +S with the module pins
removed, the rear bearing in from −S, and dowel service-punch access.
Measure each bearing bore, finish the seats to the measured 3–6 µm
interference, match-mark them, and record in the F101 inspection CSV.
Belt tensioning and the M5 blind-hole tap depth follow
[docs/14](14-f101-tapping-operation.md).

If you get stuck: a bearing that will not seat means the bore is wrong.
Measure the bore first and never force the bearing in. A bearing that drops
in means the seat is undersize and must be remade, not retained with
Loctite.

### 5.3 Carriers and guides on the bed

Bolt the F201 bases to the bed. Fit the PSFJ12-240 posts into the Ø12.01×30
bores with clamp slits (0.01 nominal clearance, clamped). Slide the LHFRW12
bushings onto the posts, then assemble the F202 plates (Ø21.01 bushing
bores, 4×M4 PCD32, 10 dowel holes) with the F203 bridges, F208 spring caps
and SWU17-60 springs. Set the DIN 913 adjusters so each nip lands in
50–75 N, measured, not guessed. Fit the F205 holders at X38–47 with the
F206 brass inserts sized to wire OD + 0.5 clearance, with the 60° lead-in
cones facing the wire entry. Check the guide insert axis sits on the wire
axis within 0.1 mm.

If you get stuck: if you cannot reach the 50–75 N window, move the DIN 913
adjusters in small steps and re-measure after each one. The springs are
catalogue parts, so the force comes from the adjuster position, not from
stretching the spring.

### 5.4 Head

Build the head in this order: H001, then H002, then the two custom axes, then
H003 adapters, then the 34E1K-85 motors, then the R+W couplings, then the
H004 holders. Fit the blade pair last, after the energisation section.

Each axis goes together per [docs/39 §5](39-custom-cut-axis.md). Mill the
H005 rail seats flat to 0.02/300, bolt the two HGR20 rails at 60 mm pitch,
align them with a gauge bar and torque the M5 screws to 6 Nm. Bolt H006 onto
the four HGH20CA blocks, mount the DSG16h nut housing in its pocket, install
the screw through BK12 (fixed, preloaded angular bearings) and BF12
(floating), adjust the nut preload per the supplier setting and verify the
drag. Mount both axes on H001/H002, run them in for 1 hour, then re-measure
block preload and repeatability, target ±0.02 mm over the 194 mm travel with
the DTI. Check carriage travel parallel to X within the stage specification
(physical run-in).

Bolt the H003 adapters to the carriages with the F0 register at Ø49.97 and
4× M5×16 class 10.9 screws on PCD70. Mount the 34E1K-85 motors with the
pilot at Ø73.10 and 4× M5×25 on the 69.58 grid. Fit the R+W couplings per
the H003 sheet axial stack: at least 0.53 mm of hub insertion, at least
12.58 mm of shaft-end gap, and the bellows never stretched. Then the H004
blade holders.

If you get stuck: if the bellows look stretched, the axial stack is wrong.
Back the hubs off and set the free-length coupling again, keeping the
0.53 mm insertion and 12.58 mm end gap before you tighten anything.

### 5.5 Feed motors and belts

Land both cartridges onto their F201 bases with the wire contact station at
S108.75 and the nips at X75.5/138.0 (docs/12 §6). Mount the 24HS40 motors
and the belt drive, using the HTPA pulleys with 5JS9 keys and the HTBN
belts, 25 mm wide. Check belt alignment on both pulleys and the minimum
tooth engagement per the MISUMI belt records.

If you get stuck: a belt that walks off the pulley means the two pulley
planes are not coplanar. Check the keyed pulley faces with a straight edge
before you blame the belt tension.

### 5.6 Measuring module

Bolt the M206 bracket and M207 column to the bed at the X−205 side. Press
the two 626 bearings into M203 (K6 seats), then fit the M202 shaft (6 h6,
seats 6 j5) with the M204 arm, M209 cap and the SWU10.5-35 spring. Set the
wheel force into 6–19 N, measured. Fit the ifm E60137 wheel (6 bore on 6h6,
hub clamp; the hub screw detail is unpublished, confirm at receipt) and the
supplied encoder coupling. The M201 R2 plate holds the ifm RU3100 with its
flange face on A, 3× M4×12 into the PCD42 taps and 4× M4×6 support screws
in the M203 counterbores, shaft tip 10 mm from the face. Measure the wheel
runout and circumference: 200 mm is the ifm claim with no published
tolerance, so measure it and enter the value into the calibration
(docs/13).

If you get stuck: if the encoder reading jumps, check the wheel runout and
the coupling before you touch the firmware. The 200 mm figure is a claim
until you measure your own wheel.

### 5.7 Payoff (floor, independent)

Weld the P004/P005 posts to the P003 base on a coaxial fixture: a Ø20 gauge
pin through both finished Ø20.5 bores and Ø47 gauge plugs in both bearing
pockets, with the pockets coaxial ≤0.1 after welding. Press the two 6204
bearings into the pockets from the spool side. Slide the P002 spindle
through the P001 spool and both bearings. Fit the P006 disc outboard of the
right post with its key and M6×8 setscrew, thread the M20×1.5 nut (KM-class
plus tab washer) and set the axial float so the spool spins freely, 0.1–0.3
mm end float. Mount the P007 caliper bracket, fit the pads and the
adjuster spring, and set the braking tension by trial. Anchor P003 through
its 4× Ø13 holes. The brake is a friction tension brake, not a safety brake.
Full-spool mass, spindle deflection, brake adjustment and runaway checks
are [docs/26 §4/§6](26-guard-and-payoff-selection.md) gates.

If you get stuck: a spool that binds usually has too little end float.
Measure the 0.1–0.3 mm float before you blame the bearings, and check the
posts are still coaxial after welding.

### 5.8 Guard

Weld the G106 plate to the door jamb and drill the SLC mounting pattern
ONLY from the Bernstein mounting drawing, never from a bounding box. Hang
the G105 door (5 mm PC in a 20×20×2 tube frame, door weight ≤15 kg) on two
hinges rated ≥15 kg, fit the handle and the ACF-1 actuator, and mount the
SLC-F-024-20/20-R4 body to the plate. Verify insertion depth, alignment and
locking travel physically. Fit the G101/G102 side panels (Ø30 wire
pass-throughs at Y0/Z112), the G103 rear and G104 roof, with M6×12 class
8.8 screws at ≤200 mm pitch into rivet nuts, and edge grommets on the
apertures. The guard safety distance, aperture treatment and risk assessment
must be signed off before the first guarded movement (docs/26 §6, docs/28
§2.2).

If you get stuck: a door that will not lock is an alignment problem. Work
from the Bernstein manual and the archived drawing, and check the actuator
approach and the stop set inside 0.2–2 mm before you drill anything.

### 5.9 Cabinet

Follow [docs/23](23-cabinet-electrical-assembly.md) in its numbered order:
enclosure, PE bar, supplies, contactors, XPS, terminals, IF-B01, drives,
glands, field wiring, point-to-point verification. Energising the cabinet is
prohibited until the docs/22 §10 holds have evidence.

If you get stuck: if nothing powers up, go back to the point-to-point
record first. Do not substitute a fuse or a terminal for a different rating
to make a test pass.

## 6. Check the wire path

Run the taut-wire check with 1.2 and 4.5 mm wire from the payoff through
both guides, both nips, the cut plane and the output, without touching
anything but the intended contacts. The 880 mm straight route and both wire
envelopes must pass the mechanism wire-route checks on the assembled
machine. If a wire touches a panel, a post or a bracket, stop and correct
the routing before anything else.

## 7. Powering up (gated — read the warnings first)

Read §9 of this guide, then follow this order: voltages and polarity,
E-stop channel test, guard channel test, encoder direction, drive enable,
contactor opening plus bus decay measurement, stop-time measurement, and
record everything in the FAI template. No E1/E2 guard-release circuit is
authorized. If any earlier step in this guide is missing its record, do not
power up.

If you get stuck: a failed E-stop or guard channel test is a wiring or
termination fault. Fix it with the power off, using the point-to-point
record, and never bridge a safety contact to continue.

## 8. First article

Run the 30-piece per cell procedure in [docs/20](20-physical-qualification.md)
with raw data retention. The mandatory cells are wire OD 1.2 and 4.5 mm at
commanded lengths of 200 and 1000 mm, with Cpk ≥ 1.33 per cell.

## 9. Warnings and holds (read once before starting, then per gate)

1. No step in this guide is authorized to skip a measurement listed in its
   check. A missing record blocks the next dependent step.
2. Initial torque values (class 8.8/10.9 dry, to be confirmed at first
   assembly and recorded): M3 1.2 Nm; M4 2.8 Nm; M5 6 Nm (steel) / 4.5 Nm
   (aluminium); M6 10 Nm; M8 24 Nm; M10 47 Nm; M20 spindle nut finger-tight
   against the tab washer plus 1/8 turn. The F0 M5×16 10.9 joint targets
   the 8.0 kN preload of the adapter study. Verify with a torque-preload
   test on spare parts and record the actual torque.
3. Blade pair: purchase-to-drawing. Edge qualification is physical.
4. Bearing seats: matched-fit. Never assume a nominal j5 seat fits a
   measured bore.
5. Guard safety distance, aperture treatment and risk assessment must be
   signed off before first guarded movement (docs/26 §6, docs/28 §2.2).
6. The XPS fuse is an ESKA 522620 2 A T 5×20 link in a Phoenix 3046032
   fused terminal. A 10 A link does not protect the 5 A DC-1 relay outputs
   (docs/22 §11.3). The 2 A rating is fixed: do not substitute a larger
   link.
7. LC1D12BD DC switching duty is unconfirmed. Obtain the manufacturer DC
   utilization answer before relying on the three-pole series arrangement.
8. Cabinet energisation is prohibited until the docs/22 §10 holds have
   evidence.
9. Full-spool mass ≤30 kg until the spindle deflection re-check (docs/28
   §2.1). Weigh the first reel.
10. This guide is the fabrication path. The qualification, safety and
    release status of the machine is unchanged: nothing here releases
    fabrication results, and no row in the master plan may be closed by
    relaxing a criterion.
