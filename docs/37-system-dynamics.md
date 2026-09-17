# 37 — Whole-machine dynamics and error-budget simulation — S1-B

**Revision:** `S1-B-DYN-R1-2026-09-16`

**R2 pointer (2026-09-17):** the RU3100 encoder is now parameterised to
**2048 PPR via IO-Link** ([docs/13 §5 step 9](../docs/13-measuring-interfaces-and-calibration.md));
the §3 error-budget, drive step-response, E-stop and payoff/feed numbers are
recomputed in [docs/42](42-process-dynamics-r2.md), which supersedes §1/§3/§5
numbers below where they differ. The §3 resolution line and the quantisation
row below are marked accordingly.

**Status:** desk calculation record. Every number below is derived from the
repository constants ([machine_spec](../shared/machine_spec.hpp),
[feed_drive_spec](../shared/feed_drive_spec.hpp),
[cutting_head_spec](../shared/cutting_head_spec.hpp),
[measurement_spec](../shared/measurement_spec.hpp)) plus the archived
datasheets named per section. Where a manufacturer publishes no value, the
assumption is stated with its source and the reason. No physical machine,
measurement or qualification exists; nothing here releases fabrication or
energisation.

## 1. Drive sizing

### 1.1 Feed axis — reflected inertia (worst case, per motor/cassette)

Each cassette has one motor driving two lower rollers through one belt
([docs/12:13](../docs/12-feed-cartridge-fabrication.md)). Roller radius
r = 20 mm (`feed_drive_spec.hpp catalogue::roller_d/2`); belt drive is 1:1
(20-tooth pulleys both ends).

| Element | Value | Basis |
|---|---|---|
| Motor rotor | 1.080e-4 kg·m² | [24HS40-5004D-E1000](../engineering/sources/24HS40-5004D-E1000.pdf): rotor inertia 1080 g·cm² |
| Coupling MK2/100/50/10/12 | ≈1.6e-5 kg·m² | Assumed equal to the archived MK2/100/50/8/14 family value `coupling_inertia_kg_m2 = 160e-7` in [cutting_head_spec](../shared/cutting_head_spec.hpp) — stated assumption, the 10/12 datasheet inertia is not separately archived |
| Shaft ×2 | ≈4.5e-6 kg·m² each | Geometric estimate, variable Ø10–19 steel sections over 130 mm (`feed_drive_spec.hpp` stations) |
| Pulley HTPA20S5M250-A-N14 ×2 | ≈9.8e-6 kg·m² each | Geometric estimate: aluminium Ø30.87 × 32 mm body + Ø36 flanges ≈ 77 g at pitch radius 15.92 mm ([misumi-s5m-pulleys](../engineering/sources/misumi-s5m-pulleys.pdf): A2000-series aluminium, flange type) |
| Roller UMCL40-10-20-MC ×2 | ≈2.8e-5 kg·m² each | Geometric estimate: Ø40 × 20 mm, steel core + PU shell ≈ 0.14 kg at r = 20 mm ([misumi-collar-rollers](../engineering/sources/misumi-collar-rollers.pdf) publishes no mass) |
| Belt HTBN225S5M-250 | 4.8e-6 kg·m² (19.1 g) | [misumi-s3m-belts](../engineering/sources/misumi-s3m-belts.pdf): S5M HTBN unit mass **34.0 g/m at 10 mm width** → 85 g/m at 25 mm × 0.225 m; reflected as m·r_p² |
| Wire, worst case | 2.21e-4 kg·m² (553 g) | Ø4.5 mm Cu: ρ = 8960 kg/m³ → 0.1425 kg/m; longest commanded part 3000 mm (`machine_spec.hpp max_length_um`) + 880 mm machine route ([docs/26](../docs/26-guard-and-payoff-selection.md)) = 3.88 m between payoff brake and cut position; reflected m·r² |

**Total reflected inertia ≈ 4.35e-4 kg·m².** The wire term (51 %) exceeds the
motor rotor; the inertia is dominated by the moving wire mass, not the drive
hardware.

### 1.2 Feed axis — required torque and operating points

Angular acceleration at 1.5 m/s² (`machine_spec.hpp feed_accel_um_s2`):
α = a/r = 75 rad/s².

| Operating point | Speed | Torque | Basis |
|---|---|---|---|
| Steady 500 mm/s | 239 rpm | 1.0 Nm | `feed_drive_spec.hpp design::running_torque_nm` envelope (equivalent to 50 N wire pull at 20 mm radius) |
| Peak: 1.5 m/s² + running | up to 239 rpm | **1.03 Nm** | 1.0 + J·α = 1.0 + 0.033 |
| With 2.5 reversal service factor | — | **2.58 Nm** | `design::reversal_service_factor = 2.5` on the running envelope, acceleration included |
| 5 Nm mechanical fault | 0 rpm | 5.0 Nm | `design::mechanical_fault_torque_nm` |

Belt capacity across 50–260 rpm: minimum **10.89 Nm** at 260 rpm
([feed-analysis](../engineering/generated/feed-analysis/report.md)) → ratio
**4.36×** against the 2.5 Nm factored envelope and **2.18×** against the
5 Nm fault envelope (existing PASS checks). The belt is not the binding
element.

Motor torque at the operating point: the archived
[torque-curve PDF](../engineering/sources/24HS40-5004D-E1000-torque.pdf)
shows ≈**4.4 Nm** near standstill (the datasheet table prints 4.0 Nm holding
at 5.0 A/phase, 0.60 Ω, 2.60 mH). The curve value at 239 rpm is **not
digitised in the repository**; the sizing below uses a stated assumption band
of 60–70 % of the 4.4 Nm curve value = **2.6–3.1 Nm at 239 rpm**
(conservative band for a 4 Nm-class NEMA 24 stepper on the 36 V CL57T/LE-350-36
drive; no extrapolation below 50 rpm is claimed, per the standing repo rule).

| Margin basis | Value | Verdict |
|---|---|---|
| Curve (2.6–3.1 Nm) vs peak demanded 1.03 Nm | **2.5–3.0×** | PASS at the assumed curve |
| Curve vs fully factored 2.58 Nm | **1.01–1.20×** | **Marginal** — the motor, not the belt, is the binding drive element if the 2.5 reversal factor is applied to the motor itself |

**Feed-axis verdict:** desk PASS with one flagged condition — digitise the
archived torque curve at 200–300 rpm before release; if it falls below
≈2.6 Nm at 239 rpm, reduce `feed_accel_um_s2` or re-select the motor. The
standard interpretation (service factor sizes the belt; the motor is sized on
peak torque with a ≥50 % margin, i.e. ≥2.06 Nm at 239 rpm) is satisfied by
the assumption band.

### 1.3 Head cut axes — blade force and screw thrust

Blade force from shear-area literature: ultimate shear strength of annealed
copper τ_u ≈ 0.6–0.7 × UTS ≈ **135–165 MPa** (UTS 210–240 MPa; Kalpakjian &
Schmid, *Manufacturing Engineering and Technology*, ultimate-shear-strength
relation; ASM Handbook shear values). Guillotine (square-cut) force on the
Ø4.5 mm wire: A = π/4·4.5² = 15.9 mm² → **2.15–2.62 kN**, central 2.39 kN.
The opposed 90° V blades cut progressively on two 45° faces; inclined-blade
shearing reduces the axial force (Lange, *Handbook of Metal Forming*;
Boljanovic, *Metal Shaping Processes*). Stated wedge factor 0.3–0.5 →
**F_cut ≈ 700–1200 N**, central ≈ 950 N. Conductor-only lower bound
(4.5 mm² Cu): 675 N guillotine ≈ 200–340 N with the wedge factor.

Screw thrust at 10 mm lead (`cutting_head_spec.hpp catalogue::lead`), η = 0.9
(`design::efficiency_lower`):

| Source | Torque | Thrust 2π·η·T/lead |
|---|---|---|
| KK86 stage max input torque 2.79 Nm ([hiwin-kk86-catalogue-2026](../engineering/sources/hiwin-kk86-catalogue-2026.pdf), Table 5.1) | 2.79 Nm | **1578 N** |
| Motor curve at 75 rpm (12.5 mm/s blade speed; interpolated 7.475 Nm, −0.1 Nm read allowance) × 0.8 running-curve fraction × 1.25 minimum margin convention (`cutting_head_spec.hpp design::`) | 5.98 Nm | **3381 N** |

Margins against F_cut:

| Check | Ratio | Verdict |
|---|---|---|
| Stage 2.79 Nm cap (1578 N) vs 1200 N worst estimate | 1.32 | PASS (≥1.25 repo minimum) |
| Stage cap vs 700 N low estimate | 2.25 | PASS |
| Motor (0.8 × curve) vs 1200 N | 2.8 | PASS |
| Screw dynamic rating 7144 N vs 1200 N | 5.9 | PASS |

The `design::working_thrust_n = 2500 N` acceptance load **cannot be produced
by the drive** (stage cap 1578 N); per the spec comment it is an explicit
externally applied acceptance/qualification load, not a drive demand — the
two figures must not be compared as drive margin. Blade speed 12.5 mm/s is
1.7 % of the KK86 740 mm/s limit; acceleration 300 mm/s² is 2 % of the stage
15 m/s² limit — the cut axes run far inside the stage envelope.

## 2. Belt dynamics

First natural frequency of the 225 mm S5M 25 mm span, string model
f₁ = (1/2L)·√(T/μ), L = 62.5 mm span (equal pulleys), μ = 85 g/m
(MISUMI 34.0 g/m per 10 mm width, [misumi-s3m-belts](../engineering/sources/misumi-s3m-belts.pdf)):

| Initial tension | f₁ |
|---|---|
| 180 N (acceptance min) | 368 Hz |
| **190 N (target)** | **378 Hz** |
| 200 N (acceptance max) | 388 Hz |

Worst step excitation: 500 mm/s feed / 5 mm pitch = **100 Hz tooth-passing
fundamental**, trapezoid harmonics at 200/300 Hz. Separation of the first
belt mode from the fundamental is **3.8×** and the first three mesh harmonics
stay below f₁. Verdict: **PASS** for the string model; belt bending
stiffness raises f₁ further. Measure the strand frequency at commissioning
(the 1.000 mm-deflection/14.5 N method of [docs/12](../docs/12-feed-cartridge-fabrication.md)
sets the same quantity).

Tension requirement: initial 180–200 N, target 190 N, manufacturer maximum
initial tension 221 N ([misumi-belt-installation](../engineering/sources/misumi-belt-installation.pdf)
Table 32). At 180 N initial and the 5 Nm fault torque the slack strand keeps
**22.9 N > 0** (existing [check](../engineering/generated/feed-analysis/checks.csv)) —
no tooth jump at the fault envelope. Belt length tolerance ±0.41 mm maps to
±0.205 mm centre change before elastic tensioning (existing record).

## 3. Measuring-chain error budget

Per [measurement_spec](../shared/measurement_spec.hpp) (R1 numbers; the
2026-09-17 2048 PPR setting supersedes them — see [docs/42](42-process-dynamics-r2.md)):
200 mm E60137 wheel, 1024 PPR RU3100 → 4096 counts/rev,
**0.048828 mm/count** (superseded by 0.024414 mm/count at 2048 PPR).
Target: ±(0.2 mm + 0.05 %) of commanded length
([docs/20](../docs/20-physical-qualification.md)) = **±0.30 mm at 200 mm**,
**±0.70 mm at 1000 mm** (the ±0.70 mm/m acceptance).

| Term | Value at 1 m | Basis |
|---|---|---|
| Wheel factory circumference | ±1.0 mm | ±0.2 mm on 200 mm = 0.1 %; **assumed** — ifm publishes no tolerance (`measurement_spec.hpp` placeholder) |
| Calibration procedure | +0.025 mm + 0.01 % (=0.125 mm at 1 m) | Chosen metrology allocation; traceable reference, 32×128-count phase map, held-out verification ([docs/13 §4–5](../docs/13-measuring-interfaces-and-calibration.md)) |
| Quantisation | ±0.024 mm (R1) | Half-count, 0.048828/2, superseded: at 2048 PPR the half-count is 0.0122 mm and the allocation halves to 0.025 mm ([docs/42 §1](42-process-dynamics-r2.md)) |
| Encoder edge jitter | ±0.02 mm | ifm publishes no jitter; stated assumption ≤ ±1° electrical + ±1 µs capture timing at 10250 edges/s (500 mm/s); non-accumulating |
| Wheel slip at 6–19 N force | 0.25 mm allocation | Traction μ·F_c ≈ 0.5 × 19 = 9.5 N vs demand ≈ 0.5 N (bearing/encoder drag 0.31 N + 0.10 N accel) → 6–20× margin; the 0.25 mm is the retained qualification allocation, not a measured slip |
| Temperature 15–30 °C | ≈0.23 mm | Al wheel core 23e-6/K × 10 K; PU tread and indentation effects are **unquantified** and flagged; wire Cu 17e-6/K cancels (feed and measure at the same temperature) |
| Runout/TIR 0.2 mm | 0.2 mm uncalibrated | Calibration maps the first harmonic (synthetic bound 0.001 mm); arbitrary lobing does not inherit that bound |
| Wire tension/extension | 0.15 mm allocation | Recipe-controlled term |

Worst-case sums:

| Configuration | 200 mm | 1000 mm | vs target | Verdict |
|---|---|---|---|---|
| Uncalibrated (factory scale + allocations) | 0.85 mm | **1.65 mm** | 0.30 / 0.70 | **FAIL** (2.4× over at 1 m) — matches the existing [measurement budget](../engineering/generated/measurement-analysis/report.md) |
| Calibrated (calibration replaces factory scale; allocations retained) | 0.54 mm | **0.80 mm** | 0.30 / 0.70 | **FAIL at the retained allocations**; PASS requires the slip/tension/temperature terms to be demonstrated, not allocated |

**Verdict:** traceable calibration is necessary and removes the dominant
term, but no desk number can close the ±0.70 mm/m acceptance — the residual
is held by process terms (wheel force 6–19 N, tension, 15–30 °C, direction)
that only the 30-piece qualification cells ([docs/20](../docs/20-physical-qualification.md),
gate 25) and the physical calibration (gate 29) can demonstrate. The
short-length target (±0.30 mm at 200 mm) is dominated by the constant
allocations (slip 0.25 + tension 0.15); these must be reduced by evidence,
not re-budgeted on paper. The 626 CN bearing 0.5 µm/10 K clearance residue
([docs/13 §3](../docs/13-measuring-interfaces-and-calibration.md)) remains an
open bearing-design item, not yet included in this sum.

## 4. Blade mechanics

Cut-force reaction path: blade → H004 holder (42CrMo4+QT, 70 × 122.5 × 48 mm)
→ KK86 carriage/rail → stage → riser → H001 datum plate (20 mm). Deflection
estimates (cantilever, E = 205 GPa, stated geometry):

| Element | Estimate at F_cut = 1200 N | At 2500 N acceptance load |
|---|---|---|
| H001 datum (20 mm plate, 1040 mm long) | negligible | negligible |
| H004 holder arm (70 × 44 section, ≈100 mm lever) | **≈4 µm** | ≈8 µm |
| Blade itself (3 mm plate wedge, ≈24 mm edge overhang) | **≈300 µm plain-plate bound** | ≈600 µm |

The blade-face gap is 0.020 mm (`design::blade_face_gap`); a 3 mm plain-plate
edge would deflect far beyond it, so **blade edge stiffness and backing
geometry are purchase-drawing requirements** for the unqualified blade pair
(gate 13), not achievable by the holder or datum. Holder/datum contributions
are ~1 µm-class and not design drivers.

Blade life order of magnitude for Cu cutting: **10⁵–10⁶ cuts per edge** for
hardened tool-steel V-blades on copper conductors (tool-life literature for
shear blades on Cu, Kalpakjian/ASM tooling chapters; wire-harness tooling
practice). No manufacturer blade-life figure is published in the reviewed
material ([docs/02](../docs/02-benchmarks-and-sources.md)); this is an
order-of-magnitude planning number, and the blade pair is not yet sourced or
edge-qualified (gate 13).

## 5. Payoff/feed interplay — wire tension bounds

Roller traction: μ (PU tread on Cu/PVC) ≈ **0.4–0.6** (stated tribology
handbook range for polyurethane on metals/plastics, dry). Per nip (wire
between two rollers, both faces gripping): F_pull = 2·μ·N_nip:

| Nip preload | μ = 0.4 | μ = 0.6 |
|---|---|---|
| 50 N | 40 N | 60 N |
| 75 N | 60 N | 90 N |

Two nips in series give ≈ **80–180 N** worst-case traction capacity (linear
sum; verify by pull test — nip sharing is not guaranteed). Wire acceleration
demand is trivial: 0.553 kg × 1.5 m/s² = **0.83 N**. Payoff brake guidance:
set light tension **5–20 N** (keeps the strand taut, ≪ 25 % of traction
capacity, leaves ≥ 50–100 N headroom for strip forces, and stays far below
the upstream measuring-wheel traction ≈ 3–9.5 N at μ·(6–19 N)). The brake is
a friction tension brake, not a safety brake
([docs/26 §3](../docs/26-guard-and-payoff-selection.md)); brake adjustment
trials and runaway/overspeed measurement are commissioning steps
([docs/26 §6.3](../docs/26-guard-and-payoff-selection.md)). Hard bound:
brake tension + strip force + accel force must stay below the nip traction at
μ = 0.4; verify with the creep/pull test in the physical qualification.

## 6. Thermal / EMC / robustness watchlist

Every identified machine failure mode mapped to its existing protection and
the new check it still requires.

| # | Failure mode | Existing protection | Required check (new or already gated) |
|---|---|---|---|
| 1 | Wire jam / slug | Firmware `SlugJam`, `MoveTimeout` (30 s), `FollowingError` faults; closed-loop drivers raise `DriverAlarm`; belt holds 5 Nm fault with 22.9 N slack margin; roller force envelope ±60 N | Commission jam-detection following-error threshold per wire OD; 5 Nm/10 s/10-cycle slip-marked belt bench test (docs/12:152, P4-4/P4-5) |
| 2 | Blade stick / hard cut | KK86 closed-loop 34E1K-85 + driver alarm; `MoveTimeout`; blade position limited 4.2 mm | Add a cut-stroke deviation/force monitor (no cut-force sensing exists); edge-qualified blade pair first (gate 13, P6-8) |
| 3 | Belt skip / tooth jump | 180–200 N tension setting; slack strand positive at 5 Nm; belt capacity 2.18× fault | Cross-check feed-axis encoder against measuring-wheel counts after each move; belt-frequency check at commissioning (§2) |
| 4 | Encoder/wheel slip | Wheel force 6–19 N qualification band; traction margin 6–20× (§3) | Two-wheel (upstream/downstream) agreement window per move; wheel-surface wear inspection interval |
| 5 | Backfeed / reversal error | Signed floor-division mapping with Z-index phase reference, reversal-tested ([docs/13 §4](../docs/13-measuring-interfaces-and-calibration.md)) | Direction-dependent hysteresis inspection per recipe (docs/13 step 7); backfeed must not exceed the 25 mm tail budget |
| 6 | EMI on measuring chain | Shielded encoder cables, screen bonding at PE bar, 2 × M20 EMC glands (docs/22) | **Open**: J8/J9 receiver/termination review for RU3100 TTL/HTL (written for RS-422 AM26LV32, not re-approved); EMC assessment of built cabinet (gate 41, P5-9) |
| 7 | Heartbeat / control loss | 750 ms `HeartbeatLost`; 750 µs timeout in `machine_spec.hpp` | Fault-injection validation (gate 23, P5-4/P5-7) |
| 8 | Motor overheat at hold | 30 W standstill loss; belt thermal break; documented HOLD: seat ΔT 29.66 K vs 20 K envelope | Motor-case temperature at rated hold (docs/28 §2.5, P1-1…P1-5, P9-1…P9-3) |
| 9 | Payoff runaway/backlash | Friction brake only (no guaranteed stop) | Runaway/overspeed measurement (docs/26 §6.3, P7-1…P7-4); spool ≤ 30 kg weighed |
| 10 | Length cell out of tolerance | Calibration model + fail-closed qualification tool | 30-piece cells, Cpk ≥ 1.33, gauge U ≤ 0.05 mm/1.0 N (docs/20, gate 25, P10-1…P10-5) |
| 11 | Guard/E-stop breach | XPS relay, K1–K3 NC chain, guard channels, `Estop`/`GuardOpen` faults | Stop-time and bus-decay measurement, risk assessment sign-off (gates 14/23, P5-4, P8-2) |
| 12 | Belt/roller wear and fatigue | 4.36× belt capacity, 2.18× fault capacity, keyed joints | Physical rotating-fatigue test (gate 11, P6-9) and pressure/friction/jam qualification (gate 12, P6-4…P6-6) — the desk dynamics here supply load spectra, not life numbers |

## 7. Assumption register (every assumption, with source)

1. Coupling inertia for MK2/100/50/10/12 = 160e-7 kg·m², the archived 8/14
   family value (`cutting_head_spec.hpp`) — 10/12 value not archived.
2. Pulley ≈77 g, roller ≈0.14 kg, shaft ≈4.5e-6 kg·m² — geometric estimates;
   MISUMI publishes no masses for these SKUs.
3. Wire worst case = 3 m part + 0.88 m route of Ø4.5 mm Cu (8960 kg/m³).
4. Feed motor at 239 rpm = 2.6–3.1 Nm (60–70 % of the 4.4 Nm curve value in
   the archived torque PDF) — **the curve is not digitised in the repo**;
   verify before release.
5. τ_u(Cu) = 135–165 MPa and the 0.3–0.5 V-wedge force factor are literature
   ranges, not measurements on S1 wire lots.
6. μ (PU on Cu/PVC) = 0.4–0.6 — stated tribology range, unmeasured.
7. Belt unit mass 85 g/m at 25 mm width from MISUMI 34.0 g/m per 10 mm.
8. Wheel ±0.2 mm circumference tolerance is the `measurement_spec.hpp`
   placeholder — ifm publishes none.
9. Encoder edge jitter ≤ ±1° electrical + ±1 µs timing — ifm publishes no
   jitter spec.
10. Error-budget "allocations" (slip 0.25, tension 0.15, quantisation 0.05)
    are the repository's retained planning allocations, not measured bounds.
11. Blade deflection uses a plain 3 mm plate model; the real blade is a
    purchase-to-drawing wedge and may be stiffer.
12. String model for belt f₁ ignores bending stiffness (raises f₁) — the
    computed value is therefore conservative.
13. Two-nip traction summed linearly; nip load sharing must be verified.

## 8. Verdict summary

| Item | Key number | Verdict |
|---|---|---|
| Feed torque margin | 1.03 Nm peak vs 2.6–3.1 Nm assumed curve (2.5–3.0×); 1.01–1.20× if the 2.5 factor is applied to the motor | PASS with flagged curve-digitisation condition; motor is the binding element, belt is not (4.36×) |
| Belt first natural frequency | 378 Hz vs 100 Hz tooth-passing (3.8× separation) | PASS |
| Error budget total | 1.65 mm/m uncalibrated, ≈0.80 mm/m calibrated-at-allocations, vs ±0.70 mm/m (R1; R2 at 2048 PPR: 1.625 / ≈0.79 mm/m, [docs/42](42-process-dynamics-r2.md)) | FAIL on desk; calibration necessary, process terms must be demonstrated physically |
| Blade force | ≈700–1200 N (guillotine bound 2.15–2.62 kN × wedge factor) | PASS: stage cap 1578 N, motor 3381 N (0.8× curve) |
| Blade deflection | holder ≈4 µm, plain blade plate ≈300 µm vs 20 µm face gap | Blade stiffness is a purchase-drawing requirement (gate 13) |
| Blade life | 10⁵–10⁶ strokes order of magnitude (Cu) | Planning number only; unqualified tooling |
| Payoff/feed tension | traction 80–180 N, brake setting 5–20 N, accel demand 0.83 N | Guidance issued; physical pull/runaway tests required |

No repository gate is closed by this document alone; the desk halves it
supplies are recorded in the [gate ledger](../docs/33-gate-ledger.md) §2.6.
