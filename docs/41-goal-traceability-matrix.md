# 41 — Goal traceability matrix — S1-B — audit re-verdict R2

**Revision:** `S1-B-GTM-R2-2026-10-13`
**Status:** consolidated desk record, **re-verdicted by the external audit of
2026-10-13**. This revision replaces the R1 self-assessment categories row by
row with the audit's verdicts; no verdict was softened and no acceptance
number was changed. Physical machines, measurements and qualifications do not
exist; nothing here releases fabrication or energisation.

## Verdict legend (audit status definitions, reproduced)

- **BLOCKED** — a present design/control contradiction or known failed
  calculation.
- **DESK ONLY** — a bounded documentary/model result that is useful but does
  not close physical operation.
- **UNPROVEN** — required physical/vendor evidence is absent.
- A goal can have both a correct local calculation and a blocked
  machine-level use, hence the combined verdict forms below.

Old R1 categories (MET / MET-WITH-PHYSICAL-REMAINDER / NOT-MET-DESK /
NOT-EVALUABLE) are retired: the audit's verdicts supersede them. The R1
counts "12 MET, 18 MET-WITH-PHYSICAL-REMAINDER" must **not** be interpreted
as 30 operationally achieved goals; several listed "physical remainders"
contained unfinished design decisions that must close before fabrication.

## 1. Audit verdicts — goals 1–40 (unchanged acceptance)

| # | Goal and unchanged acceptance | Audit verdict | Required closure |
|---|---|---|---|
| 1 | Finished length ±(0.20 mm + 0.05 % L) | **BLOCKED** | F06/F09/F10/F21; complete functional metrology and demonstrate relaxed-part errors at 200–3000 mm |
| 2 | Strip accuracy ±0.20 mm | **BLOCKED** | F07/F08/F22; correct sequence/closure and add actual strip-accuracy cells |
| 3 | Cut quality, no conductor damage, qualified incision | **BLOCKED / UNPROVEN** | Complete blade/backing definition and cut/strip qualification; counting is not inspection |
| 4 | Throughput/cycle budget | **BLOCKED** | F29/F19; supported-length, state-machine timing and a stated rate requirement |
| 5 | Cpk ≥ 1.33, ≥ 30 consecutive pieces/cell | **BLOCKED / UNPROVEN** | F32; unique actual pieces, complete process envelope, guarded metrology and stable-process evidence |
| 6 | Komax survey/benchmark without parity claim | **DESK ONLY** | Comparison is useful; update stale S1 component/feature rows, preserve the lack of measured parity |
| 7 | 0.5–2.5 mm² Cu/PVC; OD 1.2–4.5; lane clearances | **DESK ONLY / UNPROVEN** | Local clearance calculation is useful; traction, handling, actual lots and quality remain unqualified |
| 8 | 200–3000 mm; independent 3–25 mm strips; 10 mm insulated grip margin | **BLOCKED** | F06/F07/F11; minimum-part sensing and process geometry; supported 3 m output |
| 9 | 0.50 m/s feed; initial 0.05 m/s | **BLOCKED / UNPROVEN** | Command frequency fits nominally; F19/F20 mean payoff/traction do not establish the speed envelope |
| 10 | 2000 feed pulses/rev; 260 rpm ceiling; loaded radius calibration | **DESK ONLY / UNPROVEN** | Nominal operating point is 238.7 rpm/7958 Hz; qualify loaded radius, driver setting and actual motion |
| 11 | Blade 12.5 mm/s, 300 mm/s², 100 pulses/mm for 10 mm lead | **BLOCKED** | F13–F15; select/freeze screw/driver map and revalidate the real custom axis |
| 12 | Head acceptance loads 2500 N axial, 750 transverse, 375 vertical; double proof | **BLOCKED** | F14/F15/F22; do not substitute 1200/2400 N or old HIWIN capacities |
| 13 | Feed 1 Nm ×2.5; 5 Nm fault; belt ±480 N; roller ±60 N | **DESK ONLY, INCOMPLETE** | Existing bounded shaft/belt analyses useful; F24 and actual traction/jam/rotor roles must be included |
| 14 | Machined section stiffness ≥ reduced model, 191-section audit | **DESK ONLY** | Sampled nominal result retained; tolerance-continuous proof, local contacts and fabrication remain separate |
| 15 | Shaft fatigue, 10,000 h / 1.8×10⁷-cut duty | **UNPROVEN** | F24/F31; correct spectrum and validate actual shaft/manufacture under it |
| 16 | Matched j5, 3–6 µm interference, stated clearance envelope | **BLOCKED / UNPROVEN** | Conditional corner screen retained; F23/F35 require resolved temperature paths and adequate measured matching |
| 17 | Bearing/housing contact stress within allowables | **DESK ONLY** | Local stated-load study retained; assembled stiffness, actual loads and manufacturing contacts need validation |
| 18 | Shaft ≤ 20 K above housing | **BLOCKED** | Published central thermal bound exceeds it; redesign/model then instrument worst-duty paths |
| 19 | F109/collar survives 5 Nm plus 7 kN clamp | **DESK ONLY / UNPROVEN** | Bounded contact result useful; exact collar/key geometry, torque scatter, proof/fretting records still required |
| 20 | Adapter peak converged and ≤ 216.67 MPa screen | **BLOCKED** | Reported ≈313 MPa remains above the screen; custom support invalidates automatic reuse; do not substitute yield alone |
| 21 | Blade force within actual axis capability | **BLOCKED / UNPROVEN** | Actual selected screw/support/motor data and measured wire/blade force; old KK86 input-torque cap is not a custom rating |
| 22 | Blade-face gap 0.020 mm under cut | **BLOCKED** | Complete tooling/backing and assembled load-clearance verification |
| 23 | Nip force 50–75 N, target 60 | **DESK ONLY / UNPROVEN** | Measure each nip, actual sharing, damage and friction; F20/F24 |
| 24 | Each measuring wheel force 6–19 N, target 10 | **BLOCKED / UNPROVEN** | Integrate both modules and resolve CN fits; qualify both wheels individually |
| 25 | Full head/coupling fit and independent jaw sweep | **BLOCKED for selected build** | Existing envelope checks are useful historical scope; custom hardware geometry must be integrated and checked |
| 26 | Full layout/mechanism static integration | **BLOCKED** | Second module, custom internals, payoff torque path, cabinet, takeaway and guard mechanics are incomplete |
| 27 | Payoff ≤ 30 kg, spindle static support | **DESK ONLY / UNPROVEN** | Ø20 shaft beam result is plausible; complete retention/torque/weld load paths, weigh and load-test |
| 28 | Belt 180–200 N, target 190; dynamic margin | **DESK ONLY, CORRECTION REQUIRED** | Static tension arithmetic useful; use 79.58 Hz tooth-pass and measured resonant response |
| 29 | 40 °C room; drive ambient ≤ 40 °C | **UNPROVEN / INCOMPLETE** | Complete cabinet loss/airflow/overtemperature design, then actual 40 °C mapping |
| 30 | Cabinet dissipation within cooler capacity | **BLOCKED as current budget** | Include all four drives, all supplies/coils and cabinet heat gains before declaring margin |
| 31 | K1–K3 DC make/break duty on 36/60 V branches | **UNPROVEN** | Catalogue series-pole selection useful; actual capacitive inrush, bus topology and measured interruption still required |
| 32 | Correct branch/XPS protection and coordination | **BLOCKED as claimed closure** | Fuse curves/I²t/DC interruption and PSU-limited faults need complete coordination, not nominal-current equivalence |
| 33 | Regeneration absorption and deterministic discharge | **BLOCKED** | Voltage-headroom calculation, exact components and measured branch decay/stop behaviour |
| 34 | Guard aperture safety distance | **UNPROVEN** | Assess actual opening type and nearest hazards, all paths, door/guard states and machine use; blade-only distance insufficient |
| 35 | Two-channel independent E-stop/guard safety function | **UNPROVEN** | Complete schematic/risk-required performance, fault injection, stop/energy validation |
| 36 | Guard release only after safe energy/standstill; safe power-loss direction | **BLOCKED at design level** | F25/F26; exact independent contacts, release request, timer/monitor and validated mechanics |
| 37 | Cheaper custom cut axis meeting unchanged goals | **BLOCKED** | F13–F15; manufacturable custom assembly, correct scale, actual ratings, original loads and inspected equivalent quote |
| 38 | Priced/selectable sourcing lines | **DESK ONLY, INCOMPLETE** | Dated observations remain observations; exact package/kit contents and purchase-time offers must agree |
| 39 | Remaining coupling/cooler/drive/Tunisia enquiries | **UNPROVEN** | Obtain and reconcile actual offers; complete missing items; no landed total established |
| 40 | Rust/C/C++ machine stack; independent safety; no uncommissioned start | **PARTLY MET / BLOCKED** | Language and hardwired-safety intent retained; production approval absent, target startup broken, validation incomplete |

## 2. Added goals 41–47 (absent/underrepresented in R1; release criterion from the audit)

The audit requires these as explicit rows. Status is mapped from the audit's
findings for each (the audit table itself states the release criterion); no
desk evidence closes any of them yet.

| # | Goal | Audit-mapped status | Release criterion to add (audit) |
|---|---|---|---|
| 41 | Easy, reproducible assembly | **BLOCKED** (F35: release travellers/kit not complete) | A competent builder completes a release traveller without choosing geometry, component variants or wiring; correct parts/quantities, tools, access and fixtures are supplied |
| 42 | Screenless routine operation | **BLOCKED** (F12: no commissioning/recipe approval interlock) | Safe panel-only start/stop/reset/repeat batches with retained approved recipe; distinguish partial/scrap recovery, material changes and calibration service |
| 43 | Complete automatic handling | **BLOCKED** (F06/F07/F11) | Verified prime, independent strips, slug disposal, output clearance and supported full-length delivery |
| 44 | Factory robustness | **UNPROVEN** | Cold/hot start, mains/USB loss, EMI, sensor open/short/stuck faults, friction variation, spool depletion, wear and maintenance recovery |
| 45 | Resonance/structural precision | **DESK ONLY, CORRECTION REQUIRED** (F30) | Qualified relative blade/wire displacement over the allowed speed/duty envelope, with stated sensor bandwidth |
| 46 | Whole-machine endurance/serviceability | **UNPROVEN** (F31) | Duty-specific endurance acceptance, preventive maintenance, spares and inspected replacement/recalibration procedures |
| 47 | Cost and insourcing | **UNPROVEN** (F35) | Complete equivalent delivered build cost, machining/grinding/metrology/qualification labour and consumables; exact externally supplied items identified |

## Counts (audit re-verdict)

| Verdict carried | Goals 1–40 | Added 41–47 | Total rows |
|---|---|---|---|
| BLOCKED (alone or combined) | 22 | 3 | **25** |
| DESK ONLY (alone or combined) | 11 | 1 | **12** |
| UNPROVEN (alone or combined) | 17 | 3 | **20** |
| PARTLY MET / BLOCKED | 1 | 0 | **1** |
| **Rows** | **40** | **7** | **47** |

Combined verdicts mean one row is counted under each verdict it names, so
columns do not sum to the row totals. No row is MET; no criterion was
relaxed and no raw failed data was deleted. The audit's fix references
(F01–F35) and required closures above are the exact remaining work; the
qualification subset (200–1000 mm, 15–30 °C) is labelled a subset everywhere
and does not qualify 3000 mm/hotter by implication ([docs/01 §2](01-design-basis.md)).

## Relation to the gate ledger

Every physical/external closure referenced above remains in
[docs/33](33-gate-ledger.md) with its category unchanged; this matrix adds
the audit's re-verdicts and the seven added goals, and removes no gate. The
R1 matrix's per-row "MET" desk closures were reviewed by the audit and
re-verdicted as shown; the earlier adapter/throughput/custom-axis/integration/
software rows in particular overstated completion.
