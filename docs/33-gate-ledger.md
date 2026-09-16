# 33 — Gate ledger: every open gate and hold in the repository

**Revision:** `S1-B-LEDGER-R1-2026-09-16`
**Status:** single source of truth for "what remains". One table below lists
every open gate/hold found across the repository (docs/24 §9+§10, docs/22
§10, docs/26 §6, docs/28 §4+§7, docs/20, docs/12, docs/15–18, docs/30,
README.md warnings, engineering/README.md "not yet qualified" mentions, and
the verdict/status files under `engineering/generated/`). Categories:

- **CLOSED** — fully closed with recorded evidence (none of the open gates
  qualify; the table contains no CLOSED rows).
- **DESK-CLOSABLE** — closable by calculation/document work at a desk. Gates
  whose desk part is now closed say so and give the study/record; gates
  being handled by a parallel workstream are named.
- **PHYSICAL-ONLY** — requires hardware, measurement or competent-person
  sign-off; the exact procedure/template is cited.
- **EXTERNAL-INQUIRY** — requires a manufacturer/distributor answer; the
  exact enquiry text/row is cited.

Nothing in this ledger releases fabrication or energisation. Desk closures
recorded here (§2) are calculations with stated assumptions; the physical
steps attached to each gate remain mandatory.

## 1. Open gates

| # | Gate | Where | Category | Current evidence | Exact closure action |
|---|---|---|---|---|---|
| 1 | Housing/collar key contact | docs/24 §3/§9/§10.6; docs/12:68,158; feed-fem "key/collar contact" holds | DESK-CLOSABLE — desk part **CLOSED 2026-09-16** | New [collar-contact study](../engineering/generated/collar-contact/report.md): 5 Nm resolved-pressure linear FEM, ≤35 k nodes, both key identities PASS the C45 allowables (bearing 120, shear 180 MPa). F109 3×4×18: contact 26.2 MPa FEM (25.25 ideal), shear 18.5, seat bearing 30.9, key peak vm 164 MPa < 300 MPa F109 min yield, aluminium collar-wall margin 1.66× (limit 65/1.5 = 43.3 MPa). DIN 3×3×20 passes C45 allowables but erodes the aluminium margin to ≈1.0× — not an endorsed F109 substitute. Clamp: M4 8.8 @2.8 Nm, K=0.2 → 3.5 kN/screw, 7 kN total (69% of 5.1 kN proof) | Remaining physical part: incoming bore 10.000–10.030 mm and keyway 3.04±0.01/2.35±0.025 measurement on F110 (inspection/incoming-inspection-template.csv), clamp-screw identity + torque record at assembly, belt proof test 5 Nm/10 s/10 cycles slip-marked (docs/12:152) |
| 2 | Adapter FEM raw-peak REFINE / seat contact model | docs/24 §9/§10.6; adapter-r6-analysis; engineering/sources/adapter-contact-findings.md | DESK-CLOSABLE + PHYSICAL-ONLY (adapter workstream completed 2026-09-16) | Resolved-pressure model (C¹ quartic seat profile, peak 376.33 MPa, mean 200.71 MPa per 8.0 kN seat; resolved Winkler foundation) removes the patch-edge singularity: pair peak 285.99 MPa sits 0.59 mm inside the washer-OD edge; gate pair (1.0→0.7 mm) still 18.46% raw peak / 4.96% displacement → rows stay REFINE (never relaxed); seat-scale pair (0.15→0.1 mm, 64,944 nodes) refines 321.67→312.71 MPa, 2.86% ≤ 5%, bounding the model peak at ≈313 MPa (below 650 MPa yield, above the 216.7 MPa yield/3 screen) | Remaining gap is physical: measured washer/joint contact stiffness or a two-body ISO7092 washer contact solve (docs/24 §9, adapter-contact-findings.md); keep the unchanged gates and the 150 k-node cap |
| 3 | 34-exterior same-surface centroid whole-shaft audit | docs/26:22; docs/28 §7; README warnings; engineering/sources/centroid-closure-findings.md | DESK-CLOSABLE — desk part **CLOSED 2026-09-16** (exterior-centroid workstream); residual containment audit **EXECUTED 2026-09-16** | Config sweep + final rerun: `no_exterior_same_surface_centroids` 0 → PASS_SCOPED and `all_reported_peak_centroids_in_CAD` 0 → PASS_SCOPED on the gmsh torus series (≤150 k nodes; loads/phases/gates unchanged; Netgen series reproduced byte-identically). The residual whole-shaft continuous CAD-containment audit was then run exactly ([findings](../engineering/sources/containment-audit-findings.md), [counts](../engineering/generated/feed-fem-torus/gmsh/containment-audit.csv)): BRep point-in-solid classification of all 1,125,721 element centroids of the four levels (2/0.3, 1.6/0.2, 1.2/0.13, 1/0.09 mm) at 1×10⁻⁷ mm finds **32 exterior / 0 on-boundary** (0/24/3/5 per level), all facet slivers on the concave fillet tori (faces 68, 7, 9, 61, 63, 71, 72), gaps 0.0071–5.51 µm | Residual: `continuous_CAD_containment_verified` remains **false** (32 exterior centroids of 1,125,721); not a release blocker; keep the four-mesh raw values and every gate unaltered |
| 4 | Fastener preload/torque record (M5/M10 joints) | docs/24 §10.5; docs/30:97; docs/12:115 | DESK-CLOSABLE — desk part **CLOSED 2026-09-16** (ledger §2.2) | Torque values "not published" for M5/M10; H003 joint preload record pending | Desk calc recorded §2.2; physical: record actual torque values in the production build record (inspection/production-build-record-template.csv) at assembly |
| 5 | Roller-key identity slip (3×3×20 vs F109 3×4×18) | docs/28 §2.3 ("UMCL roller key … (3×3)"); docs/12:109 (F109 3×4×18); catalogue-evidence:142 | DESK-CLOSABLE — desk part **CLOSED 2026-09-16** (ledger §2.3) | F109 is 3×4×18 C45+N (width 2.975–3.000, height 3.950–4.000, length 17.90–18.00); the audit's "3×3" shear line is a transcription slip | Both identities evaluated in the collar-contact study (gate 1); keep F109 as the governing selection; no doc edit made outside this ledger |
| 6 | Kübler wheel M4×6 set-screw torque | docs/30:182 | DESK-CLOSABLE — desk part **CLOSED 2026-09-16** (ledger §2.4) | Register row "set screw torque pending" | Desk value 2.0 Nm recorded §2.4; physical: record applied torque at assembly in the build record |
| 7 | Guard/payoff weld & assembly procedure | docs/26 §6.7; docs/34 | CLOSED-DESK (physical inspection remains) | Procedures drafted with fixtures, processes and criteria: post-to-base coaxial weld (gauge-pin fixture, coaxiality ≤0.1), spool hub weld (runout ≤1.0), jamb/door-frame weld + glazing (docs/34) | Physical weld inspection and first-article checks (docs/26 §6.3, docs/20) |
| 8 | True helical-thread roots (M5 blind hole, M15) | feed-fem holds "true_threads_verified:false"; docs/15:124; docs/12:60 | PHYSICAL-ONLY | FEM uses nominal thread envelopes; not qualified | Machining QA per docs/12 (M5 depth/tip record docs/14, thread gauge) plus thread inspection at receipt; no FEM number releases the thread |
| 9 | Final edge breaks 0.05–0.10 mm | feed-fem; docs/12 | PHYSICAL-ONLY | Deburr practice specified; not measured | Deburr without raised lips and inspect per docs/12:127 + build record |
| 10 | Bearing/housing compliance, preload, thermal field | docs/24 §9 "Bearings, joints and thermal behavior"; bearing-fit (`thermal_field_verified:false`); docs/12:68 | PHYSICAL-ONLY — desk parts (housing compliance + thermal field) **CLOSED 2026-09-16** (ledger §2.5) | 256-corner fit envelope passes conditionally (min 1.28 µm); new [bearing-contact-thermal study](../engineering/generated/bearing-contact-thermal/report.md): the 6 µm matched interference resolves to 17.65 MPa on the seat (seat vm 22.73 vs 650 MPa, ring 82.26 vs ≈1700 MPa); the record's F112 bore is a clearance fit, so the housing carries no fit pressure (bore-wall 0.98 MPa under the resolved 310 N); housing compliance changes the seat stress ≤3.9 MPa absolute (a rigid-boundary artifact) — not material; the 30 W standstill thermal FEM gives a 29.66 K front-seat rise vs the record's 20 K shaft-above-housing envelope → **not met at the central stated seat conductance** (sensitivity 15.5–51.7 K) | Remaining physical: measure/justify preload, contact and the seat temperature field on the actual build; instrumented cold-start/hot-run test (docs/12:38); record the motor-case temperature at rated hold (docs/28 §2.5) |
| 11 | Rotating fatigue qualification | feed-fem (`fatigue_verified:false`); docs/15:124; docs/24 §9 | PHYSICAL-ONLY | No fatigue life established | Physical fatigue test on the actual shaft/build (docs/15 requirement); nothing desk-side substitutes |
| 12 | ±60 N roller-force envelope / pressure, friction, jam | docs/verification/feed-shaft-fem.md:116; docs/24 §9 | PHYSICAL-ONLY | 5 Nm at r=20 mm could mean 250 N tangential — torque alone does not prove the force limit | Physical pressure/friction/jam qualification on the assembled cassette + docs/20 nip/wheel force records |
| 13 | Blade pair T001 | docs/24 §4.4; head (`tooling_qualified:false`) | EXTERNAL-INQUIRY + PHYSICAL-ONLY | Purchase-to-drawing; edge-unqualified; no blade pair procured | Specialist manufacture quote to the drawing; loaded clearance verification before installation; edge qualification (docs/24 §4.4) |
| 14 | Guard risk assessment & 30 mm aperture sign-off | docs/26 §6.1/6.5; README warnings; mechanism (`guard_safety_function_validated:false`) | PHYSICAL-ONLY | Desk reach calc passes 2.9× (EN ISO 13857: 345 mm > 120 mm, docs/28 §2.2); requires sign-off | Task-specific risk assessment (cutting, wire release, entanglement, ejection, access, maintenance, loading, power loss, egress) on inspection/guard-payoff-release-template.csv + aperture verification against the current standard edition |
| 15 | Bernstein SLC/ACF mounting + G106 drilling pattern | docs/26 §6.2 + G106 row | EXTERNAL-INQUIRY — desk part **CLOSED 2026-09-16** | Manufacturer documents retrieved and archived: Bernstein installation/operating manual 0800000865 (2022-07-12, `engineering/sources/bernstein-slc-installation-manual-0800000865.pdf`) + datasheet 6018200007 (`engineering/sources/bernstein-slc-f-024-20-20-r4-datasheet.pdf`). Geometry recorded in docs/26 §2/§6: SLC-F 30×192×44 mm, 4×M5 (Ø5.4; rows 161.6 mm / columns 22 mm), 3×M20×1.5, flat surface, any orientation, alignment X/Y 29+3, 2 mm overtravel (stop 0.2–2 mm), ACF-1 R ≥400 mm, one-way screws M5×25/40 available | Physical fit and locking-travel verification remains (docs/26 §6); transfer the exact hole coordinates from the archived drawing p.8 to the G106 fabrication sheet before drilling |
| 16 | Payoff physical validation (spool mass, spindle deflection, welds, brake, runaway) | docs/26 §6.3; mechanism (`payoff_selection_complete/rotation_validated:false`) | PHYSICAL-ONLY | Desk spindle bending bounds: 30 kg → 0.65 mm/52 MPa; 61 kg → 1.33 mm/107 MPa (docs/28 §2.1) | Weigh full spool (≤30 kg until re-checked), measure spindle deflection, weld inspection, brake adjustment, runaway/overspeed measurement (docs/26 §6.3) |
| 17 | PC door impact + door hardware | docs/26 §6.6 | PHYSICAL-ONLY | 5 mm PC front door selected; no impact evidence | Impact verification + door hardware inspection per docs/26 §6.6 |
| 18 | IF-B01 review, fabrication, fixture | docs/22 §10.1; docs/24 §5 | PHYSICAL-ONLY | ERC/DRC/parity clean (draft); no human review or produced board | Electrical/mechanical review, fabricator panel/stack-up confirmation, bare-board build, `if-b01-factory-test.uf2` fixture run with J4–J7 disconnected (hardware/if-b01/if-b01-test.md); archive fixture logs |
| 19 | LC1D12BD DC switching duty | docs/22 §10.2/§11.1; docs/24 §10.4; README | EXTERNAL-INQUIRY — desk part **CLOSED 2026-09-16** | TeSys Deca catalogue MKTED210011EN (2026) retrieved from download.schneider-electric.com and archived (`schneider-tesys-deca-catalogue-2026.pdf`, extract `schneider-tesys-dc-tables-fuse-carriers.pdf`). A5/120–121: 3 poles in series, DC-1 and DC-2..5 rated operational current **20 A at ≤75 V** → 36 V/9.7 A covered 2.06×, 60 V/5.8 A covered 3.45×; load L/R ≈0.1–0.3 µs ≪ 1 ms; verdict in docs/22 §11.1; CT-K1-K3/CB-K1-K3 → `selected_DC_duty_qualified` | Measured K1/K2/K3 opening behavior on hardware (docs/22 §8.5) and the catalogue critical-current durability note (5.8/9.7 A below the 20 A table value) remain physical checks; the §11.1 alternatives stay a contingency record only |
| 20 | DC branch fuses/holders/coordination/inrush | docs/22 §10.3/§11.6 | EXTERNAL-INQUIRY (identities — electrical-identities workstream) | Wire ampacity/regen/discharge desk parts done; XPS fuse corrected to 2 A and moved to the priced 5×20 solution (docs/22 §11.3); drive-logic feeds on 3046032 + 1 A links; the 10×38 holder identity is now catalogue-confirmed (DF101, gate 38) but superseded by the 5×20 solution | Confirm the branch fuse identities on live distributor pages at purchase (exact proposed identities in docs/22 §11.6); measured decay + prospective-fault coordination at commissioning |
| 21 | AC incoming/protection, enclosure, terminals, glands, PE, local regulation | docs/22 §10.4/§11.5/§11.6; docs/24 §6 | EXTERNAL-INQUIRY + PHYSICAL-ONLY | Thermal budget passes on paper (≈152 W vs 280 W L35, docs/22 §11.5); identities proposed (§11.6); PE/AC design not released | Competent-person electrical review under local regulations; confirm enclosure/terminal/gland/PE identities at purchase; physical PE continuity/insulation tests (docs/23) |
| 22 | E1/E2 guard-release-permission design | docs/22 §11.8 | CLOSED-DESK (physical verification remains) | R2 + K1–K3 NC chain + T2 on-delay design specified with fail-safe direction (docs/22 §11.8); original design-input list satisfied at design level | Physical: measured bus decay, timer setting, risk-assessment sign-off, fault-injection validation (docs/22 §10 hold 5/6) |
| 23 | Safety-function validation (fault injection, stop time, bus decay) | docs/22 §10.6/§8.4–8.5; docs/20 | PHYSICAL-ONLY | Only the hardwired XPS/contactor path is the safety function; unvalidated | Reviewed test plan: each E-stop channel, both guard paths, cross-wire faults, reset/EDM, each contactor fault mode; measured stop time, DC-bus decay, regenerated-energy behavior (docs/22 §8) |
| 24 | Commissioning of IF-B01, USB, encoders, field I/O, contactors, drivers | docs/22 §10.7; docs/24 §7 | PHYSICAL-ONLY | No hardware commissioning exists | Staged energisation under competent oversight per docs/24 §7: voltages, polarity, field contacts, encoder direction, drive enable, interlocks, fault response, restart prevention; record instruments |
| 25 | Physical machine qualification (length cells, forces, Cpk) | docs/22 §10.8; docs/20; qualification (`qualification_passed` false everywhere) | PHYSICAL-ONLY | Procedure + fail-closed tool exist; all templates blank; synthetic fixtures are not results | Execute docs/20: ≥30 pieces per mandatory cell (OD 1.2/4.5 mm × 200/1000 mm), Cpk ≥ 1.33, nip 50–75 N, wheel 6–19 N, environment 15–30 °C, gauge U ≤ 0.05 mm / 1.0 N; retain raw CSVs; `physical_results` only for non-SYN identities |
| 26 | First article + energisation gate | docs/24 §8.1 | PHYSICAL-ONLY | Template blank | Complete inspection/first-article-and-energisation-gate-template.csv with actual evidence and reviewer disposition |
| 27 | Fabrication/incoming/build-record evidence | docs/24 §1–§3 | PHYSICAL-ONLY | All templates blank; source checks ≠ physical results | Execute inspection/production-build-record-template.csv and inspection/incoming-inspection-template.csv per work package; run tools/check_production_records.py etc. |
| 28 | Measuring module physical support/torque + fabrication | engineering/README measuring paragraph; docs/13 | PHYSICAL-ONLY | M201 source and interfaces exist; "physical support/torque, fabrication and qualification evidence remain open" | Fabricate and measure per docs/13 + mechanism manifest; wheel contact force 6–19 N record (docs/20) |
| 29 | Measurement calibration: factory accuracy (1.65 mm budget) + 626 CN thermal screen | measurement-analysis (`unresolved_source_checks:2`, `physical_calibration_qualified:false`); README:60; engineering/README:93–94 | PHYSICAL-ONLY | 392 synthetic observations, 0 residual failures (numerical only); 626 CN leaves 0.5 µm with 10 K ΔT — not accepted with full js5 interference | Traceable calibration of the 100±0.1 mm circumference claim + actual index reference; separate 626 thermal/matching design or substitution; `--require-factory-accuracy` fails until then |
| 30 | R+W coupling interior clearance verification | docs/verification/feed-r6.md:46 | EXTERNAL-INQUIRY | "Original coupling interior clearance has not been verified from supplier CAD". 2026-09-16 re-fetch of the June-2026 MK2 datasheet (byte-identical to rw-mk2-datasheet-2026.pdf, SHA256 f6c663d7…; see mechanical-documents-findings.md §2) still does not dimension the bellows interior | Obtain supplier CAD/data for MK2/100/50/8/14 and verify interior clearance before hub installation (R+W enquiry alongside the quotes in docs/24 §2.2) |
| 31 | HIWIN KK8610P340A1F0 quote (article 10.00026, ×2) | docs/24 §2.2 | EXTERNAL-INQUIRY | 31-channel search found no public price; unverified leads: eBay item 406408632737 US $450 (403-gated), MISUMI VN/TH sibling. 2026-09-16: current HIWIN catalogue PS-11-1-EN-2605-K (2026-05-22) archived as hiwin-kk86-catalogue-2026.pdf — confirms 340/440, lead 10, P class, and the **input shaft Ø8 h7 / 18 mm exposed** (mechanical-documents-findings.md §1); price still unobserved | Request from two independent HIWIN distributors; compare against the eBay/MISUMI leads from a browser; keep the axis unsubstituted unless a change record approves |
| 32 | R+W MK2/100/50 quotes (10/12 ×2, 8/14 ×2) | docs/24 §2.2; engineering/sources/sourcing-findings-rw.md | EXTERNAL-INQUIRY | Exact bores have no live price (23 channels); live class anchor MK2/100/50/10/14 EUR 99.43 net/pc (ebmia.pl); OD 40 × L 50 × hub 16 dims verified. 2026-09-16 re-verified from the June-2026 datasheet (byte-identical to rw-mk2-datasheet-2026.pdf): size 100 = **10 Nm** TKN > 8.5 Nm motor holding torque, OD 40, A-1 50/60 (selected 50), fit length C 16, bores 5–24 H7 (8/14 and 10/12 both in range), M4 @ 4.5 Nm | Any R+W dealer can order the exact bores at the class price; confirm OD 40 × L 50 × hub 16; no substitute without a change record |
| 33 | Kübler measuring wheel 8.0000.3123.0006 ×2 quote | docs/24 §2.2 | EXTERNAL-INQUIRY | No public price; 100±0.1 mm circumference claim unverified until receipt. 2026-09-16 re-verified from the live kuebler.com `/pdf?Messrad_en.pdf` (byte-identical to kuebler-messrad-en.pdf): wheel No.123 natural rubber 60 Shore A, **Ø31.83 ±0.025**, 100 ±0.1 mm circumference, 6 mm (H7) bore, 9.5 tread / 14.5 overall, M4×6 ISO4029; not a stock type. Supplementary kuebler-messradsysteme-2025.pdf archived | Compare distributor offers against keeping the archived wheel; verify the circumference claim at incoming inspection |
| 34 | Autonics E40S6-1024-6-L-5 ×2 quote | docs/24 §2.2 | EXTERNAL-INQUIRY | No public price observed. 2026-09-16: live product page + current EN product manual TCD210019AD 2025-07-21 (archived autonics-e40s-datasheet-2025.pdf) confirm Ø6 shaft (−0.004/−0.016), 1024 P/R, line driver A,Ā,B,B̄,Z,Z̄, 5 VDC ±5%, 3-M3 DP5 on PCD30, pilot Ø20 −0.021/0, body Ø40×31, shaft 15 + pilot 5; M201 plate check unchanged; FLAG: shaft-tolerance and pilot-tolerance deltas vs shared/measurement_spec.hpp (see mechanical-documents-findings.md §4; spec.hpp not edited) | Request from an Autonics distributor |
| 35 | Rittal SK3303.500 ×1 quote | docs/24 §2.2 | EXTERNAL-INQUIRY | Conrad listing observed EUR 1,669 incl VAT (merchandise only) | Request from a Rittal distributor; confirm the 230 V 50 Hz variant |
| 36 | STEPPERONLINE CL57T-V41 ×2 / LE-350-36 ×1 | docs/24 §2.2 | EXTERNAL-INQUIRY | Exact strings have no live price (shop 403); class anchors: EUR 173.78 (amazon.de B0FYPDGC85), EUR 77.04 (B0HDN818SJ) | Record the StepperOnline shop price at purchase; class fallbacks documented |
| 37 | SEEGER-ORBIS A12 ×8 | docs/24 §2.2 | EXTERNAL-INQUIRY | Brand-exact has no live price; DIN 471 12×1 anchors: 100 pcs EUR 8.57 (Wegertseder 1950-010) | Any DIN 471 12×1 federstahl ring meets the groove; brand substitution needs a change record |
| 38 | Schneider DF2CN02 10×38 fuse holder + prospective-fault coordination | docs/22 §11.3/§11.6; docs/24 §2.2 | EXTERNAL-INQUIRY — desk part **CLOSED 2026-09-16** | DF2CN02 observed EUR 55.99/10 (Conrad, 2026-09-16); holder identity CONFIRMED from the TeSys 2026 catalogue (B4/4, B4/10–11, archived schneider-tesys-deca-catalogue-2026.pdf): **DF101** — 1P DIN-rail 10×38 fuse carrier, Ith 32 A, Ui 690 V, IP20, 17.5 mm, padlockable (family DF101N/DF102/DF103, LED DF101V…DF103NV); price not_observed (no shop listing reached 2026-09-16, acceptable); recorded as the confirmed catalogue identity (CB-FUSE-HOLDER row, superseded) | Design keeps the priced 5×20 CB-XPS-FUSE solution (docs/22 §11.3); DF101 not reintroduced; verify prospective short-circuit coordination at commissioning |
| 39 | CT-PS36 6EP3332-7SB00-0AX0 + CT-IF-U1/U2-U3 identities | docs/24 §2.2 | EXTERNAL-INQUIRY | Identities per selected-controls.csv; no live-page price | Record the distributor page at order time; confirm USB-isolator/interface availability before the board order |
| 40 | Tunisia sous-douane landed-cost RFQ | docs/24 §2.2 (2026-09-16 step) | EXTERNAL-INQUIRY | No landed-cost comparison exists (merchandise observations only) | Send the complete quotation-register SKU list as one RFQ to the sous-douane freight forwarder and two distributors shipping to Tunisia; require unit USD price, currency/payment terms, EXW/FOB, consolidation freight, handling, air/sea freight, destination charges, customs (DDU sous-douane), lead time, validity; record offers before ordering |
| 41 | EMC assessment | docs/22 §7/§8; docs/24 §6 | EXTERNAL-INQUIRY + PHYSICAL-ONLY | No EMC assessment exists | Enclosure/standards review of segregation, shield terminations and ducts, then an EMC assessment on the built cabinet by competent persons |
| 42 | Guard/payoff physical validation flags | mechanism status.json (`guard_door_kinematics_validated:false`, `guard_safety_function_validated:false`, `payoff_selection_complete:false`, `payoff_rotation_validated:false`) | PHYSICAL-ONLY | R2 CAD/checks pass; flags remain false | Physical build and validation per docs/26 §6 (rows 14–17 above) |
| 43 | Full-machine integration flags | mechanism/layout/head/feed-cassette status.json (`full_machine_integrated:false`, `fabrication_released:false`, `physical_results:false`) | PHYSICAL-ONLY | 507-occurrence R3 source exists; cabinet/safety not modelled | Assemble per docs/30/docs/31 and record; no flag flips without physical evidence |
| 44 | Feed-shaft regional stress release | feed-fem/feed-fem-torus status (`numerical_gates_passed:false`); docs/24 §9 | DESK-CLOSABLE — desk part closed by docs/26+28 (submodel record) + gate 3 | All eleven regional rows converge 0.02–4.0% at ≤150 k nodes with Pilkey cross-checks (docs/28 §5); 13 gmsh/10 Netgen four-mesh rows preserved unaltered; centroid gates now PASS_SCOPED (gate 3) | Residual: `continuous_CAD_containment_verified:false` — the whole-shaft containment audit is now executed and reported (gate 3: 32 exterior / 0 on-boundary of 1,125,721 centroids, 2026-09-16); no fabrication release from FEM alone (docs/24 §10.6) |

## 2. Desk closures recorded in this ledger

### 2.1 Housing/collar key contact — desk part (gate 1)

[Study](../engineering/generated/collar-contact/report.md),
[status](../engineering/generated/collar-contact/status.json),
[source](../engineering/src/collar_contact.cpp),
[header](../engineering/include/collar_contact.hpp),
[test](../engineering/tests/test_collar_contact.cpp) (CTests
`collar_contact_benchmarks`, `collar_contact_report`). Local linear FEM
(OpenCASCADE solid → STEP → gmsh, ≤35,406 nodes, CHOLMOD/simplex direct
solve): Ø10 × 60 mm shaft section, DIN 6885 t1=1.8 mm seat (R0.15 floor
blends), key bonded, rigid collar as resolved pressures (clamp: ±x̂ traction
on two 90° arcs over the 20 mm width ≈ 7 kN total; torque: +x̂ 1000 N on the
protruding key side face reacted on the rear 120° bore arc), fixed end z=60.
Numbers: 3×3×20 → contact 44.6 MPa FEM (41.67 ideal), shear 16.67 MPa,
seat bearing 27.78 MPa, key peak vm 151.6 MPa; 3×4×18 (F109) → contact
26.2 MPa (25.25 ideal), shear 18.52 MPa, seat bearing 30.86 MPa, key peak vm
164.2 MPa; clamp-only key peak ≈20 MPa. Verdict **PASS** against the C45
allowables (bearing 120, shear 180 MPa) for both identities; the F109
3×4×18 configuration is the endorsed selection (aluminium collar-wall
bearing 43.3 MPa limit: 1.66× margin; the DIN 3×3×20 identity leaves ≈1.0×).
Physical part unchanged (gate 1 closure action).

### 2.2 Fastener preload/torque desk values (gate 4)

T = K·F·d with K=0.2 (dry zinc-plated steel, ISO 16047 range 0.15–0.25):

| Joint | Fastener | Recommended torque | Preload | Utilisation of proof load |
|---|---|---|---|---|
| H003 adapter / motor (M5×16 and M5×25, class 10.9) | A_s=14.2 mm², proof ≈11.8 kN | 6 Nm | 7.5 kN | 64% (matches the adapter FEM's 8.0 kN F0 = 60% screen order) |
| F108 roller retaining (M5×16, class 10.9) | A_s=14.2 mm², proof ≈11.8 kN | 6 Nm | 7.5 kN | 64% |
| Bed F301 (M10×35 class 8.8 into DIN 929 nuts) | A_s=58 mm², proof ≈33.6 kN | 47 Nm (docs/31:164) | 23.5 kN | 70% |
| Payoff spindle M20 nut | — | finger-tight + supplier spec (docs/31:164) | — | — |

These are desk torque-to-preload conversions, not assembly authorisations:
apply and record the actual torque values in the production build record at
assembly.

### 2.3 Roller key identity reconciliation (gate 5)

docs/28 §2.3 computes the UMCL roller key shear as "τ = 1000/(3·20) =
16.7 MPa" (a 3×3×20 key). The authoritative part record is docs/12:109
F109 = **3×4×18** C45+N (width 2.975–3.000, height 3.950–4.000, length
17.90–18.00, R1.5 ends, 0.2×45° shaft-facing chamfer), matching
catalogue-evidence "3×4×18 mm positive key" and the 2.35 mm-deep collar
keyway (protrusion 2.2 mm + 0.15 mm top clearance). The collar-contact
study (gate 1) evaluated both identities; both pass, F109 governs. The
slip is recorded here; docs/28 is left unedited.

### 2.4 Kübler wheel set-screw torque desk value (gate 6)

M4×6 ISO4029 cone-point set screw into the E40S6/M202 shaft flat: desk
value **2.0 Nm** (cone-point practice range 1.5–2.5 Nm for M4 into a flat;
retention/torque connection, not a clamping joint). Record the applied
torque in the build record at assembly; the supplier's value governs if
provided.

### 2.5 Bearing/housing compliance and thermal field — desk parts (gate 10)

[Study](../engineering/generated/bearing-contact-thermal/report.md),
[status](../engineering/generated/bearing-contact-thermal/status.json),
[source](../engineering/src/bearing_contact_thermal.cpp),
[header](../engineering/include/bearing_contact_thermal.hpp),
[test](../engineering/tests/test_bearing_contact_thermal.cpp) (CTests
`bearing_contact_thermal_benchmarks`, `bearing_contact_thermal_report`).
Local linear FEM (OCC solid → STEP → gmsh, 22,407 / 8,914 nodes, ≤150,000
cap; resolved-pressure contact in the collar_contact.cpp pattern; scalar
steady-state conduction solve).

**Study 1 (housing compliance, PASS):** the Ø15 seat + 6002 rings + F112
block are meshed with stated surface stand-ins (Ø14.995 seat, Ø15.2
ring-bore, Ø32.3 bore) so the near-coincident conforming surfaces stay
separable for the resolved-pressure patches; the 6 µm matched interference
(record window 3–6 µm, worst case) is applied as a Lamé resolved pressure
(24 radial sectors) with a stated 35 N/µm balls+bore chain support
(sensitivity ×0.5/×2). The fit record's housing value is an **8–28 µm
diametral clearance** (F112 bore 32.008–32.019 vs 6002 OD 32.000/−0.009;
minimum thermal residual 2.432 µm) — there is **no housing interference in
the record** (the task reading "K6-class housing interference" conflicts
with the record's +8/+19 µm bore; the record governs), so the housing
carries 0 MPa fit pressure. Numbers: seat contact pressure 17.65 MPa; seat
peak vm 22.73 vs 650 MPa (42CrMo4+QT); ring peak vm 82.26 vs ≈1700 MPa
(hardened 100Cr6); housing bore-wall peak vm 0.98 vs 260 MPa (6082-T6)
under the resolved 310 N front-bearing resultant. The compliant chain
changes the load-induced seat stress by 91.9 % relative to the four-mesh
rigid-boundary peak (4.28 → 0.35 MPa), but only **3.9 MPa absolute** — the
seat stress is dominated by the shrink fit, not by housing compliance.

**Study 2 (thermal field, HOLD):** steady-state conduction FEM of the F101
front half (S31.5–113): 30 W standstill motor holding loss (24HS40
2·5²·0.6, docs/28 §2.5) applied uniformly on the Ø14 pulley-seat surface,
k = 42.7 W/mK, h = 10 W/m²K still-air convection on exposed surfaces,
front bearing seat as a stated 2800 W/m²K conduction path to the 25 °C
housing (series chain ≈1.2 W/K), roller surfaces pinned at 25 °C, rear cut
face insulated (upper bound: all 30 W flows forward). Front-seat rise
**29.66 K (max) / 19.98 K (mean)** vs the fit record's explicit 20 K
shaft-above-housing envelope → **not met** at the central stated
conductance (sensitivity 15.5 / 51.7 K at 8000 / 1000 W/m²K). The desk FEM
does not robustly demonstrate the envelope for the prescribed worst-case
bound; the instrumented cold-start/hot-run test (docs/12:38) and the
measured motor-case temperature remain the required closure. The S5M belt
thermal break and the rear-half conduction path are stated mitigations,
not used to relax the number. `thermal_field_verified` stays false.

## 3. Counts

- DESK-CLOSABLE rows: 8 (gates 1, 3–7, 22, 44). Desk part now closed and
  recorded in §2: 6 (gates 1, 3, 4, 5, 6, 44); in progress in parallel
  workstreams: 1 (gate 22); open desk item requiring a released procedure
  document: 1 (gate 7). In addition, the desk parts of the PHYSICAL-ONLY
  gate 10 (housing compliance + thermal field) are closed and recorded in
  §2.5; gate 10 remains PHYSICAL-ONLY because its remaining action is the
  physical build measurement and instrumented test.
- PHYSICAL-ONLY rows: 19 (gates 2, 8–12, 14, 16–18, 23–29, 42, 43).
- EXTERNAL-INQUIRY rows: 17 (gates 13, 15, 19, 20, 21, 30–41).
- Mixed rows counted by their governing remaining action: gates 2, 13, 21
  and 41 count as PHYSICAL-ONLY/EXTERNAL-INQUIRY as listed above.
- Total open rows: 44. CLOSED rows: 0 (no gate is fully closed — every
  closed desk part retains its physical/enquiry step).

Every row above keeps the original acceptance criteria unchanged; no raw
failed data was deleted and no gate was closed by relaxing a number.
