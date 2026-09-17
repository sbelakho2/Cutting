# 42 — Process dynamics and error budget, R2 — S1-B

**Revision:** `S1-B-DYN-R2-2026-09-17`
**Status:** desk calculation record. R2 of [docs/37](37-system-dynamics.md):
it records the RU3100 **2048 PPR IO-Link setting**, the recomputed
measuring-chain error budget, the closed-loop stepper step-response model,
the E-stop dynamics and the payoff/feed spool-dynamics analysis. Every
number derives from the repository constants
([measurement_spec](../shared/measurement_spec.hpp),
[machine_spec](../shared/machine_spec.hpp),
[controls_spec](../shared/controls_spec.hpp),
[feed_drive_spec](../shared/feed_drive_spec.hpp),
[cutting_head_spec](../shared/cutting_head_spec.hpp),
[mechanism_spec](../shared/mechanism_spec.hpp)) plus the archived datasheets
named per section. Where a manufacturer publishes no value, the assumption
is stated with its source and the reason. No physical machine exists;
nothing here releases fabrication or energisation, and no gate is closed
by this document alone.

## 1. Encoder 2048 PPR setting and the recomputed error budget

### 1.1 The setting and its verification

The ifm RU3100 datasheet ([archived](../engineering/sources/ifm-ru3100-datasheet.pdf),
`ifm-ru3100-datasheet-en.txt`) states the resolution is **programmable
1…10000 PPR via IO-Link** with a 1024 PPR factory setting; 2048 PPR is
therefore a supported setting, not an extrapolation. The setting step is
recorded in [docs/13 §5 step 9](../docs/13-measuring-interfaces-and-calibration.md):
with the same IO-Link master used for the TTL-mode parameterisation, set the
resolution parameter to **2048 PPR**, verify by IO-Link readback, and record
the setting with the module identifier before any calibration capture.
This halves the count spacing with no mechanical change.

| Quantity | 1024 PPR (factory, R1) | **2048 PPR (R2 setting)** | Basis |
|---|---|---|---|
| Quadrature counts/rev (x4) | 4096 | **8192** | `machine_spec.hpp encoder_counts_rev` |
| Count spacing | was 0.048828 mm/count | **0.0244140625 mm/count** | 200 mm / 8192 |
| Nominal nm/count | was 48828 | **24414** (24414.0625) | `machine_spec.hpp nominal_encoder_nm_count` |
| Nominal counts per metre | was 20480 | **40960** | `controls_spec.hpp nominal_counts_per_metre` |
| Half-count quantisation | ±0.0244 mm | **±0.0122 mm** | spacing/2 |
| A/B cycles/s at 500 mm/s (199.8 mm min circumference) | 2562.5626 | **5125.1251** | 150.1502 rpm |
| Quadrature edges/s at 500 mm/s | 10250.2503 | **20500.5005** | x4 decode |

The doubled edge rate stays far inside the 1000 kHz A/B-frequency rating
(the check `encoder_A_B_frequency` passes at 5125 Hz) and the MCU-side
capture is a separate, already-gated commissioning item.

### 1.2 Full error-budget table (docs/37 numbers, same allocations, 2048 PPR)

Target: ±(0.2 mm + 0.05 %) of commanded length = **±0.30 mm at 200 mm**,
**±0.70 mm at 1000 mm** ([docs/20](../docs/20-physical-qualification.md)).

| Term | Value at 1 m | Change from R1 |
|---|---|---|
| Wheel factory circumference | ±1.0 mm | unchanged — assumed ±0.2 mm on 200 mm; ifm publishes no tolerance; calibration replaces it |
| Calibration procedure | +0.025 mm + 0.01 % (=0.125 mm at 1 m) | unchanged — chosen metrology allocation, 32×256-count phase map, held-out verification |
| Quantisation | ±0.0122 mm (0.025 mm allocation) | **reduced**: half-count halves 0.0244→0.0122 mm; the allocation policy "2× half-count" then halves 0.05→0.025 mm — see 1.3 |
| Encoder edge jitter | ±0.02 mm | allocation unchanged — computed per-edge jitter stays ≈0.0008 mm: edge spacing halves while the edge rate doubles, so the stated ≤±1° electrical + ±1 µs timing contribution is rate-invariant; non-accumulating |
| Wheel slip at 6–19 N force | 0.25 mm allocation | unchanged — retained qualification allocation, not measured slip |
| Temperature 15–30 °C | ≈0.23 mm | unchanged — Al core 23e-6/K × 10 K; PU tread/indentation unquantified (flagged) |
| Runout/TIR 0.2 mm | 0.2 mm uncalibrated | unchanged — calibration maps the first harmonic (synthetic bound 0.001 mm); arbitrary lobing does not inherit that bound |
| Wire tension/extension | 0.15 mm allocation | unchanged — recipe-controlled term |

Worst-case sums:

| Configuration | 200 mm | 1000 mm | vs target | Verdict |
|---|---|---|---|---|
| Uncalibrated (factory scale + allocations) | 0.825 mm | **1.625 mm** (R1: 1.65) | 0.30 / 0.70 | **FAIL** (2.3× over at 1 m) |
| Calibrated (calibration replaces factory scale; allocations retained) | 0.524 mm | **0.788 mm ≈ 0.79 mm** (R1: 0.80) | 0.30 / 0.70 | **FAIL at the retained allocations** — PASS requires the slip/tension/temperature terms to be demonstrated, not allocated |

Calibrated arithmetic: calibration 0.125 + quantisation half-count 0.0122 +
jitter 0.02 + slip 0.25 + temperature 0.23 + mapped-runout 0.001 + tension
0.15 = **0.788 mm at 1 m**; at 200 mm: 0.045 + 0.0122 + 0.02 + 0.25 + 0.046
+ 0.001 + 0.15 = **0.524 mm**.

### 1.3 Defensible reduction and the verdict

The quantisation allocation is reduced **0.05 → 0.025 mm** because the
allocation was defined as twice the half-count (`measurement_analysis.cpp`
`nominal_count_spacing` check), and the 2048 PPR setting halves the
half-count to 0.0122 mm. This is the only allocation the setting can
defensibly reduce: the calibration granularity does not improve further
because the fit grid stays 32 intervals (256 counts each — interpolation
bound unchanged), and the slip/tension/temperature allocations are process
terms, not quantisation. The new calibrated total **0.79 mm/m** still FAILs
the ±0.70 mm/m acceptance by ≈0.09 mm: quantisation is not the binding term
(slip 0.25 + tension 0.15 + temperature 0.23 = 0.63 mm/m), so the 2048 PPR
setting buys −0.012 mm/m and no verdict change. The short-length target
(±0.30 mm at 200 mm) remains dominated by the constant allocations.
**Verdict:** desk FAIL retained; the improvement is real but marginal, and
the physical demonstrations (gates 25, 29) are unchanged. The regenerated
[measurement-analysis](../engineering/generated/measurement-analysis/report.md)
artifacts carry the 0.025 mm quantisation allocation and 8192-count phase map.

## 2. Drive closed-loop step response

### 2.1 Parameters from the archived datasheets

| Axis | Motor | Driver | R / L | Kt (holding/I) | Reflected inertia |
|---|---|---|---|---|---|
| Feed | [24HS40-5004D-E1000](../engineering/sources/24HS40-5004D-E1000.pdf): 4.0 Nm @ 5.0 A/phase, 0.60 Ω, 2.60 mH, rotor 1080 g·cm² | [CL57T-V41](../engineering/sources/CL57T-V41.pdf) | 0.60 Ω / 2.60 mH → τ_e = 4.33 ms, electrical pole 36.7 Hz | 0.8 Nm/A | **4.35e-4 kg·m²** ([docs/37 §1.1](37-system-dynamics.md)) |
| Head | [34E1K-85](../engineering/sources/34E1K-85.pdf): 8.5 Nm @ 5.7 A, 0.54 Ω, 5.0 mH, rotor 3600 g·cm² | [CL86T-V41](../engineering/sources/CL86T-V41.pdf) | 0.54 Ω / 5.0 mH → τ_e = 9.26 ms, electrical pole 17.2 Hz | 1.49 Nm/A | ≈4.5e-4 kg·m² stated (rotor 3.6e-4 + coupling 1.6e-5 + SFU1605 screw ≈2.9e-5 + ≈12 kg carriage/holder/motor/adapter through the 10 mm lead ≈3.0e-5) |

Neither manual publishes a numeric position-loop gain, bandwidth or
following-error threshold. The CL57T/CL86T S1 tables give only relative
rigidity codes (position loop Kp 25/50/100, velocity loop Kp 25/15/5,
velocity loop Ki 0/16; factory code: Ki 0 — "stop time is long, position
error smaller", Kp 25/25 — "rigidity weak"). The model below therefore uses
stated assumptions with a sensitivity band, and commissioning verification
is required.

### 2.2 Model

Closed-loop stepper as a torque-source position servo with the PI current
loop closed inside the drive (the motor electrical pole at 36.7 Hz feed /
17.2 Hz head is cancelled by the current-loop integrator — stated assumption
consistent with the drives' "no tuning" vector-controlled closed-loop mode).
Position loop: second order, **ω_n = 2π × 25 Hz = 157 rad/s, ζ = 0.9**
(stated: consistent with the factory "weak rigidity, long stop, small
error" code; bounded below by the head electrical pole at 17.2 Hz and above
by the feed pole at 36.7 Hz — the loop crossover cannot robustly exceed the
motor electrical dynamics). Sensitivity band: 10–40 Hz.

### 2.3 Results

| Quantity | Feed (J = 4.35e-4) | Head (J ≈ 4.5e-4) |
|---|---|---|
| Overshoot (ζ = 0.9) | **0.15 %** | 0.15 % |
| Settling time to 2 % (4/ζω_n) | **28.3 ms** | 28.3 ms |
| Settling time to 5 % | 21.2 ms | 21.2 ms |
| Tracking lag during accel (a/ω_n²) | 1.5 m/s² → **0.061 mm** = 0.174° mechanical (0.97 microsteps at 2000 ppr; 8.7° electrical) | 0.3 m/s² → **0.0122 mm** = 0.44° mechanical (1.22 steps at 1000 ppr; 21.9° electrical) |
| Step demand at profile speed | 500 mm/s → **7958 Hz** | 12.5 mm/s → 1250 Hz |
| Step period at max speed | 125.7 µs | 800 µs |

Settling sensitivity: 70.7 ms (10 Hz) / 28.3 ms (25 Hz) / 17.7 ms (40 Hz);
tracking lag 0.38 / 0.061 / 0.024 mm across the same band.

### 2.4 Verdict vs the 500 mm/s motion profile

- The 7958 Hz step stream is 1.6 % of the drive's 500 kHz pulse-input limit
  and 63× below the 500 kHz (5 V) input rating — the drive input is not
  binding. The binding firmware envelope is `controls_spec max_step_hz =
  8000` against 7957.7 Hz demanded (**0.53 % margin, 42 Hz** — flagged: the
  envelope is software, not drive, limited).
- At 239 rpm the motor torque assumption band 2.6–3.1 Nm vs 1.03 Nm peak
  demand ([docs/37 §1.2](37-system-dynamics.md)) bounds the profile by
  torque, not by loop dynamics.
- The per-step transient never settles between steps at speed (125.7 µs ≪
  28.3 ms); the loop instead tracks continuously with the stated
  acceleration lag of 0.061 mm (0.17° mechanical) — far inside the drive's
  closed-loop operating range (electrical error 8.7° vs the ±90° torque
  reversal and the ±1.8°-class step; the following-error alarm threshold is
  not published and must be probed at commissioning).
- **Verdict:** PASS as a desk model with stated assumptions; because the
  manuals publish no gains or following-error threshold, a commissioning
  check (single-step response, accel-decel tracking error, alarm threshold)
  is mandatory before release — recorded against gate 23/24 physical work.
  The head axis at 12.5 mm/s / 1250 Hz is 6.4× slower and passes trivially.

## 3. E-stop dynamics

### 3.1 Drive-braked stop at the 2.5 SF torque limit

Worst case: 500 mm/s feed, deceleration torque bounded by the factored
envelope **2.58 Nm** (`feed_drive_spec.hpp reversal_service_factor = 2.5` on
the 1.03 Nm peak). The 2.58 Nm envelope is a design limit, not the drive's
electrical cap (≈4.48 Nm peak at the 5.6 A S1 peak-current setting ×
0.8 Nm/A), and it sits inside the assumed 2.6–3.1 Nm motor band at 239 rpm
(1.01–1.20× — the same marginality flagged in docs/37 §1.2); braking torque
capability is assumed equal to motoring (stated).

| Quantity | Value | Basis |
|---|---|---|
| Wire force at the roller | 2.58 Nm / 20 mm = **129 N** | r = 20 mm |
| Equivalent linear mass | J/r² = 4.35e-4/4e-4 = **1.0875 kg** | reflected inertia |
| Deceleration | **118.6 m/s² (12.1 g)** | F/m_eq |
| Stopping time | **4.2 ms** | v/a |
| Stopping distance | **1.05 mm** | v²/2a |

**Belt stress pulse:** the decel puts a 129 N tension difference across the
strands on top of the 190 N target initial tension → tight strand
**254.5 N**, slack strand **125.5 N**. The tight strand stays below the
480 N radial design load (`belt_radial_design_n`) and 2.58 Nm is 24 % of the
10.89 Nm belt capacity at 260 rpm (4.2×); the slack strand keeps a large
positive margin (the 5 Nm fault already passes at 22.9 N). The pulse lasts
≈4.2 ms ≈ 0.34 tooth-passes at 79.7 Hz tooth passing — a sub-cycle transient,
benign for the S5M. Regenerated energy ≈½·J·ω² = **0.14 J** (trivial for the
500 µF-class bus; the regen/bus-decay measurement stays gated, docs/22 §8.5).

### 3.2 Contactor-open phase and guard travel

Sequence: E-stop → XPS drops → K1–K3 contactors open → drives de-energised
(category 0 power removal, [docs/22 §11.8](../docs/22-controls-and-wiring.md)).
After the contactor opens there is **no drive torque**; the feed motor has no
holding brake (24HS40-5004D-E1000 has no brake field; the CL57T BRK output is
unused), so the wire coasts against the payoff friction brake (5–20 N) plus
≈1 N measured-drag budget (wheel bearing/encoder 0.31 N + guides; stated).

Coasting mass couples the full spool through the taut strand:
m_eq = 0.553 kg wire + J_spool/r² = 0.553 + 0.6/0.04 = **15.55 kg** (30 kg
full spool). Coast distance s = m_eq·v²/(2·F_ret):

| Retarding force (brake + drag) | Coast distance | Coast time |
|---|---|---|
| 5 N + 1 N = 6 N | **324 mm** | 1.30 s |
| 20 N + 1 N = 21 N | **93 mm** | 0.37 s |

Guard travel (stated): the wire route runs along X at Y=0, Z=112 inside the
guard inner half-width ±345 mm (`mechanism_spec.hpp guard_inner_half_x`); the
cut station sits at X ≈ 0 between the cassettes, so the travel available
before the +X panel aperture is **≈345 mm**. The blade carriages coast
≈0.011 mm from 12.5 mm/s (KK86 start-torque/friction limited, KE ≈ 1 mJ) —
orders of magnitude inside their 194 mm travel and the Y ±625 mm panels.

**Verdict:** the drive-braked stop (1.05 mm) is trivially inside the guard.
The contactor-open coast is the binding case: 93–324 mm vs ≈345 mm available
— it stays inside the guarded volume at every brake setting **if the ≈1 N
drag estimate holds**, but the 5 N floor leaves only ≈21 mm margin and the
wire end can nominally reach the exit aperture. **Recommendation:** set the
brake floor ≥7 N (guarantees ≤300 mm coast with no drag credit) or add a
mechanical wire stop; measure the actual coast at commissioning (gates 14,
16, 23, P5-4/P7-1…P7-4). No guard breach is desk-provable — the aperture
safety treatment remains gate 14.

## 4. Payoff/feed interaction

### 4.1 Full-spool acceleration and nip traction

Spool 30 kg full at r = 0.2 m → J = ½·m·r² = **0.6 kg·m²**. Feed profile
acceleration 1.5 m/s² → spool angular acceleration α = 7.5 rad/s² →
torque J·α = 4.5 Nm → **22.5 N wire tension** at the spool radius.

Total pull the nips must supply (worst case, full spool):

| Term | Force |
|---|---|
| Spool acceleration | 22.5 N |
| Payoff brake (window max) | 20 N |
| Wire mass acceleration (0.553 kg × 1.5 m/s²) | 0.83 N |
| Route wire mass (0.88 m × 0.1425 kg/m × 1.5 m/s²) | 0.19 N |
| Measuring wheel + guide drag | ≈0.46 N |
| **Total** | **≈44 N (29 N at the 5 N brake floor)** |

Nip traction capacity: 2·μ·N per nip, two nips → **80 N at μ = 0.4** (180 N
at μ = 0.6, [docs/37 §5](37-system-dynamics.md)). Margin **1.8×** at the
brake maximum / 2.8× at the floor — the feed accelerates the full spool from
rest at 1.5 m/s² **without nip slip**. Nip load sharing is a stated
linear-sum assumption (pull-test required, gate 12).

### 4.2 Measuring-wheel traction (Hertzian + friction bound)

The wheel rides on the wire at 19 N max contact force
(`wheel_force_max_n`), driven by the wire. Demand on the wheel-wire contact:
bearing/encoder drag 0.31 N + wheel acceleration J_w·α/r = 0.05 N + 0.10 N
accel ≈ **0.46 N**.

Hertzian line contact, wire Ø4.5 mm (R = 2.25 mm) on the 9.5 mm PU tread,
E_pu ≈ 40 MPa (90 Shore A PU, stated band 25–60 MPa), ν = 0.49, Cu
E = 117 GPa:

| Quantity | Value |
|---|---|
| Effective modulus E* = 1/((1−ν²)/E_pu + (1−ν_cu²)/E_cu) | **52.6 MPa** |
| Contact half-width b = √(4·F·R/(π·E*·L)) | **0.33 mm** |
| Peak pressure p_max = 2F/(π·b·L) | **3.86 MPa** |
| Mean pressure | 3.03 MPa |

p_max ≈ 3.9 MPa is far below the 90 Shore A PU compressive strength
(30–80 MPa class) — elastic rolling contact, no indentation ploughing, so
the traction stays a Coulomb bound. μ (90 Shore A PU on Cu, dry) stated
**0.4–0.6**, central 0.5 (tribology range, consistent with docs/37 §5):
friction capacity μ·N = **7.6–11.4 N (9.5 N central)** vs 0.46 N demand →
**16–25× margin**; required μ = 0.46/19 = 0.024 ≪ 0.4. The wheel cannot slip
even at the minimum force (6 N: 2.4–3.6 N capacity, still ≥5×). The wire
tension (up to ≈44 N during spool acceleration) passes through the wheel as
an along-wire load and does not consume this traction bound.

### 4.3 Brake setting table

Hard bound: brake tension + spool accel force + wire accel + strip-force
budget ≤ nip traction at μ = 0.4 (80 N), with a 20 N strip budget retained:

T_brake,max = r·(80 N − J·α/r − 0.83 N − 20 N), r = 0.2 m, α = 7.5 rad/s².

| Full spool mass m (kg) | J = ½·m·r² (kg·m²) | Spool accel force at wire (N) | **Max allowed brake torque (Nm)** | Max allowed brake tension (N) |
|---|---|---|---|---|
| 6.5 (empty spool) | 0.13 | 4.9 | **10.9** | 54 |
| 10 | 0.20 | 7.5 | **10.3** | 52 |
| 15 | 0.30 | 11.3 | **9.6** | 48 |
| 20 | 0.40 | 15.0 | **8.8** | 44 |
| 25 | 0.50 | 18.8 | **8.1** | 40 |
| 30 (maximum) | 0.60 | 22.5 | **7.3** | 37 |

Operating guidance remains the docs/37 window **5–20 N back-tension =
1.0–4.0 Nm** at r = 0.2 m — never binding against the table (minimum hard
bound 7.3 Nm at full spool), and the §3.2 E-stop coast recommends the floor
≥7 N (1.4 Nm) for guard travel. Above the table, nip slip at μ = 0.4
becomes possible; at μ = 0.6 the bound doubles. Verify by the pull/creep
test in the physical qualification (gate 12, P6-4…P6-6) and weigh the spool
(gate 16).

## 5. Assumption register (additions on top of docs/37 §7)

1. RU3100 2048 PPR setting is within the datasheet's 1…10000 programmable
   range; IO-Link readback verification is part of the commissioning step.
2. Position-loop model: ω_n = 2π × 25 Hz, ζ = 0.9 (sensitivity 10–40 Hz
   band stated); the CL57T/CL86T manuals publish no numeric gains,
   bandwidth or following-error threshold.
3. Current loop closes inside the drive with bandwidth ≥5× the position
   loop (stated), cancelling the 36.7 Hz feed / 17.2 Hz head electrical
   poles.
4. Head reflected inertia ≈4.5e-4 kg·m² is a stated estimate (carriage
   stack ≈12 kg; screw inertia geometric).
5. Braking torque capability equals motoring capability within the
   2.6–3.1 Nm band at 239 rpm (stated).
6. Coast retarding force = payoff brake 5–20 N + ≈1 N measured drag
   (stated budget); the strand stays taut so the full spool inertia
   couples during coast.
7. Guard travel for the feed coast = cut station (X ≈ 0) to the +X guard
   panel aperture ≈345 mm (mechanism geometry, stated).
8. μ (90 Shore A PU on Cu, dry) = 0.4–0.6, central 0.5; E_pu = 40 MPa
   (25–60 MPa band), ν = 0.49 — stated tribology/material values.
9. Strip-force budget 20 N retained in the brake table (docs/37 leaves
   50–100 N headroom without spool acceleration).
10. Two-nip traction sums linearly; nip load sharing must be verified.

## 6. Verdict summary

| Item | Key number | Verdict |
|---|---|---|
| Encoder setting | 2048 PPR via IO-Link: 0.0244140625 mm/count, 8192 counts/rev, 40960 counts/m | Setting documented (docs/13 §5 step 9); constants, tests and artifacts updated |
| Error budget (R2) | 1.625 mm/m uncalibrated; **0.79 mm/m calibrated-at-allocations vs ±0.70 mm/m**; quantisation allocation 0.05→0.025 mm | Still FAIL on desk (−0.012 mm/m gain); process terms (slip 0.25 + tension 0.15 + temp 0.23) bind — demonstrated physically, not re-budgeted |
| Drive step response | settling ≈28.3 ms (2 %), overshoot ≈0.15 %, tracking lag 0.061 mm at 1.5 m/s²; 7958 Hz demand vs 500 kHz input | PASS with stated 25 Hz/ζ 0.9 assumptions; commissioning verification required (unpublished gains); PIO 8000 Hz cap flagged at 0.53 % margin |
| E-stop | 2.58 Nm → 129 N → 1.05 mm / 4.2 ms (12.1 g); belt tight strand 254.5 N vs 480 N radial design; contactor-open coast 93–324 mm vs ≈345 mm guard travel | PASS with brake floor ≥7 N recommendation and commissioning coast measurement; blade axes trivial |
| Payoff/feed | spool accel demand 22.5 N; total pull ≤44 N vs 80 N nip minimum (1.8×); wheel Hertzian p_max 3.86 MPa, μ·N 7.6–11.4 N vs 0.46 N (16–25×); brake hard bound 7.3–10.9 Nm by spool mass, guidance 1.0–4.0 Nm | PASS as desk model; physical pull/runaway tests unchanged (gates 12, 16) |

No repository gate is closed by this document alone; the desk halves it
supplies are recorded in the [gate ledger](../docs/33-gate-ledger.md) §2.7.
