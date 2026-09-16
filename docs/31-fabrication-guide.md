# Fabrication guide — build the STARZ S1-B, step by step

**Revision:** `S1-B-FAB-R1-2026-09-16`

This is the ordered build manual. Follow the sections in order. Every step
names the part, the operation, the check and where to record the result.
Records go in the blank templates under `docs/inspection/` and
`engineering/generated/qualification/templates/`.

Tools assumed: laser or waterjet cutter, press brake, CNC mill, lathe with
grinding (or outsourced grinding), MIG welder, drill press, tap set, torque
wrench, 0–25 mm micrometer, bore gauges, DTI with stand, gauge pins, digital
multimeter, insulation tester.

---

## 1. Order everything

Order from the exact rows below. Where a price is not public, use the
quotation request in [the plan §2.2](24-production-and-release-plan.md).

**Phase A — mechanism (order first; long lead items marked *):**

| Item | Exact order | Qty | Price observed |
|---|---|---|---|
| HIWIN axis * | KK8610P340A1F0 (article 10.00026) | 2 | quotation required |
| R+W coupling * | MK2/100/50/8/14 | 2 | quotation required |
| R+W coupling * | MK2/100/50/10/12 | 2 | quotation required |
| Cut motor kit | 1-CL86T-P85-V41 (34E1K-85, CL86T-V41, CM2M-18, CE2M, CABLE-PC-1) | 2 | kit contents per catalogue evidence |
| Feed motor | 24HS40-5004D-E1000 | 2 | USD 36.87 (STEPPERONLINE) |
| Feed driver | CL57T-V41 | 2 | shop price at purchase |
| 60 V supply | LE-350-60 | 2 | in motor kit / shop |
| 36 V supply | LE-350-36 | 1 | shop price at purchase |
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
| Kübler wheel | 8.0000.3123.0006 | 2 | quotation |
| Autonics encoder | E40S6-1024-6-L-5 | 2 | quotation |
| Dormer tap | E412M5 material 7441118 | 1 | per docs/14 |
| Fasteners | ISO4762 M5×16 (8), M5×25 (8), M4×20 (16), M3×8 (32), M3×10 (6), M10×35 (8); ISO7092 M5/M4/M3 washers; ISO8734 3m6×20 (16); DIN929 M10 (8); C45 key 5×5×28 (4) | — | traceable supplier |

**Phase B — controls/cabinet (order after §2 of the electrical record is
accepted):**

| Item | Exact order | Qty |
|---|---|---|
| Safety relay | XPSUAF13AP | 1 (EUR 133.50 observed) |
| E-stop | 3SU1851-0NB00-2AC2 | 1 (EUR 110.70 observed) |
| Guard lock / actuator | SLC-F-024-20/20-R4 6018200007 / ACF-1 3911742391 | 1 / 1 (EUR 143.50 / 17.90 observed) |
| Contactor | LC1D12BD | 3 (EUR 70.40 observed) |
| Fuse link | DF2CN02 | 1 |
| Fuse holder | DF2BA-class DIN-rail 10×38 fuse base (confirm identity) | 1 |
| 24 V supply | 6EP3332-7SB00-0AX0 | 1 |
| Pico | Raspberry Pi Pico 2 SC1631 | 1 (USD 5.00 observed) |
| Cooler | Rittal SK3303.500 | 1 (quotation) |
| Terminals | Phoenix PT 2,5 (3209510 ×n, 3209523 ×n), UT 4 (3044131 ×n) | per layout |
| Enclosure | Rittal AE ≥800×600×210 (confirm size) | 1 |
| Wire | H07V-K 0.75/1.0/1.5 mm² + ferrules | per §11.4 of controls |
| Glands | 2× M20 EMC metal gland (Lapp/Pflitsch identity confirm) | 2 |
| Pushbuttons/LED | 3SU1152 pair, 3SU1200 buzzer, IE5338 sensor, PLC-RSC-24DC/21 | per selected-controls.csv |

## 2. Machine the custom parts

Machine in this order; each part's drawing sheet is linked.

| Part | Stock | Operations (in order) | Check |
|---|---|---|---|
| F101 shaft ×2 | per [docs/12](12-feed-cartridge-fabrication.md) | turn, thread M15×1, keyseats, A12 grooves, blind M5 tap with E412M5 | matched j5 seats 3–6 µm interference to measured bores; docs/12 §checks |
| F102–F112 ×2 | per docs/12 | mill/lathe per record | docs/12 |
| H001 | 180×1040×20 tooling plate | mill flat, tap bed pattern | flat ≤0.05 |
| H002 ×2 | per head manifest | mill risers | parallel/height |
| H003 ×2 | 105×105×20 | mill both faces, registers Ø49.97/Ø73.10, counterbores, 4×M5 | [H003 sheet](../engineering/generated/drawings/H003-R6-adapter.pdf) |
| H004 ×2 | per head manifest | mill blade holders | per sheet |
| M201 | 50×50×8 | mill plate, pilot, 3 PCD holes | [M201 sheet](../engineering/generated/drawings/M201-R1-encoder-plate.pdf) |
| M202–M210 | per measurement manifest | mill/turn per record | docs/13 |
| F201–F208, F301–F305 | per mechanism manifest | mill/laser + weld frame | [manifest](../engineering/generated/mechanism/manifest.csv) |
| P002 | Ø20 ground bar | cut 660, thread M20×1.5 both ends | [P002 sheet](../engineering/generated/drawings/P002-R2-payoff-spindle.pdf) |
| P001 | 3 mm sheet + Ø60×2.5 tube | laser flanges Ø400, weld hub through flanges | flange bore Ø20.5 |
| P003/P004/P005 | 8/18 mm plate | laser, mill Ø47 H7 pockets + Ø20.5 bores, weld posts to base | [P004–P007 sheet](../engineering/generated/drawings/P004-P007-R2-payoff-plates.pdf) |
| P006 | 6 mm plate | laser Ø180, bore Ø20 H7, keyway 6 JS9 | sheet above |
| P007 | plate stock | laser caliper bracket | sheet above |
| G101–G106 | 2 mm DC01, 5 mm PC, S235 5 mm | laser/brake; G106 drill pattern ONLY from the Bernstein mounting drawing | [G101–G106 sheet](../engineering/generated/drawings/G101-G106-R2-guard-panels.pdf) |

## 3. Assemble (in this order)

**3.1 Frame and bed.** Weld F304 legs to F302 rails + F303 cross rails; weld
F305 feet; check foot pad coplanarity ≤1 mm. Bolt F301 bed with M10×35 +
DIN929 nuts. Level to Z=0 within 0.1 mm.

**3.2 Feed cartridges (both, on the bench).** Follow
[docs/12 §6 order](12-feed-cartridge-fabrication.md) exactly: rotor
preassembly → keyed rollers/pulleys → front module → rear bearing → belt.
Measure each bearing bore, finish seats to the measured 3–6 µm interference,
match-mark. Record in the F101 inspection CSV.

**3.3 Carriers and guides on the bed.** Bolt F201 bases; insert PSFJ12-240
posts (clamp slits); fit LHFRW12 bushings, F202 plates, F203 bridges, F208
caps, SWU17-60 springs, DIN 913 adjusters; set nips 50–75 N (measured).
Fit F205 holders + F206 inserts (wire OD + 0.5 clearance).

**3.4 Head.** H001 → H002 → KK86 stages → H003 adapters (F0 register
Ø49.97, 4×M5×16) → 34E1K-85 motors (pilot Ø73.10, 4×M5×25) → R+W couplings
(axial stack per the H003 sheet: ≥0.53 mm hub insertion, ≥12.58 mm end gap,
no bellows stretch) → H004 holders. Blade pair last, after §5.

**3.5 Feed motors and belts.** 24HS40 motors, HTPA pulleys (5JS9 keys),
HTBN belts, check belt alignment and minimum tooth engagement.

**3.6 Measuring module.** M206 bracket + M207 column on bed → 626 bearings
into M203 → M202 shaft → M204 arm + SWU10.5-35 → wheel force 6–19 N
(measured) → Kübler wheel (6H7 on 6h6, M4×6 set screw) → M201 plate + E40S6
encoder (pilot 19.98–20.00, tip 20 mm, 3×M3×10).

**3.7 Payoff (floor, independent).** Weld P004/P005 to P003 on a coaxial
fixture (pockets coaxial ≤0.1 after welding) → press 6204 bearings → P002
through P001 spool and bearings → P006 disc + key + M6×8 → M20×1.5 nut, set
0.1–0.3 mm spool float → P007 caliper + pads + adjuster spring → anchor P003
through 4×Ø13.

**3.8 Guard.** G106 plate to jamb (drill from Bernstein drawing) → hang
G105 door (2 hinges ≥15 kg, handle, ACF-1) → mount SLC body → verify
insertion/locking travel → fit G101–G104 panels + grommets (M6×12 at
≤200 mm pitch).

**3.9 Cabinet.** Follow [docs/23](23-cabinet-electrical-assembly.md) in its
numbered order: enclosure → PE bar → supplies → contactors → XPS → terminals
→ IF-B01 → drives → glands → field wiring → point-to-point verification.

## 4. Check the wire path

Taut-wire check: 1.2 and 4.5 mm wire from payoff through both guides, both
nips, the cut plane and the output without touching anything but the
intended contacts. Verify the mechanism wire-route checks pass on the
assembled machine.

## 5. Energisation (gated — read the warnings first)

Voltages/polarity → E-stop channel test → guard channel test → encoder
direction → drive enable → contactor opening + bus decay measurement →
stop-time measurement → record in the FAI template. No E1/E2 guard-release
circuit is authorized.

## 6. First article

Run the 30-piece per cell procedure in [docs/20](20-physical-qualification.md)
with raw data retention.

## 7. Warnings and holds (read once before starting, then per gate)

- No step in this guide is authorized to skip a measurement listed in its
  "Check" column. A missing record blocks the next dependent step.
- **Initial torque values** (class 8.8/10.9 dry, to be confirmed at first
  assembly and recorded): M3 1.2 Nm; M4 2.8 Nm; M5 6 Nm (steel) / 4.5 Nm
  (aluminium); M6 10 Nm; M8 24 Nm; M10 47 Nm; M20 spindle nut finger-tight
  against the tab washer plus 1/8 turn. The F0 M5×16 10.9 joint targets the
  8.0 kN preload of the adapter study — verify with a torque-preload test on
  spare parts and record the actual torque.
- Blade pair: purchase-to-drawing; edge qualification is physical.
- Bearing seats: matched-fit — never assume a nominal j5 seat fits a
  measured bore.
- Guard safety distance, aperture treatment and risk assessment must be
  signed off before first guarded movement (docs/26 §6, docs/28 §2.2).
- The XPS fuse is DF2CN02 (2 A) — a 10 A link does not protect the 5 A
  DC-1 relay outputs (docs/22 §11.3).
- LC1D12BD DC switching duty is unconfirmed: obtain the manufacturer DC
  utilization answer before relying on the three-pole series arrangement.
- Cabinet energisation is prohibited until the docs/22 §10 holds have
  evidence.
- Full-spool mass ≤30 kg until the spindle deflection re-check (docs/28
  §2.1); weigh the first reel.
- This guide is the fabrication path; the qualification, safety and release
  status of the machine is unchanged: nothing here releases fabrication
  results, and no row in the master plan may be closed by relaxing a
  criterion.
