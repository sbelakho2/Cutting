# 42 — Process dynamics and error budget, R2 — S1-B

**Revision:** `S1-B-DYN-R2-2026-09-17`
**Corrections 2026-10-13 (audit F19/F20/F21/F26):** §1.4 added (tension term
cannot close the worst case); §4 rewritten for the driven-contact traction
bound, the full spool mass/radius range, the radius-band brake table and the
selected buffered payoff with reversal slack handling; §3.1 regen reference
to docs/22 §11.2 corrected.

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
| Wire tension/extension | 0.15 mm allocation **conditional on \|ΔT\| ≤ 8.25 N** | corrected — the 44 N worst-case tension gives 0.8 mm/m on 0.5 mm² Cu; see §1.4 |

Worst-case sums:

| Configuration | 200 mm | 1000 mm | vs target | Verdict |
|---|---|---|---|---|
| Uncalibrated (factory scale + allocations) | 0.825 mm | **1.625 mm** (R1: 1.65) | 0.30 / 0.70 | **FAIL** (2.3× over at 1 m) |
| Calibrated (calibration replaces factory scale; allocations retained) | 0.524 mm | **0.788 mm ≈ 0.79 mm** (R1: 0.80) | 0.30 / 0.70 | **FAIL at the retained allocations** — PASS requires the slip/tension/temperature terms to be demonstrated, not allocated |

Calibrated arithmetic: calibration 0.125 + quantisation half-count 0.0122 +
jitter 0.02 + slip 0.25 + temperature 0.23 + mapped-runout 0.001 + tension
0.15 = **0.788 mm at 1 m**; at 200 mm: 0.045 + 0.0122 + 0.02 + 0.25 + 0.046
+ 0.001 + 0.15 = **0.524 mm**. The 0.79 mm is a **floor conditional on
\|ΔT\| ≤ 8.25 N** between calibration and measurement (0.5 mm² Cu, §1.4): it
cannot be claimed as the worst case.

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
The tension allocation above is additionally conditional — §1.4.

### 1.4 The tension term cannot close the worst case (correction)

The corrected payoff/feed analysis (§4) removes the "recipe-controlled
constant" assumption from the tension term:

- The full-spool acceleration case demands ≈44 N at the nips (§4.1), and the
  passive-brake tension scales as T_brake/r, reaching tens of newtons as the
  spool empties (§4.3).
- The elastic extension of the measured span is ΔL = T·L/(E·A). For the
  smallest specified conductor, 0.5 mm² Cu with E = 110 GPa, this is
  **0.8 mm on 1 m at 44 N** — 0.0182 mm/N/m. The 0.15 mm tension allocation
  is consumed by only **8.25 N** of tension variation.
- Therefore the calibrated-at-allocations 0.79 mm/m is a floor that assumes
  \|ΔT\| ≤ 8.25 N; a 44 N tension excursion adds ≈(44 − 8.25)·0.0182 ≈
  **0.65 mm/m**, putting even a calibrated machine past 1.4 mm/m against the
  ±0.70 mm/m acceptance.
- The allocation cannot be booked as a bound until tension control is
  demonstrated (buffered or radius-compensated payoff, §4.4) and the
  metrology closes the remaining terms. The acceptance requirement stays
  **±(0.20 mm + 0.05 %)** and the budget status stays **FAIL**.

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
≈4.2 ms ≈ 0.33 tooth-passes at the corrected 79.6 Hz tooth-passing — a
sub-cycle transient, benign for the S5M. Regenerated energy with the full
reflected inertia (J = 4.35e-4 kg·m², wire included) is ≈½·J·ω² =
**0.14 J**; docs/22 §11.2 records the stop-energy partition (≤0.1 J with the
rotor-only basis) against ≥0.18 J bus storage — the two use different inertia
bases and the margin is not "trivial" (≈1.3× on the conservative basis), so
the regen/bus-decay measurement stays gated there (docs/22 §11.2, §10).

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

### 4.1 Full-spool acceleration and the driven-contact traction bound

Spool 30 kg full at r = 0.2 m → J ≈ 0.6 kg·m² (the component model is in
§4.3: flanges 0.1184 kg·m² + wound wire). Feed profile acceleration
1.5 m/s² → spool angular acceleration α = 7.5 rad/s² → torque J·α = 4.5 Nm →
**22.5 N wire tension** at the spool radius.

Total pull the nips must supply (worst case, full spool, no buffer):

| Term | Force |
|---|---|
| Spool acceleration | 22.5 N |
| Payoff brake (window max) | 20 N |
| Wire mass acceleration (0.553 kg × 1.5 m/s²) | 0.83 N |
| Route wire mass (0.88 m × 0.1425 kg/m × 1.5 m/s²) | 0.19 N |
| Measuring wheel + guide drag | ≈0.46 N |
| **Total** | **≈44 N (29 N at the 5 N brake floor)** |

**Corrected traction model (replaces the doubled 2·μ·N model).** Only the
driven lower rollers transmit tangential force; each upper roller is a
reaction/drag element and contributes no extra tractive capacity. With the
two driven contacts loaded at N1 + N2, the capacity is **μ·(N1 + N2)**:

| Nip preload (each) | N1 + N2 | μ = 0.4 | μ = 0.6 |
|---|---|---|---|
| 50 N | 100 N | **40 N** | 60 N |
| 60 N | 120 N | **48 N** | 72 N |
| 75 N | 150 N | **60 N** | 90 N |

These are pre-loss bounds (friction and load-sharing losses not included).
The 44 N full-spool demand **exceeds the 40 N minimum-preload bound** and is
92 % of the 48 N target-preload bound: the previously claimed 80 N capacity
and 1.8× margin counted both faces of each nip and are withdrawn.
Consequences:

- a 50 N-preload recipe cannot start the 30 kg spool at 1.5 m/s²;
- a 60 N recipe consumes 92 % of the pre-loss bound — no loss margin;
- either the preload recipe is set to the 75 N window top with a verified
  pull test (60 N before ≤10 % loss = 54 N, 1.23×) and the strip-force
  budget re-based, or the §4.4 design change (buffered payoff) removes the
  spool-inertia term entirely (demand then 6.5–21.5 N, ≥1.8× even at the
  50 N floor).

Nip load sharing remains a stated linear-sum assumption (pull-test required,
gate 12).

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
tension (up to ≈44 N during an unbuffered full-spool acceleration, §4.1)
passes through the wheel as an along-wire load and does not consume this
traction bound — it does drive the tension/extension length term (§1.4),
which is why the buffered payoff is selected (§4.4).

### 4.3 Full mass/radius range and the radius-dependent brake setting

The spool inertia is not one number: the flanges alone are **0.1184 kg·m²**
(stated R2 spool geometry), and the wound wire adds ½·m_w(r)·r² with
m_w(r) = 23.5 kg × (r² − r_c²)/(0.2² − r_c²), r_c = 0.03 m (Ø60 hub). The
inertial tension demand at the nips is **J(r)·a/r²** at a = 1.5 m/s², and a
friction brake at torque T produces wire tension **T/r**:

| Spool state | Wire radius r (m) | J ≈ flanges + wire (kg·m²) | Inertial demand J·a/r² at 1.5 m/s² | 5–20 N brake torque band T = T_wire·r (Nm) |
|---|---|---|---|---|
| Full 30 kg | 0.20 | 0.59 | **22.0 N** | 1.00–4.00 |
| — | 0.15 | 0.26 | **17.6 N** | 0.75–3.00 |
| — | 0.10 | 0.15 | **21.9 N** | 0.50–2.00 |
| — | 0.07 | 0.124 | **38.0 N** | 0.35–1.40 |
| — | 0.05 | 0.120 | **72.0 N** | 0.25–1.00 |
| Empty (flanges only) | 0.03 (core) | **0.1184** | **197.3 N** | 0.15–0.60 |

The flanges-only case is the worst: **197.3 N at the 0.03 m core** (and even
at 0.07 m the wound wire plus flanges demand 38 N). A **1 Nm brake at the
core is 33.3 N** of tension — above the whole 5–20 N guidance and most of
the corrected 40–60 N traction bound. The docs/37 guidance "5–20 N =
1.0–4.0 Nm" is therefore valid **only at the 0.2 m full radius**; a passive
brake must be set by radius band:

| Radius band | Nominal radius | Brake torque for 5–20 N tension |
|---|---|---|
| 0.16–0.20 m (full) | 0.20 m | 1.00–4.00 Nm |
| 0.12–0.16 m | 0.15 m | 0.75–3.00 Nm |
| 0.08–0.12 m | 0.10 m | 0.50–2.00 Nm |
| 0.06–0.08 m | 0.07 m | 0.35–1.40 Nm |
| 0.04–0.06 m | 0.05 m | 0.25–1.00 Nm |
| ≤0.04 m (core) | 0.03 m | 0.15–0.60 Nm |

Passive-mode acceleration limit: the nips still have to accelerate the spool
inertia, so the 1.5 m/s² profile survives only while J(r)·a/r² ≤ the
traction bound — with 50 N preload (40 N) down to
r = √(0.1184×1.5/40) = **0.067 m**, with 75 N preload (60 N) to **0.054 m**.
Below that the profile must fall to a ≤0.30–0.46 m/s² core-radius recipe or
the payoff must be buffered (§4.4). The §3.2 E-stop coast floor (≥7 N
≈ 1.4 Nm at r = 0.2 m) is unaffected at the full radius but must be re-checked
against the radius-band setting as the spool empties. Verify by the
pull/creep test in the physical qualification (gate 12, P6-4…P6-6) and weigh
the spool (gate 16).

### 4.4 Resolution: buffered/powered payoff (selected), slack handling

Full-range verdict: a fixed-torque passive friction brake cannot serve this
spool range. Tension = T_brake/r varies 6.7× over 0.03–0.20 m; the
empty-spool core case (197 N inertial demand for the flanges alone at
1.5 m/s²) is an order of magnitude beyond the corrected 40–60 N nip
traction, so no brake setting lets the empty spool follow the strand.

| Option | Evaluation | Selected |
|---|---|---|
| Radius-band table + core-radius acceleration recipe | implementable on the existing brake, but the 1.5 m/s² profile must fall to ≤0.30–0.46 m/s² below r ≈ 0.054–0.067 m, changing the cycle; a friction brake also cannot take back wire on reversal | interim only |
| Lighter spool (lighter flanges) | reduces but does not close: even at J = 0.03 kg·m² the core demand is 0.03×1.5/0.0009 = **50 N**, above the 40 N minimum bound; a purchased-part change with its own qualification | no |
| **Buffered/powered payoff (dancer/accumulator)** | removes the J·a/r² term from the nips (demand 6.5–21.5 N, ≥1.8× at the 50 N floor), holds tension in the 5–20 N window independent of radius, and takes up reversal slack | **yes** |

**Selected resolution: buffered/powered payoff.** It is the only option that
closes the core-radius physics rather than de-rating the machine. Required
design work (open): select/design the dancer with sufficient stroke for the
230 mm ejection move and the reversal slack at 1.5 m/s²; verify its spring
and roller dynamics against the demand; re-run the §3.2 E-stop coast with
the buffer in the wire path; and re-qualify the tension term (§1.4). The
existing friction brake is retained only as a low-torque runaway/overrun
drag. Until the buffer exists, the passive-mode radius-band table and
core-radius acceleration limit above are **not a released operating mode**.

**Slack handling during feed reversal.** Every reversal (25 mm tail
backfeed, strip positioning, recovery moves) pushes wire back toward the
payoff; a passive friction brake cannot take it back, so the strand goes
slack, can loop/tangle, and snap-loads on the next payout. A take-up element
is **required** — the selected buffer/dancer provides it. Until then,
reversal moves must be length-limited and slack visually checked at
commissioning (docs/26 §6.3).

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
9. Strip-force budget 20 N retained against the corrected driven-contact
   bound (docs/37 leaves 50–100 N headroom without spool acceleration).
10. Driven-contact traction model: only the driven lower rollers transmit
    (capacity μ·(N1 + N2)); upper rollers are reaction/drag. Nip load
    sharing is a linear-sum assumption and must be verified (gate 12). The
    R1 doubled model (2·μ·N per nip, 80–180 N) is withdrawn.
11. Spool flange inertia 0.1184 kg·m² and the annular wire-winding model of
    §4.3 are stated geometry; the empty-spool core case (197 N) is the
    governing worst case.
12. Buffered/powered payoff (dancer/accumulator) is the selected design
    change and required work (§4.4); the passive radius-band table and
    core-radius acceleration limit are interim limits only.
13. Reversal slack take-up is required; a passive friction brake cannot take
    back wire during feed reversal.

## 6. Verdict summary

| Item | Key number | Verdict |
|---|---|---|
| Encoder setting | 2048 PPR via IO-Link: 0.0244140625 mm/count, 8192 counts/rev, 40960 counts/m | Setting documented (docs/13 §5 step 9); constants, tests and artifacts updated |
| Error budget (R2) | 1.625 mm/m uncalibrated; **0.79 mm/m calibrated-at-allocations vs ±0.70 mm/m**, itself a floor conditional on \|ΔT\| ≤ 8.25 N; 44 N tension alone is 0.8 mm/m on 0.5 mm² | Still FAIL on desk; process terms (slip 0.25 + tension ≤0.15 conditional + temp 0.23) bind — tension control and metrology must be demonstrated physically, requirement stays ±(0.20+0.05 %) |
| Drive step response | settling ≈28.3 ms (2 %), overshoot ≈0.15 %, tracking lag 0.061 mm at 1.5 m/s²; 7958 Hz demand vs 500 kHz input | PASS with stated 25 Hz/ζ 0.9 assumptions; commissioning verification required (unpublished gains); PIO 8000 Hz cap flagged at 0.53 % margin |
| E-stop | 2.58 Nm → 129 N → 1.05 mm / 4.2 ms (12.1 g); belt tight strand 254.5 N vs 480 N radial design; contactor-open coast 93–324 mm vs ≈345 mm guard travel | PASS with brake floor ≥7 N recommendation and commissioning coast measurement; blade axes trivial; §3.2 must be re-run with the buffer in the wire path |
| Payoff/feed | driven-contact traction **40/48/60 N** at 50/60/75 N preload (μ = 0.4 pre-loss); 44 N full-spool demand exceeds the minimum-preload bound (92 % of the 60 N bound); empty flanges 0.1184 kg·m² → 197 N at the 0.03 m core; 1 Nm brake = 33.3 N at the core; wheel p_max 3.86 MPa, μ·N 7.6–11.4 N vs 0.46 N | **Passive mode FAIL/conditional** — buffered payoff selected and required (slack take-up included); interim radius-band table only; physical pull/runaway tests unchanged (gates 12, 16) |

No repository gate is closed by this document alone; the desk halves it
supplies are recorded in the [gate ledger](../docs/33-gate-ledger.md) §2.7.
