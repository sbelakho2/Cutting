# Feed cartridge — dimension and assembly record

Revision **S1-B-FEED-R6-2026-09-12**. Dimensions are in millimetres unless stated otherwise. R2 corrected assembly paths; R3 integrated MC access holes, the wire lane and belt relief. R4 introduced matched j5 seats and bounded housing clearance. R5 added shoulder/key-seat/groove radii, matching spacer/key relief, M15 thread runout and the blind M5 drilling geometry. **R6 adds explicit end/thread-lead chamfers and blended washer-slot/key-seat intersections**, plus a nominal machined-section stiffness audit. Pressure/tensioning slides and machine supports are still being integrated; this is **not a full-machine fabrication release**.

## 1. Records and coordinates

- [Cartridge STEP](../engineering/generated/feed/feed-cartridge.step), [part manifest](../engineering/generated/feed/manifest.csv), [model validation](../engineering/generated/feed/status.json).
- [Purchased parts](../bom/selected-transport-and-cabinet.csv), [custom quantities](../bom/feed-custom-parts.csv), [calculation record](../engineering/generated/feed-analysis/report.md).
- F101 R6 A3 drawings: [sheet1 — shaft dimensions](../engineering/generated/drawings/F101-R6-shaft.pdf), [sheet2 — machining details](../engineering/generated/drawings/F101-R6-details.pdf). [Machined-section audit](../engineering/generated/feed-sections/report.md) and [191 section records](../engineering/generated/feed-sections/sections.csv).
- Axis **S** is the CAD **+Y** axis. S0 is the rear end of the rotor shaft. X/Z are transverse. The roller centre is at S103.5. A is the common axis of the two bearing seats; B is the shoulder face at S82. Dimensions from S0 are baseline coordinates, not an additive tolerance chain.
- **Wire contact is at S108.75**, not the roller's S103.5 midplane. In the right cassette, use assembly Y = S − 108.75, with nominal nips at X75.5/138.0. At the maximum centre distance of 64.5 mm, the far nip at X140 retains 10 mm of insulated grip on a 200 mm part during the 25 mm leading strip / 25 mm tail backfeed. The motor is on the far shaft.
- Incoming MC access diameter: ≤ 4.2 mm, centred at S103.5 ± 0.15. The assembled wire-lane tolerance of ± 0.20 mm includes axial endplay/alignment. For 4.5 mm wire, the bounded pocket/tread-edge clearances are 0.55/2.10 mm.
- Eight identical shaft/cartridge sets: four lower driven rollers and four upper passive rollers. **Only two motors and two couplings**: one motor drives two lower rollers through one belt per cassette. The exported example shows one motor-driven cartridge, not eight motor kits.

## 2. F101 — rotor shaft

Material: **42CrMo4+QT**, certified minimum yield strength 650 MPa. Rough stock: Ø20 × 120 mm. Turn and finish the bearing seats in a common setup; outsource cylindrical finishing if the available machine cannot demonstrate the required diameters and runout. Do not use a printed shaft.

| S interval | Feature / nominal size | Requirement |
|---|---|---|
| 0–18.5 | Ø12 coupling journal | 12h6; 11.989–12.000; external end C0.20 ± 0.02 at S0 |
| 18.5–19.62 | SEEGER A12 groove | Ø11.45 ± 0.02; width 1.12 ± 0.02; start 18.50 ± 0.02 |
| 19.62–31.5 | Ø12 rear-bearing/spacer journal | Matched 12j5; 11.997–12.005 AND 3–6 µm actual interference with the assigned 6001 bore |
| 31.5–64.5 | Ø14 pulley journal | 14h6; 13.989–14.000 |
| 64.5–71.5 | M15 × 1-6g thread | KM2/MB2 retention; entry C0.50 ± 0.02; full-profile threads over S65.10–71.30; GO/NO-GO gauged |
| 71.5–73 | External thread runout | Ø13.50 ± 0.02; R0.20 ± 0.02 at both roots; no tool overrun into the bearing seat |
| 73–82 | Ø15 locating-bearing journal | Matched 15j5; 14.997–15.005 AND 3–6 µm actual interference with the assigned 6002 bore |
| 82–87 | Ø19 shoulder | 19.00 ± 0.02; B at 82.00 ± 0.02 |
| 87–92 | Ø12 neck | 12.00 −0.02/0 |
| 92–113 | Ø10 roller journal | 10h6; 9.991–10.000; external end C0.20 ± 0.02 at S113 |
| End S113 | M5 × 0.8-6H axial tapped hole | C0.20 ± 0.02 entry; 14.00–14.05 mm full-profile thread after the entry; Ø5 cosmetic thread envelope to S98.8 nominal |
| Same blind hole | Minor drill / point | Ø4.20 +0.03/0; 18.00 ± 0.05 mm cylindrical depth from the end; 118° ± 2° point beyond that depth |

General axial-coordinate tolerance: ± 0.02 mm, unless a tighter feature callout is given. Bearing-seat cylindricity: 0.001 mm; total radial runout: 0.002 mm to common axis A. Other-journal cylindricity: 0.005 mm. Total radial runout of the Ø14 and Ø10 journals to A: ≤ 0.010 mm. Shoulder B perpendicularity: 0.004 mm to A. **Bearing seats: Ra ≤ 0.4 µm**; roller/coupling seats: Ra ≤ 0.8 µm; other turned surfaces: Ra ≤ 3.2 µm. These are machining/inspection requirements, not measured shop capability.

Use the [matched-fit limits](../engineering/generated/bearing-fit/matching-limits.csv) and [256-corner fit record](../engineering/generated/bearing-fit/report.md). Measure each SKF bearing bore, then finish its assigned shaft seat within the intersection of j5 limits and 3–6 µm diametral interference. Do not use j5 limits alone. With combined diameter-measurement uncertainty ≤ 0.5 µm, accept measured interference of 3.5–5.5 µm and apply the corresponding guard band to the j5 limits. Match-mark spares. No Loctite retainer is specified.

The thermal field must demonstrate the explicit temperature limits in the record before machining release; the fit calculation is not a heat-transfer test.

**Pulley key seat:** centre S47.5; length 28.10 ± 0.02; width 5.04 ± 0.01; rounded ends R2.52. On +Z; bottom Z4.00 ± 0.02, 3 mm below the nominal Ø14 surface. Uses a C45 DIN 6885-A 5 × 5 × 28 key. Deburr without rolling a raised lip onto the bearing journal.

**Roller key seat:** centre S103.5; length 18.10 ± 0.02; width 3.04 ± 0.01; rounded ends R1.52. On −Z; bottom Z−3.20 ± 0.02, 1.8 mm below the nominal Ø10 surface. The key is the custom 3 × 4 × 18 F109, not a 3 × 3 stock key.

**Angular datum C:** the radial midplane of the pulley-key opening through axis A, with +Z towards that opening. The roller-key opening is **180° ± 0.10°** from the pulley key. The MB2-slot median plane must lie within **0.025 mm** of C. These requirements make the transverse coordinate orientation explicit rather than leaving the two keys and washer slot independently indexed.

**MB2 tab slot:** width 4.20 ± 0.05, on +Z; floor Z5.50 ± 0.05; **S31.20–73.00**, intersecting the deeper pulley key seat. The start moved 0.20 mm rearward to clear the new shoulder radius. The MB2's 15 mm bore and 13.5 mm inside dimension put its 4 mm wide tang tip at Z6.00. The slot must remain open across the entire 14 mm seat: do not leave a land at S31.5 or 61.5. **Install MB2 before the pulley key.** The continuous tang-sweep test covers S−2…72.

**R6 slot details:** four plan corners **R0.50 ± 0.02**; four vertical intersections with the rounded pulley-key ends **R0.50 ± 0.02**; the complete slot-floor boundary, including the two lips descending into the deeper key pocket, **R0.15 ± 0.02**. The straight key load flanks retain their dimensions. Apply all meeting blends together as defined by the STEP, not as unconnected radius callouts. The slot's S73 end must also finish **0.00–0.04 mm before the actual start of the Ø15 bearing seat**; satisfy this relational limit as well as the absolute coordinate tolerance. No milling into the finished bearing land is permitted. The rear entry is an interrupted cut through the Ø12/Ø14 shoulder, not a full-width semicircular end.

**Machined root details now in CAD:**

- S31.5, S82 and S92 shoulders: **R0.20 ± 0.02**. The S82 bearing abutment stays within SKF's R0.3 maximum. Its axial datum face stays at S82; it is not replaced by a ramp.
- S87 neck shoulder: **R0.50 ± 0.05**, clear of the front housing's Ø29.8 aperture.
- Both A12 groove roots: **R0.04 ± 0.01**. SEEGER permits a loaded-side radius up to 0.1 times ring thickness. The chosen maximum R0.05 is below 0.094 mm for the thinnest 0.94 mm ring; the minimum 1.10 mm groove also retains 1.00 mm between radius tangencies. CAD places the nominal ring at S18.56–19.56, not inside a root fillet.
- Both key-seat floor perimeters: **R0.15 ± 0.02**. Chamfer each key's shaft-facing perimeter **0.20 ± 0.02 × 45°**; do not remove the outside load-bearing flank. F109's shaft-facing face is its +Z face; the pulley key's is its −Z face.
- M15 runout: the Ø13.50 relief above provides a positive tool-exit space before the matched bearing seat.
- R6 external end chamfers: **0.20 ± 0.02 × 45° at S0 and S113**. These are separate from the blind M5 entry chamfer. M15 lead at S64.50: **0.50 ± 0.02 × 45°**; full-profile thread must cover S65.10–71.30, preserving the nominal KM2 engagement S66–71.
- Deburr the remaining slot openings with a controlled **0.05–0.10 mm edge break**; do not round away the dimensional load flanks or alter the bearing-seat lands. This final small deburring operation is a drawing/inspection requirement, not a resolved feature in the section-stress model.

For the M5 blind hole, the selected tap is **Dormer E412M5, material ID7441118**,3D blind-hole application, formC2–3-pitch cutting lead. The [dedicated operation record](14-f101-tapping-operation.md) gives its actual manufacturer dimensions, purchase entry and physical-tip setup. Worst-case cylindrical allowance beyond the specified full thread, entry and **cutting lead** is **1.28 mm**; a projecting male centre is not included in that figure. Require a guarded measured physical-tip-to-full-form distance≤3.18 mm to retain at least0.50 mm cylindrical bottom clearance. Verify actual full-thread depth with an appropriate depth/thread gauge; merely drilling14 mm is insufficient. CAD includes the nominal118° conical point to S93.738, separate from the Ø5 cosmetic thread.

**Machined-section check:** 191 nominal B-rep plane sections, including the actual slot/key blends, chamfers and blind bore, were integrated for area, centroid and the full X/Z centroidal inertia tensor. The smaller principal inertia was not below the reduced beam inertia at any checked station; minimum ratio1.00. This is a sampled bending-stiffness check, not a continuous dimensional-tolerance proof or local stress solution. The beam's reduced key interval is now S33.30–61.70 to include the floor-lip transitions; the end sections include both external chamfers. The updated all-sign displacement screen is **0.02636934 mm**; working/fault3× sensitivity values remain approximately175.371/329.551 MPa.

OpenCASCADE's integration estimate is dimensionless **relative area convergence**, not a millimetre tolerance or a certified error bound on every inertia component. The code requests1e−10, rejects estimates above1e−8 and uses a1e−7 stiffness-comparison tolerance; none is a fabrication tolerance. Independent solid/annular/translated/rotated/keyed-section benchmarks check the calculated moments. `Ixz_mm4` stores the inertia-tensor off-diagonal term, i.e. minus the centroidal product integral.

**Nominal 3D shaft study:** the [four-mesh distributed-support calculation](../engineering/generated/feed-fem/report.md) now includes all radial-load signs, opposing transmission-torque paths, shaft rotation and actual mesh gravity. The finest mesh has 131909 nodes / 603290 tetrahedra. Working/fault angular stress bounds are 76.5876 / 158.193 MPa; the working displacement bound is 0.0205005 mm. Whole-shaft peak/displacement last-refinement checks pass, but **16 regional raw-peak/angular-bound checks remain unmet**. The current maximum is near the rear shoulder/washer-slot intersection; the specified blends and controlled finishing remain necessary. [Equations and limits](15-feed-shaft-stress-model.md); [verification record](verification/feed-shaft-fem.md).

**Remaining local-stress hold:** converge every retained regional peak and resolve key/collar contact, true thread roots, final edge breaks, fastener preload, bearing/housing compliance and the actual bearing thermal field. The nominal shaft calculation does not validate contact stresses or fatigue, and neither sampled section stiffness nor the3× sensitivity replaces those checks.

## 3. F102/F103/F112 — bearing support

F102 material: 6082-T6. Finished blank: 46 mm (X) × 73 mm (S) × 53 mm (Z), extending from X−23…23, S19…92 and Z−34…19. The bottom face at Z−34 is mounting datum C.

| Feature | Coordinates / callout |
|---|---|
| Rear floating-bearing bore | Ø28.008–28.018, S19–31.5; CAD 28.013 |
| Front module mounting face | Z−18.50 ± 0.01, S72.98–92; upper block removed from F102 |
| Central clearance passage | Ø21, S19–92 before pocket operations |
| Pulley/service bay | X−23…23, S31.5–92, Z−18.5…19; front module removed for insertion |
| Belt-backing relief | Through both X edges, S34.50…60.50, Z−20.00…19; 14 mm remaining bottom bridge; 25 ± 0.6 mm belt width |
| Module screw holes | 2 × Ø4.50 +0.10/0 through the bridge; X±16, S84; Ø9.00 +0.10/0 counterbores, 5.00 ± 0.02 deep from Z−34 |
| Module locating holes | 2 × Ø3H7 through the bridge; X±10, S89; match-ream with F112 |
| Bottom mount attachment | 4 × M5 × 0.8-6H, 12 mm deep; X±16, S25/**65**, from Z−34; old S85 position withdrawn |

F112: 6082-T6, 46 mm (X) × 19.02 mm (S) × 37.5 mm (Z); X−23…23, S72.98…92, Z−18.5…19. Front locating bore: **Ø32.008–32.019**, S72.98–82; CAD 32.0135. Front aperture: Ø29.80 ± 0.05, S82–92. Four M3 × 0.5-6H cover threads, 6 mm deep from S72.98 at X±19, Z±12.5. Two M4 × 0.7-6H threads, 10 mm deep from the joint face at Z−18.5, at X±16, S84.

Two Ø3.010–3.014 dowel bores pass **through the full 37.5 mm height** at X±10, S89. These through-holes allow access for an Ø2.8 service punch from +Z; blind holes would trap the locating pins. Match-mark each F102/F112 pair; finish the bearing bores with the pair located and clamped. Do not exchange blocks between cartridges.

Module hardware per cartridge: 2 × ISO 4762 M4 × 20 screws, class 10.9; 2 × ISO 7092 M4 washers, 4.3 × 8 × 0.5 mm (200 HV); 2 × ISO 8734 Ø3m6 × 20 hardened dowels. Screw head: Z−33.5…−29.5; washer: −29.5…−29; tip: −9.5, giving 9 mm nominal thread engagement. Pins: Z−32.5…−12.5, recessed 1.5 mm from the base underside, with 6 mm engagement into F112. Fit dowels **after axial module insertion**. The joint preload/torque and local stress study are separate from these verified geometric interfaces.

Bore-axis coaxiality: 0.010 mm; axis parallelism to C: 0.010 mm over 73 mm; axis height: 34.00 ± 0.02 mm above C. Seat Ra ≤ 0.8 µm. Front shoulder at S82: perpendicularity 0.010 mm to the bore axis. With the cover installed, the actual front bearing pocket depth is 9.02–9.05 mm; record the actual width of the 9 mm bearing. Resulting outer-ring end clearance: 0.020–0.110 mm. Do not use the cover to squeeze both bearing rings together.

F103: S355 plate, 46 × 37.25 × 2.00 mm, from X−23…23, Z−18.25…19; Ø29.80 ± 0.05 central aperture; four Ø3.3 holes at X±19, Z±12.5. The 0.25 mm clearance above the housing bay floor avoids the interference detected in the first model. Keep both faces flat within 0.03 mm. Four ISO 4762 M3 × 8 screws, class 8.8, are included in CAD; the tool-access and housing stress checks remain separate.

## 4. Spacers, roller key and secondary roller operation

| Part | OD (mm) | ID (mm) | Thickness (mm) | Material |
|---|---:|---:|---:|---|
| F104 | 16.00 ± 0.02 | 12.10 +0.03/0 | 2.70 ± 0.02 | C45 |
| F105 | 16.00 ± 0.02 | 12.10 +0.03/0 | 1.00 ± 0.02 | C45 |
| F106 | 19.50 ± 0.02 | 15.10 +0.03/0 | 1.00 ± 0.02 | C45 |
| F107 | 17.50 ± 0.02 | 10.10 +0.03/0 | 1.50 ± 0.02 | C45 |
| F108 | 17.50 ± 0.02 | 5.50 +0.05/0 | 3.00 ± 0.02 | C45 |

Spacer face parallelism: 0.01 mm; Ra ≤ 1.6 µm. F104/F105 contact only the 12 mm bearing's inner ring; F106 contacts only the 15 mm bearing's inner ring. The 17.5 mm roller plates contact the 18 mm aluminium collar, not the polyurethane tread.

F105: chamfer the bore on its **S31.5 shaft-shoulder face**, 0.30 ± 0.02 × 45°. F107: the same bore chamfer on its **S92 shaft-shoulder face**. These clear the R0.20 shoulder roots without changing the axial stack thickness. Do not reverse the spacers or chamfer their bearing/roller contact faces instead.

F109: material C45+N, minimum yield strength 300 MPa; width 2.975–3.000; height 3.950–4.000; length 17.90–18.00; rounded ends R1.5 nominal. Shaft-facing perimeter chamfer: 0.20 ± 0.02 × 45°. Apply the same finishing operation to the selected DIN 6885-A pulley key's shaft-facing perimeter, preserving its specified width and height.

F110: start with exactly **UMCL40-10-20-MC**. Catalogue collar material is **EN AW-5052 equivalent**, with a Shore A 50 urethane lining. Incoming bore acceptance: **10.000–10.030 mm**. This covers the current product drawing's H7 callout and the archived EU catalogue's +0.01/+0.03 callout; do not assume the narrower band without measuring. The selected 10h6 shaft gives at most 0.039 mm diametral clearance. Torque is transmitted by the positive key, not a press fit.

Protect the polyurethane from jaws, chips and cutting fluid. Fixture on the aluminium collar. Machine one axial through-keyway, width 3.04 ± 0.01, radial depth 2.35 ± 0.025 from the **actual bore**, opposite the two MC holes. Preserve the bore and both collar faces. Do not enlarge the bore or cut the polyurethane. The original collar is 18 mm OD; the remaining collar ligament and root require the local-stress check identified in the analysis record.

Roller retaining screw: ISO 4762 M5 × 16, class 10.9, with F108. Nominal tip at S100.5, giving 12.5 mm engagement in the shaft. No washer is inserted under this screw beyond F108. Its final preload/torque requirement is part of the outstanding collar/joint check; do not use the M3 set screws as the torque connection.

## 5. F111 — motor plate and coupling

6082-T6, 80 × 80 × 10 mm. Motor-side face at S−35.2. Four Ø4.5 through-holes on a 50 × 50 mm square. Pilot bore: Ø38.18 +0.02/0 through; front relief: Ø42.50 +0.05/0, starting 1.95 ± 0.02 mm from the motor face. This passes the drawing's Ø38.1 ± 0.05 pilot and the 40 mm coupling envelope. The external supports and mounting fasteners for this plate are not yet integrated.

Use **MK2/100/50/10/12**, not a 10/10 coupling. **Retain the motor's D-flat; there is no key to remove.** Nominal coupling: S−33…17; motor hub: −33…−17; shaft hub: 1…17. Motor face: −35.2, giving a nominal shaft tip at −14.2 and a 14.2 mm shaft-end gap.

Set the actual coupling at its free length; do not stretch the bellows. Mark both journals to verify complete 16 mm hub insertion. Both clamp screws: 4.5 N·m per R+W. Verify ≤ 0.05 mm radial and ≤ 0.1° angular alignment before tightening the supports.

## 6. Assembly order

1. Clean the parts. Measure all bores, journals, spacers and purchased bearing widths. Record material certificates. Reject raised key-seat burrs or damaged bearing shields.

2. **Keep F112 off F102; remove its dowels.** With the shaft outside the base, slide F112 onto the shaft from −S, passing its 29.8 mm aperture over the 19 mm shoulder to its final position. Insert 6002 from −S into its 32 mm bore and onto the 15 mm seat. Use separate inner- and outer-ring support sleeves; do not drive the bearing into two fits through one ring.

3. Slide F103 from −S over the bare shaft and seat it on F112. Install its 4 M3 × 8 screws and verify outer-ring clearance. Install F106, then one new MB2 and KM2 from −S. Align the MB2 tang with the long slot. **No pulley key may be present during this step.** Seat the inner-ring retention stack fully without using it to preload both bearings.

4. Install the 5 × 5 × 28 mm pulley key and N14 pulley from −S while the rotor is still outside F102. Set the pulley's rear face at S31.5. For upper passive cartridges, omit the pulley and key. Keep the near-flange envelope free of burrs.

5. Insert the entire preassembled shaft/pulley/cover/front-module stack **axially from +S** over F102's bridge until F112 seats. **No dowel may project above the bridge during insertion.** The continuous swept envelopes cover 100 mm of approach travel; the old one-piece housing fails this check. Align the matched block, joint face and holes. Install 2 M4 × 20 screws with washers, then 2 dowels from the underside. Final torques require the joint preload record.

6. From −S, fit F105 with its bore chamfer towards the S31.5 shaft shoulder, then the rear 6001, F104 and SEEGER A12. Use ZGA-1 pliers and verify complete groove seating between the small root radii. Rear inner-ring endfloat must be **0.08–0.50 mm**. Do not remove it with an arbitrary shim.

7. Fit F107 with its bore chamfer towards the S92 shaft shoulder, then F109 and keyed F110 from +S. Put the key's chamfered face towards the shaft. Align the key before sliding the aluminium collar over it; do not press through the polyurethane. Fit F108/M5 × 16 only after the joint torque setting is released. The shaft ends 0.5 mm short of the nominal roller face so the plate clamps the collar.

8. Motor-driven role: fit F111 and the coupling at its free length. Belt-follower role: fit a pulley/key, but no duplicate motor/coupling. Passive role: fit neither pulley nor motor. Rotate by hand after each retention step; never energise unsupported cartridges.

9. For service, remove the cartridge from its machine mount to access the underside screws. Remove the module screws, then drive both dowels downward into a supported clear space with a suitable punch before withdrawing F112/rotor towards +S. Do not lever between precision joint faces. Replace MB2 after disassembly.

10. Set the equal-pulley centres nominally to 62.5 mm. The slide must cover **52.5–64.5 mm centre distance**, a **12 mm adjustment stroke**. At an actual span of 62.5 mm, apply 1.000 mm deflection: accepted force **13.8715–15.1215 N**, target 14.4965 N. Use the actual span in the manufacturer's formula. Lock the slide and remeasure. Do not use the 221 N maximum initial tension as the target.

## 7. Inspection and bench verification

- Use the [blank F101 R6 inspection record](inspection/F101-R6.csv) with the two A3 sheets and the assigned-bearing fit record. No measured value or pass result is prefilled. Dimensional acceptance alone does not clear the separate engineering release holds.
- Record shaft diameters at two angular positions and three axial locations per seat; record B-face and roller-journal runout to A. Record bearing-bore coaxiality and height.
- Check actual rear-stack endfloat, front outer-ring end clearance, and collar/endplate clamping. Verify free rotation through 360° after each retention step.
- Belt static proof uses a rigid **non-motor** fixture and a calibrated torque transducer: 5 N·m in each direction, a 10 s hold and 10 cycles. Mark the shaft/key/pulley/hub interfaces to detect relative slip; inspect the belt teeth and collar afterwards. Do not use motor holding torque as the torque reference or use pliers on bearing journals.
- Measure running torque, loaded rolling radius and wire tension using actual wire lots before enforcing the 1 N·m operating envelope. Stop on visible indentation, slip, damage or encoder disagreement. No precision claim is inferred from the unloaded roller diameter.
- Record results; do not mark a physical test as passed because its procedure or calculation exists.

## 8. Specific unfinished release items

- Remaining shaft regional stress convergence, plus housing and collar-ligament/contact stress studies.
- Resolved stress at the defined R6 shaft-end/slot-intersection details and final deburred edges.
- Front nut and roller screw joint/preload settings.
- Remaining fasteners and tool access.
- Belt wrap/rotation and the tensioning slide.
- Upper pressure mechanism.
- Wire guides.
- Complete frame integration.

These items are not replaced by the passing static cartridge intersections or the individually scoped calculation checks.
