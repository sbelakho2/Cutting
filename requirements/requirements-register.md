# Requirements and operating-envelope register (M01)

**Build:** completion agent work on `completion/2026-10-29`, source pinned at `0f28aa3`.
**Authority:** `Cutting_Agent_Handoff_2026-10-29/ENGINEERING_HANDOFF.md` sections 0–2, 12, 14, 18; `data/original-47-goals.csv`; `data/additional-user-requirements.csv`; `data/goal-verification-map.csv`. Where this register and the handoff disagree, the handoff wins and this register is wrong.
**Status vocabulary (handoff §14):** `defined` → `implemented` → `numerically verified` → `physically validated` → `qualified for stated scope`. No row is ever collapsed to a bare PASS. A failed hard gate requires a correction and rerun, not a status edit.
**Machine-readable canonical rows:** `requirements/requirements-register.csv` (atomic IDs `E…`, `G…`, `U…`). This file defines the shared measurands, margin policy and decision register.

## 1. Frozen envelope (E-rows)

| ID | Measurand / definition | Value |
| --- | --- | --- |
| E01 | Material scope | Single-conductor stranded copper/PVC. Cross-section 0.5–2.5 mm². Actual OD 1.2–4.5 mm. Solid copper, other insulation, aluminium, multicore, coax, shielded and live wire are **out of scope** unless separately designed/qualified. |
| E02 | Active coordinate contract | X = wire travel; Y = opposing cutting strokes; Z = up; cut plane X0; wire at Y0/Z112. All units mm, force N, torque N·m, time s. A single active source of truth must be derived and tested against every consumer; legacy shared constants (e.g. 3200 feed pulses/rev, 40000 Hz ceiling) are quarantined and must not drive production consumers (M01). |
| E03 | Length envelope | Commanded finished length L = 200–3000 mm. Strip settings per end: 0 (disabled) or 3–25 mm, independently. Insulated grip margin 10 mm. |
| E04 | Environment | Machine room 40 °C (retained). Normal design air ≤ 45 °C; selected V20 drivers ≤ 50 °C operating inlet. Site RH/pressure/altitude: **owner decision** (handoff §3.7: do not silently narrow to 40–70 % RH for a 70 %-limited sensor; do not assume 0 % RH). |
| E05 | Duty | Design duty 10,000 h / 1.8×10⁷ cuts (0.5 cuts/s averaged; not an instantaneous rate for every 3 m part). Feed 0.50 m/s production, 0.05 m/s commissioning. |
| E06 | Load classes | Working, proof (double working where stated), fault (e.g. 5 N·m feed fault), and ultimate loads are distinct labels. RMS ≠ peak current; steps ≠ quadrature edges; ambient air ≠ case temperature; rated operation ≠ storage. |

## 2. Margin and uncertainty policy (all rows)

1. Every requirement is closed with a **signed margin**: positive means a demonstrated lower/upper bound meets the requirement after numerical, manufacturing, measurement and model-uncertainty allowances. An aggregate score may never compensate a failed hard limit (handoff §10).
2. Hard limits use worst-case/corner bounds; statistical claims require evidenced distributions and confidence (handoff §8.1, §12).
3. Measurands follow handoff §12.1: finished length is the relaxed part between specified end reference planes at recorded temperature under defined low measuring tension; strip length is its own measurand per end from the cut-end datum to the insulation edge. `T_L = 0.20 + 0.0005·L` mm half-width; `T_S = 0.20` mm per enabled end. Internal residual limits are strictly smaller than these external tolerances (§12.1). A strip move never inherits the 0.05 %·L allowance.
4. Measurement adequacy: length gauge expanded uncertainty ≤ 0.05 mm (target T/ U ≥ 4, i.e. U ≤ 0.05 mm for ±0.20 mm strips); force ≤ 1.0 N where applicable. Conformity rule (predeclared): `|error| + U ≤ T`. Capability: original Cpk ≥ 1.33 with ≥ 30 unique consecutive pieces per cell; proposed added release criterion (owner decision) is a one-sided 95 % lower confidence bound on Cpk ≥ 1.33.
5. Model evidence standard (handoff §9): stated decision/measurands/validity envelope, input provenance, solver versions/hashes, ≥3 systematic refinements, convergence/balance (adapter gates: peak last-pair ≤ 5 %, displacement ≤ 3 %, peak ≤ 216.67 MPa; CFD/thermal targets: energy imbalance < 1 %, mass imbalance < 0.1 %, critical temperatures < 0.2 K on final refinement), independent checks, negative controls, machine-readable manifests. A model may not receive its truth from the controller/spec it tests.
6. Physical tests use the test-record schema of handoff §11.1. Synthetic data is never relabelled physical; simulation is never called a test; scope-limited exits never conceal full-design failures.

## 3. Requirements

Atomic rows live in `requirements/requirements-register.csv`. Grouping and acceptance work follow handoff §2 exactly (goal IDs G01–G47 map 1:1 to the original 47 goals; sub-IDs atomise multi-measurand goals without weakening any of them). Verification mappings reuse `data/goal-verification-map.csv` model IDs (M01–M48) and physical test families (P00–P15).

Summary of atomisation (details in CSV):

- **G01 length accuracy** → G01.1 external tolerance incl. L-cases 0.30/0.70/1.70 mm at 200/1000/3000; G01.2 relaxed measurement definition; G01.3 closed internal error budget (correction residual is not external accuracy).
- **G02 strip accuracy** → per-end ±0.20 mm; independent/asymmetric/disabled combinations; no length-proportional allowance on strip moves; incision/slip included.
- **G03 cut/strip quality** → measurable strand/jacket/end criteria; inspection method; blade material/edge geometry/wear limit; vision/count presence alone insufficient.
- **G04 throughput** → frozen rate matrix by material/length/strip; all physical states included; no unsupported examples (3 m feed ≥ 6 s at 0.50 m/s before other work).
- **G05 capability** → Cpk ≥ 1.33; ≥ 30 unique consecutive pieces/cell; distribution/autocorrelation/uncertainty treatment; 30 pieces is a minimum record, not rare-event proof.
- **G06 benchmark** → dated Komax comparison on identical measurands; no unproved parity.
- **G07 material envelope** → 0.5–2.5 mm² Cu/PVC, OD 1.2–4.5 verified through guides/nips/blades/sensors at extremes and risky intermediates; representative lots.
- **G08 geometry range** → 200–3000 mm; independent 0 or 3–25 mm strips; 10 mm grip margin; shortest/maximum-asymmetric cases.
- **G09 speed** → 0.50 m/s with hot loaded torque/traction/payoff/control timing; 0.05 m/s commissioning; no silent speed reduction.
- **G10 feed configuration** → 2000 pulses/rev; 260 rpm ceiling; frozen delivered settings; loaded-radius calibration; min loaded radius ≥ 19.89437 mm to keep 500 mm/s inside 8000 Hz (or a qualified higher ceiling); one active constants source.
- **G11 blade axis** → 12.5 mm/s, 300 mm/s², 10 mm lead, 1000 pulses/rev = 100 pulses/mm; one real screw/driver configuration with actual end machining, direction, homing.
- **G12 head loads** → 2500 N axial / 750 N transverse / 375 N vertical, all signs, double proof; complete load path incl. blocks, nut, supports, receivers, bolts, base, tooling.
- **G13 feed loads** → 1 N·m × 2.5 reversal; 5 N·m fault; belt ±480 N; roller ±60 N both planes; rotor roles/dynamic sharing; 75 N nip upper case retained.
- **G14 stiffness** → machined section ≥ reduced model; 191-section audit retained plus discontinuity-adaptive checks; final geometry/tolerances.
- **G15 fatigue** → 10,000 h / 1.8×10⁷ cuts with consistent usage spectrum, mean stress, scatter, surface/size/notch corrections and tests.
- **G16 fits** → matched j5, 3–6 µm actual interference, remaining bearing clearance; measured pairs with capable metrology; thermal fields resolved first.
- **G17 bearing contact** → acceptable contact stress with real fits, preload, loads and housing flexibility.
- **G18 shaft heat** → shaft rise ≤ 20 K above housing across the envelope; the stored 29.6603 K conditional failure is not a reason to change the requirement.
- **G19 collar** → F109/collar withstands 5 N·m plus 7 kN clamp; slip/settlement/fretting proof.
- **G20 adapter** → converged peak ≤ 216.67 MPa screen (certified ≥ 650 MPa material, factor 3); R7 numbers are candidates until regenerated; convergence gates 5 %/3 %.
- **G21 cut force** → measured cut-force envelope inside actual hot axis capability (screw, supports, guides, motor, driver).
- **G22 loaded face gap** → 0.020 mm nominal under load; allowed interval derived from cut quality; complete loaded chain incl. holders, backing, blade flexure, joints, thermal drift.
- **G23 nips** → each of the four machine nips 50–75 N (target 60 N) over OD, travel, hysteresis, temperature, wear; traction without conductor/jacket damage.
- **G24 measuring wheels** → each of two wheels 6–19 N (target 10 N); loaded-circumference calibration; sensor-validity intervals.
- **G25 head integration** → complete motor/coupling/head fit; independent jaw sweep; tolerance-extreme continuous motion; tool/cable access.
- **G26 machine integration** → static/moving integration incl. payoff/buffer, guards, cabinet, wiring, output, service/removal paths.
- **G27 payoff** → ≤ 30 kg retained; small and large reel envelope (owner decision to freeze package dimensions); loading/retention/braking; safe loading device included in cost.
- **G28 belt** → 180–200 N (target 190); corrected 79.5775 Hz nominal tooth-pass; dynamic/resonance margin; both tension stations recorded separately.
- **G29 thermal drivers** → 40 °C room; hotter-rated replacement drivers (V20 candidates) with operating-inlet evidence; normal air ≤ 45 °C; physical margins.
- **G30 cabinet losses** → all losses incl. payoff/IO/bleed counted; installed filtered flow (≥ 800 m³/h service target from 890 m³/h two-path catalogue, measured installed); hotspots, faults, dirty filters.
- **G31 contactors** → K1–K3 adequate DC make/break on 36/60 V branches with exact catalogue topology; inrush/life/opening/bus behavior.
- **G32 protection** → exact fuse/breaker voltage, breaking capacity, curves, I²t, wire/terminal limits, PSU-limited faults; input vs output protection separated.
- **G33 regeneration** → per-branch energy/headroom (`V_final = sqrt(V_i² + 2E/C)`; the 57.411 V example is not a safe 36 V branch); exact absorption/bleed parts; measured bus decay.
- **G34 guards** → openings/distances for all hazards incl. added reel/buffer/conveyor; assessed foreseeable use.
- **G35 safety function** → risk-derived PLr/SIL; complete dual-channel E-stop/guard schematic; certified data; diagnostics; common-cause; physical validation. Ordinary MCU/relays are not automatically safety-rated.
- **G36 guard release** → release only after safe energy/standstill; safe power-loss direction; welded-contact and manual-recovery handling.
- **G37 custom axis** → complete geometry with exact verified components; unchanged loads/accuracy/life; actual equivalent cost; no cost-only C7/BK12-class substitution of ratings.
- **G38 sourcing lines** → every line exact MPN/configuration, quantity/MOQ, stock/date, tax/freight/currency, valid source; all custom operations costed; blank ≠ zero.
- **G39 open enquiries** → exact couplings, axis, monitor, protection, mats, custom work, Tunisia delivery compared; exact RFQs drafted but not sent without instruction.
- **G40 software/safety/start** → Rust/C/C++ machine stack; target cross-build and startup/timing/fault tests; independent safety; uncommissioned start prevented, approval bound to build identities, not bypassable.
- **G41 assembly** → independent competent builder completes the one PDF without choosing geometry, wiring, torques, variants or parts.
- **G42 screenless** → approved retained recipe; panel states for start/stop/reset/repeat/recovery/material-change; no PC dependency for routine production.
- **G43 handling** → real prime-stub removal (10 mm stub vs 75.5 mm nearest nip), 3–25 mm slug removal, part present-to-clear with real channel and history window, supported 200–3000 mm output, quarantine; never accept on roller turns alone.
- **G44 robustness** → cold/hot starts, mains/USB/logic faults, EMI, sensor faults, friction/spool changes, wear, maintenance/recovery; includes the field-input wetting fix (old 57.9 µA/3.3 V below relay minimum) and GPIO pull policy.
- **G45 resonance/precision** → qualified relative tool-to-wire motion across speeds/duty with measured FRFs and final assembled modes.
- **G46 endurance/service** → duty-specific test, wear limits, spares, lubrication/filter intervals, replacement access and requalification; bearing L10 alone is not machine life.
- **G47 cost** → minimum adequate complete landed cost incl. machining/grinding/heat-treatment/tooling/metrology/labour/qualification/consumables; rational insourcing; like-for-like baseline; ~70 % target pursued without weakening requirements.

## 4. User instruction rows

U01–U10 from `data/additional-user-requirements.csv` are registered as atomic rows `U01`–`U10` in the CSV (no handoff PDF; one completion-agent PDF; every line found/qualified/priced; ~70 % savings; Chinese + western sourcing; cooler replacement with calculated and tested ventilation; small/large reel solution ≤ 30 kg; quality/robustness/assembly improvement while cost falls; extremely broad/deep validation; implement and finish rather than report BLOCKED). They are cross-linked to the goals in handoff §2 (27, 29–30, 37–39, 41, 43–47) and may not disappear between workstreams.

## 5. Owner decisions (handoff §18.2) — explicit, non-paralysing

| # | Decision | Known now | Provisional conservative case used until resolved | Blocking which rows |
| --- | --- | --- | --- | --- |
| D1 | Final installation/delivery (Tunisia site, Incoterm, mains, earthing, altitude/RH/dust, bench/floor, air) | 40 °C room required | Design to 40 °C room, density ≥ 1.05 kg/m³ premise, standard 230 V 50 Hz ± tolerance scenarios; conservative tax/freight sensitivity | E04, G30, G38, G39, G47 |
| D2 | Throughput matrix (pieces/hour by recipe) | 0.50 m/s; 200–3000 mm | Propose rates from M47 physics; keep requirement open; continue all other work | G04 |
| D3 | Reel geometry envelope | ≤ 30 kg; both small and large wanted | Design adaptable cradle + cassette around evidenced package families; freeze per catalogue/measurement | G07, G27 |
| D4 | Material/quality detail (constructions, lots, application criteria) | Stranded Cu/PVC 0.5–2.5 mm², OD 1.2–4.5 | Qualify extrema + risk-based intermediates; coupon-driven recipes | G03, G07, G21 |
| D5 | Process/shop capability (grinding, matching) | Custom parts proposed | Dual-route: local CNC + qualified outsourcing quotes; capability evidence before selection | G16, G37, G47 |
| D6 | Cost baseline definition | ~70 % vs reconstructed baseline; EUR 1669 cooler reference | Reconstruct full equivalent build from dated records with consistent tax/scope; report gap honestly if target not met | G47, U04 |
| D7 | Risk/reliability confidence policy | Cpk ≥ 1.33/30 pieces; 10,000 h/1.8e7 cuts | Add proposed 95 % LCB on Cpk (adopt-or-record decision); derive PLr from risk; no universal 100 % claim | G05, G35, G46 |
| D8 | Physical resources (prototype, instruments, normative texts) | No hardware demonstrated in handoff | Execute all desk-solvable work; prepare exact executable procedures; schedule physical gates honestly | all P-rows |

## 6. Traceability rules

- Every released output identifies its input hashes; rebuilding from the same inputs reproduces semantically identical geometry, tables and binaries (M01 acceptance).
- Every change to a part, resistor value, fan, wire recipe or sensor invalidates dependent evidence rather than inheriting an old green report (handoff §16.4).
- Every goal has at least one verification method and evidence owner; unresolved physical gates are explicitly scheduled, never fabricated (handoff §16.4 rule 1).

## 7. Artifact index (created by the completion agent)

| artifact | serves | status |
| --- | --- | --- |
| `tests/reproduction-2026-10-29/` | pinned evidence reproduction + revised-tree results | done |
| `software/machine/tests/material_plant.hpp` + host groups `observability`/`waste`/`negative_control` | G43/M27/M30 regression evidence | done (host) |
| `requirements/signal-table.md` | M27 signal contract; Phase-H gates listed | done (gates open) |
| `tools/reconcile_bom.py` + `bom/custom-axis-bom.csv` + `bom/selected-wire-and-payoff.csv` + `bom/sourcing-evidence-mechanical.csv` | G37/G38/G47 source quality | reconciliation CLOSED; quotes open |
| `analysis/m36_m31/` | G01/G02/G10 metrology + timing | desk closed; P-gates open |
| `analysis/electrical_thermal/` | G30/G32/G33 candidates | desk closed; TS/P-gates open |
| `analysis/cost-basis/` | G47/U04 cost ledger | NOT ESTABLISHED (70 % test) |
| `analysis/phase-h-io/option-study.md` | M27 channel delivery options | options; owner decision |
| `tests/P10-thermal/` (35) + `tests/P00-P15/` (86) | all P/TS/TF physical tests | procedures done; execution open |
| `procurement/rfq-drafts.md` | G39 | drafted, not sent |
| `release/Cutting_Assembly_Manual.pdf` | G41/U02 (the one PDF) | pre-release manual |
| `release/decision-log.md` | decisions/rejections | live |
