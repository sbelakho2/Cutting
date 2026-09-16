# Guard and payoff selection — S1-B-GP-R2

**Revision:** `S1-B-GP-R2-2026-09-13`
**Status:** fabrication-ready selection record for the internal build: the
parts below are ordered and made per the
[fabrication guide](31-fabrication-guide.md) §2–3. Every safety and
commissioning gate in section 6 remains and must close before energised
operation.

This record advances the [preliminary CAD accommodation](25-preliminary-guard-and-payoff.md)
to explicit material, component and load-path selections for the internal
build. The parts are fabricated per the guide; the commissioning gates below
(risk assessment, safety validation, physical measurements) must close
before energised operation. `status.json` continues to
report `full_machine_integrated:false`, `physical_results:false`,
`fabrication_released:false` and all guard/payoff validation flags false.

## 1. What changed from R1

R1 modelled five static enclosure panels, a lock mounting plate and five
payoff envelopes with unselected materials. R2 keeps the R1 external
dimensions (they were verified against the 1.2/4.5 mm wire route and the
507-occurrence assembly) and adds:

1. explicit panel materials and joining/fastener classes;
2. a front-door construction with hinges, handle and actuator mounting;
3. a real payoff spindle/bearing/brake architecture replacing the 39 mm
   spindle envelope with a Ø20 h6 C45 shaft on two SKF 6204-2Z/C3 bearings;
4. a friction-brake design with a catalogue spring and a brake disc;
5. a documented static load envelope for the payoff assembly.

R2 records the Bernstein mounting geometry from the manufacturer documents
retrieved 2026-09-16 (below), but does **not** provide the drilled G106
pattern in CAD, the safety distance calculation, a completed risk
assessment, or measured static/runaway loads. Those remain gated in
section 6.

## 2. Enclosure selections

The machine envelope is a closed guard of inner half-width 345 mm (X), panels
at Y −625…625 mm and Z 0…320 mm per [mechanism specification](../shared/mechanism_spec.hpp).
The 30 mm wire pass-throughs at Y=0, Z=112 mm remain **clearance apertures,
not finger-safe openings**; their safety treatment is a hold.

| CAD ID | Item | Selection | Basis / hold |
|---|---|---|---|
| G101, G102 | Left/right side panels | DC01 steel sheet, 2 mm, powder-coated RAL 7035; 30 mm apertures with burr-free deburred edges | Traceable steel stock; the aperture is a wire clearance, not a safety gap |
| G103 | Rear panel | DC01 steel sheet, 2 mm, powder-coated RAL 7035 | Fixed panel; bolted to frame |
| G104 | Roof | DC01 steel sheet, 2 mm, powder-coated RAL 7035 | Fixed panel |
| G105 | Front door | 5 mm clear polycarbonate (Makrolon-class) door panel in a 20×20×2 S235JR steel tube door frame | Visibility plus impact resistance; PC grade/impact verification is a hold |
| G106 | Lock mounting plate | S235JR plate, 5 mm, welded to the door frame jamb | Manufacturer drawing retrieved 2026-09-16 (Bernstein 0800000865 p.8, archived `bernstein-slc-installation-manual-0800000865.pdf`): 4×M5 fixing (Ø5.4 clearance holes) on the 30×192 mm face, rows 161.6 mm apart / 15.2 mm from the 192 mm ends, columns 22 mm apart; flat surface, any orientation. Transfer the exact coordinates from the archived drawing to the G106 sheet before drilling — do not drill from the bounding box |

Door hardware (not fabricated, no supplier SKU is claimed):

- 2 × adjustable steel door hinges for a ≤15 kg door, zinc-plated,
  purchase-to-requirement from a traceable industrial hardware supplier;
- 1 × door handle (non-safety) for operator grip;
- the Bernstein ACF-1 actuator mounts on the door, the SLC-F-024-20/20-R4
  body (30 × 192 × 44 mm, 4 × M5, IP67, 3 × M20×1.5 entries) mounts on the
  G106 plate on a flat surface in any orientation (Bernstein manual
  0800000865 and datasheet 6018200007, archived 2026-09-16); ACF-1
  (3911742391) requires an approach radius ≥400 mm, the actuator-to-head
  alignment window is 29 + 3 mm in X/Y and a 2 mm overtravel remains with
  the guard stop set inside 0.2–2 mm; no lateral force may act on the head;
  the 0.5 mm CAD pocket clearance is geometry only. Actual actuator
  insertion depth, misalignment tolerance and locking travel still require
  a physical fit check.

Panel fasteners: M6×12 class 8.8 button-head screws with M6 washers into
M6 rivet nuts or weld nuts on the frame at ≤200 mm pitch (final pattern is a
fabrication detail, not released). Wire pass-through edge guards: split
grommet strip, purchase-to-requirement.

## 3. Payoff architecture (replaces P001–P005 concept state)

| CAD ID | Item | Selection |
|---|---|---|
| P001 | Spool assembly | Two S235JR laser-cut flanges, Ø400 × 3 mm, each with a Ø20.5 shaft bore, welded to a Ø60×2.5 mm ERW hub tube, 160 mm flange spacing; painted. The spool assembly rides on the Ø20 spindle through the two flange bores; axial float is limited by shaft collars (purchase-to-requirement). |
| P002 | Spindle | C45, Ø20 h6 ground, length 660 mm: Ø20 through both support posts (bearing seats), M20×1.5 retaining nuts each end; the M20 threads are not modeled in the CAD |
| P004, P005 | Support posts | S235JR plate, 18 mm, laser-cut 70×18 mm towers with a Ø47 H7 bearing pocket 14 mm deep from the inner face and a Ø20.5 spindle bore through the plate, welded to the base; M8 base bolts to P003 |
| P003 | Base | S235JR plate, 430×560×8 mm, 4 × Ø13 anchor holes; bolted to the floor or to an extended frame rail |
| P006 | Brake disc (new) | S235JR, Ø180 × 6 mm, bore Ø20 H7 on the spindle outboard of the right post; keyway and setscrew are fabrication details |
| P007 | Brake caliper (new) | S235JR bracket + two 40×20×8 mm friction pads + a MISUMI SWU-class compression spring applying adjustable pad pressure through an M8 DIN 933 adjuster; pads and spring are not modeled in the CAD |

Payoff bearings: 2 × **SKF 6204-2Z/C3** (20×47×14 mm),
[Simply Bearings product page](https://www.simplybearings.co.uk/products/6204-2z-c3-skf),
GBP 8.68 observed 2026-09-13 (merchandise, not delivered). Quality-equivalent
comparison observed on the same page family:
[FAG 6204-C-2Z-C3](https://www.simplybearings.co.uk/products/6204-c-2z-c3-2z-c3-fag)
GBP 5.75 — a substitution requires an approved change record.

Spindle retention: 2 × M20×1.5 lock nuts with tab washer (KM-class, purchase
with the nut to supplier spec) and 2 × shaft collars; exact identities are
purchase-to-requirement, not claimed catalogue SKUs.

The brake is a friction tension brake, not a safety brake: it holds light
tension against spool rotation during payout and is adjustable by the M8
adjuster. It provides **no** guaranteed stop, no holding-brake function and
no energy-removal function. Runaway/backlash behaviour must be measured.

## 4. Static load envelope (analytical only)

| Quantity | Value | Basis |
|---|---|---|
| Spool flange mass | 2 × π/4·(0.4 m)²·0.003 m·7850 kg/m³ ≈ 5.92 kg | Steel flange pair only |
| Spool hub tube | π/4·(0.06²−0.055²)·0.16·7850 ≈ 0.57 kg | Ø60×2.5 ERW hub 160 mm |
| Wire mass | unknown; must be weighed per lot | Wire diameter 1.2/4.5 mm; full-spool mass is a measured input |
| Static radial load per bearing | (spool mass + wire mass)/2 | 6204 C0 = 6.55 kN permits far more than the expected ≤60 kg half-load; this is a sanity bound, not a qualification |
| Spindle bending | not analysed | A beam check and the physical deflection record are required before fabrication |

The 6204 load rating is a catalogue bound. It does not replace: weighing the
full spool, measuring spindle deflection and post weld quality, braking force
adjustment trials, or a runaway/overspeed study.

## 5. CAD and source state

The R1 CAD envelopes in `mechanism_geometry.cpp` are the R2 starting
geometry. R2 applies to the source model (mechanism R3):

1. P002 spindle becomes Ø20 h6 × 660 mm with M20×1.5 ends (threads not
   modeled);
2. P001 spool receives the Ø20.5 flange bores, the Ø60×2.5 hub tube and
   3 mm flange thickness (external Ø400 × 160 unchanged);
3. P004/P005 posts become 18 mm S235JR plates (70 mm wide in X) with the
   Ø47 H7 bearing pockets and Ø20.5 spindle bores;
4. new P006 brake disc and P007 caliper solids with intersection checks
   against every other payoff and guard part;
5. the straight 880 mm wire route and both wire-diameter static checks are
   re-run and must pass unchanged.

Until that source change is built and its checks pass, the published
[full-machine STEP](../engineering/generated/mechanism/full-machine.step)
and [manifest](../engineering/generated/mechanism/manifest.csv) describe the
R1 geometry only. `check_mechanism_outputs.py` expectations are updated only
together with the regenerated outputs, never in advance.

## 6. Remaining gates (unchanged or new)

Using the blank
[guard/payoff release template](inspection/guard-payoff-release-template.csv):

1. task-specific risk assessment: cutting, wire release, entanglement,
   ejection, access, maintenance, loading, loss of power, emergency egress;
2. Bernstein SLC/ACF-1 manufacturer mounting documentation
   (**retrieved and archived 2026-09-16** — §2), physical fit
   check and locking-travel verification;
3. full-spool mass, spindle deflection, weld quality, brake adjustment and
   runaway/overspeed measurements;
4. the hardwired guard-contact, stop-time, energy-removal, DC-bus decay and
   E1/E2 release-permission work specified by
   [controls and wiring](22-controls-and-wiring.md);
5. safety-distance verification for the 30 mm apertures or their replacement
   with a compliant entry design;
6. PC door impact verification and door hardware inspection;
7. dimensioned A3 drawing sheets for the P001–P007 and G101–G106 parts and a
   released weld/assembly procedure for the spool, posts, base and door frame.
   (R2 sheets produced 2026-09-15:
   [P002 spindle](../engineering/generated/drawings/P002-R2-payoff-spindle.pdf),
   [P004–P007 plates](../engineering/generated/drawings/P004-P007-R2-payoff-plates.pdf),
   [G101–G106 panels](../engineering/generated/drawings/G101-G106-R2-guard-panels.pdf);
   the weld/assembly procedure and physical inspection remain open.)

Nothing in this record may be read as closing any of those gates.

The R1 accommodation record [docs/25](25-preliminary-guard-and-payoff.md)
remains the authority for the guarded assembly until the R2 source changes in
section 5 are built, checked and published.
