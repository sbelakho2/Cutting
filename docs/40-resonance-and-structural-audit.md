# 40 — Resonance and structural audit — S1-B

**Revision:** `S1-B-RES-R1-2026-09-17`
**Status:** desk calculation record. Every number below is derived from the
repository constants ([mechanism_spec](../shared/mechanism_spec.hpp),
[feed_drive_spec](../shared/feed_drive_spec.hpp),
[cutting_head_spec](../shared/cutting_head_spec.hpp),
[machine_spec](../shared/machine_spec.hpp),
[measurement_spec](../shared/measurement_spec.hpp)), the archived datasheets
named per section and the existing records ([docs/37](37-system-dynamics.md),
[docs/39](39-custom-cut-axis.md), [docs/15](15-feed-shaft-stress-model.md),
[docs/28](28-physical-contradiction-audit.md),
[feed-analysis](../engineering/generated/feed-analysis/report.md),
[feed-fem](../engineering/generated/feed-fem/report.md)). Where a
manufacturer publishes no value, the assumption is stated. No physical
machine exists; nothing here releases fabrication or energisation.

## 1. Excitation inventory — every periodic source at the operating envelope

Feed envelope: 500 mm/s at a 20 mm roller radius → ω = v/r = 25 rad/s =
**3.979 rev/s = 238.7 rpm** ([docs/37](37-system-dynamics.md): 239 rpm).
Blade envelope: 12.5 mm/s at the 5 mm SFU1605 lead
([docs/39](39-custom-cut-axis.md)) → **2.5 rev/s = 150 rpm**.

| # | Source | Frequency content at the envelope | Basis |
|---|---|---|---|
| 1 | Feed belt tooth-pass (S5M, 5 mm pitch) | **100 Hz** fundamental; trapezoid harmonics **200 / 300 Hz** | 500 mm/s ÷ 5 mm ([docs/37](37-system-dynamics.md) §2) |
| 2 | Feed motor 1× unbalance/runout | **3.98 Hz** | roller speed, 1× rev |
| 3 | Feed motor commutation (electrical) fundamental | **199 Hz** (= 50 × 3.979) | [24HS40-5004D-E1000](../engineering/sources/24HS40-5004D-E1000.pdf): step angle **1.80°** = 200 full steps/rev = 50 pole pairs → 50 electrical cycles/rev |
| 4 | Feed motor detent/cogging torque ripple | **796 Hz** (= 200 × 3.979) | Same datasheet: detent torque 1600 g·cm; a 2-phase hybrid stepper detent ripples at 4× the electrical frequency = 200 cycles/rev |
| 5 | Feed step-pulse train | **7.96 kHz** (= 2000 × 3.979); 8.67 kHz at the 260 rpm ceiling | CL57T set to 2000 pulses/rev ([catalogue-evidence](../engineering/sources/catalogue-evidence.md) line 161; the archived [CL57T-V41](../engineering/sources/CL57T-V41.pdf) table is indexed "for 1.8° motor"); matches [docs/01](01-design-basis.md) 8666.7 Hz |
| 6 | Blade screw rotation | **2.5 Hz** 1× | 12.5 mm/s ÷ 5 mm lead ([docs/39](39-custom-cut-axis.md)) |
| 7 | Blade screw ball-pass | **≈75–100 Hz** (30–40 balls × 2.5 rev/s) | Nut ball count not published for the SFU1605 offer; stated assumption. 10 mm-lead alternative halves these |
| 8 | Blade motor (34E1K-85) commutation / detent | **125 Hz** electrical (50 × 2.5); **500 Hz** detent (200 × 2.5) | 1.8° NEMA 34 class — the archived [CL86T-V41](../engineering/sources/CL86T-V41.pdf) microstep table is indexed "for 1.8° motor"; rotor inertia 3600 g·cm² per [catalogue-evidence](../engineering/sources/catalogue-evidence.md) line 92 |
| 9 | Blade command / encoder rates | **1250 Hz** command (100 pulses/mm × 12.5 mm/s); 2.5 kHz encoder A/B | [docs/01](01-design-basis.md); [cutting_head_spec](../shared/cutting_head_spec.hpp) |
| 10 | Driver PWM chopping | **not published** in the archived CL57T-V41/CL86T-V41 datasheets (verified by full-text extraction — no PWM/chopper line); **20–50 kHz** is the stated class band for these drives | CL57T manual lists anti-resonance and 500 kHz max pulse input, but no switching frequency |
| 11 | Mains | **50 Hz** + **100 Hz** (2× magnetostriction) | 230 V 50 Hz supplies/cooler — all in the separate P9 cabinet; no 50 Hz device sits on the frame |
| 12 | Measuring wheel / spool | **2.5 Hz** wheel 1× (150 rpm at 500 mm/s, [docs/13](13-measuring-interfaces-and-calibration.md)); **0.40 Hz** spool 1× (Ø400, 23.9 rpm); encoder A/B edge rate ≈10–20 kHz (electrical only, PPR-dependent) | ifm E60137 + payoff Ø400 spool ([docs/26](26-guard-and-payoff-selection.md)) |
| 13 | Blade-screw whirl | 150 rpm vs ≈2600 rpm critical ([docs/39](39-custom-cut-axis.md) §4) | 17.3× margin — not an excitation risk |

**Correction to the working brief:** the feed "25 rev/s" is 25 **rad/s**;
25 rev/s would be 6.28 m/s, 12.6× the 0.5 m/s ceiling. At the true
3.979 rev/s the 50×rev (electrical) line is **199 Hz** and the step train is
**7.96 kHz** (the brief's 50 kHz pulse rate would require the impossible
25 rev/s). Both the 50×rev and 4×-electrical (200×rev) ripple lines are
tabulated above at the corrected speed.

## 2. Structural natural frequencies (formulas stated)

Stated masses: the frame top carries ≈150 kg (bed 48.4 + H001 10.1 + H002
risers 4.2 + 2 cut axes ≈36 + 2 feed cassettes ≈22 + measuring ≈2.5 + guard
≈36 kg; geometric/component estimates).

### 2.1 Welded frame — lateral sway of the 60×60×3 columns

Tube: I = (60⁴−54⁴)/12 = 3.714e-7 m⁴, A = 684 mm², EI = 7.80e4 N·m²
(E = 210 GPa, S235JR). Sway model: four legs in parallel, mass m on top,
**f = (1/2π)·√(k/m)**:

- Fixed–fixed columns (welded top joint + anchored foot): k = 4·12EI/L³ =
  8.53e6 N/m (L = 760 mm) → **38.0 Hz**.
- Cantilever bound (pinned top): k = 4·3EI/L³ = 2.13e6 N/m → **19.0 Hz**.

Band **19–38 Hz**; the welded joints put the real mode at the upper end.

### 2.2 Top rail/bed composite bending (long rails L1080, span 1.02 m between legs)

Simply-supported distributed-mass beam, **f₁ = (π/2L²)·√(EI/μ)**:
EI = 2 rails × 7.80e4 + bed share 5.83e4 (0.64 m × 0.025³/12 × 70 GPa) =
2.14e5 N·m²; μ = (150/2)/1.02 = 73.5 kg/m → **81.5 Hz**; welded-corner
fixed-end correction ×1.506 → **122.8 Hz**. Band **82–123 Hz**.

### 2.3 Bed plate first bending mode (6082, 640×1120×25, rails at X = ±250)

Plate strip across the 500 mm rail span, **f = (π/2L²)·√(D/(ρh))** with
D = Eh³/(12(1−ν²)) = 1.023e5 N·m (E = 70 GPa, ν = 0.33, ρ = 2700 kg/m³):
**244.6 Hz** simply supported, **368 Hz** with clamped rail lines. The
long-span strip between the cross rails (1.02 m) would give 58.8 Hz but is
subsumed by §2.2 because the long rails support the bed continuously.

### 2.4 F101 feed shaft first bending modes (Rayleigh/Dunkerley SDOF reduction)

- Roller overhang: cantilever k = 3EI/L³ (Ø10 end, L = 26 mm from front
  support S77.5 to roller mid S103.5; EI = 103.1 N·m²) = 1.76e7 N/m; mass =
  0.14 kg roller + 0.236 × 0.022 kg cantilever shaft mass (Dunkerley modal
  mass) = 0.145 kg → **f ≈ 1.75 kHz**.
- Pulley mid-span: k = 48EI/L³ (Ø14 seat, L = 51 mm between S26.5/S77.5;
  EI = 396 N·m²) = 1.43e8 N/m; 77 g pulley + 0.486 × shaft mass → **≈5.8 kHz**.

### 2.5 Custom-axis HGR20 rail/plate assembly (H005, 20×86×440 on the 355 mm riser)

**f = (π/2L²)·√(EI/μ)** over the riser span: EI = 4013 (6082 plate) + 2021
(2 × HGR20 steel rails) = 6034 N·m²; μ = 7.58 kg/m → **351.6 Hz** simply
supported, **529.6 Hz** clamped. With the ≈16 kg carriage+head payload at
mid-travel (SS modal mass μL/2 = 1.35 kg): f' = f/√(1 + M/(μL/2)) =
**97.9 Hz**. Band **≈98–352 Hz**, position-dependent.

### 2.6 Payoff spindle (Ø20 C45, 560 mm span, docs/28 geometry)

k = 48EI/L³ = 4.51e5 N/m (reproduces the docs/28 static 0.142 mm at 64 N);
**f = (1/2π)·√(k/(0.486·m_shaft + m_spool))**: **39.9 Hz** spool-only
(6.49 kg), **19.3 Hz** at the 30 kg limit, **13.6 Hz** at the 61 kg absolute
bound.

### 2.7 Belt strand and screw whirl

Belt f₁ = **378 Hz** at 190 N (368–388 Hz over the acceptance window;
[docs/37](37-system-dynamics.md) §2 string model). Blade-screw first whirl
≈2600 rpm ([docs/39](39-custom-cut-axis.md)) vs 150 rpm duty.

## 3. Separation margins vs the excitation table (target ≥ 2×)

| Structural mode | Band | Nearest excitation(s) at the envelope | Margin | Verdict |
|---|---|---|---|---|
| Frame sway (§2.1) | 19–38 Hz | blade screw 2.5 Hz; wheel 2.5 Hz; spool 0.40 Hz | ≥ 7.6× | PASS |
| Rail/bed composite (§2.2) | 82–123 Hz | tooth-pass fundamental 100 Hz sits **inside** the band (crossed at 395–500 mm/s); mains 50 Hz (cabinet-only) | 1.2× | **FLAG** (weak coupling — see below) |
| Bed cross-span (§2.3) | 245–368 Hz | feed-motor electrical 199 Hz (1.23–1.85×); tooth-pass 2× 200 Hz (1.22×); 3× 300 Hz sits **inside** the band from 408 mm/s up; feed detent sweeps 0–796 Hz through it at 154–231 mm/s | 1.2–1.9× | **FLAG (primary)** |
| Cut-axis plate (§2.5) | 98–352 Hz | ball-pass 75–100 Hz overlaps the loaded bound; 34E1K detent 500 Hz (1.42× vs the bare 352 Hz mode); electrical 125 Hz (2.8×) | 1.4–2.8× | **FLAG** |
| F101 shaft (§2.4) | 1.75 / 5.8 kHz | step train 7.96 kHz (4.5× above; swept transient during accel, negligible amplitude); PWM band 20–50 kHz is ≥ 25× above | ≥ 4.5× | PASS |
| Payoff spindle (§2.6) | 13.6–39.9 Hz | spool 1× 0.40 Hz | ≥ 34× | PASS |
| Belt strand (§2.7) | 378 Hz | tooth-pass 100 / 200 / 300 Hz | 3.78 / 1.89 / **1.26×** | Marginal on the 3rd harmonic |

**Flagged margins below 2×** and their coupling basis:

1. **Bed cross-span mode (245–368 Hz) vs the tooth-pass 2nd/3rd harmonics
   (200/300 Hz) and the feed-motor electrical line (199 Hz).** The mesh
   force is a cassette-internal couple (equal and opposite at the two
   pulleys), so the net force on the bed is a small couple and roller-contact
   ripple; but the 3rd harmonic sweeps 0→300 Hz and enters the mode band
   during normal max-speed runs (from ≈408 mm/s), and the electrical line
   approaches to 1.23× at 500 mm/s. This is the strongest single finding.
2. **Rail/bed composite (82–123 Hz) vs tooth-pass 100 Hz** — the fundamental
   crosses the band at 395–500 mm/s; again a couple excitation, so the
   transmitted force is weak, but the margin is < 2×.
3. **Cut-axis plate (98–352 Hz, loaded) vs ball-pass 75–100 Hz and the
   34E1K detent 500 Hz** — the carriage-mass-loaded mode at mid-travel
   (≈98 Hz) overlaps the ball-pass band.
4. **Belt 3rd tooth-pass harmonic vs belt f₁: 1.26×** — amplitude-weighted
   PASS retained: trapezoid harmonic amplitude decays ≈1/n² (the 3rd
   harmonic carries ≈1/9 of the fundamental energy) and the string model is
   conservative (belt bending stiffness raises f₁).

**Fix evaluation and selection:**

| Candidate fix | What it changes | Evaluation | Selected |
|---|---|---|---|
| Belt tension 180→200 N | belt f₁ 368→388 Hz only (+2.6%); no effect on the bed/rail/plate modes or the tooth-pass lines | Cannot recover the 1.26× 3rd-harmonic margin; tension is pinned at 190 N by the slack-strand fault margin (22.9 N at 5 Nm). **Rejected as primary fix** | No — keep 190 N |
| Microstep setting | ripple **amplitude**, not frequency: electrical (199 Hz), detent (796 Hz) and mesh (100 Hz) lines are fixed by motor poles and speed | CL57T/CL86T offer 16 settings 200–51200 pulses/rev (archived manuals); 6400 pulses/rev would cut step-quantisation ripple ≈3× at a 25.5 kHz step rate (inside the 500 kHz input and 40 kHz legacy limits) but requires the firmware map change and changes no flagged margin | Documented option; **not applied** (spec/firmware unchanged) |
| Damping pad under the bed | viscoelastic interface (3–5 mm PU/NBR) at the bed–rail joint: resonant Q 50–100 → 10–20 at the bed mode; slight softening | Directly attacks flags 1–2. Bolt-stack check: 25 mm bed + 3 mm rail wall + 3 mm pad = 31 mm leaves ≈4 mm of M10×35 thread in the DIN 929 nut (≈0.4·d — weak) → **M10×40 required** with the pad | **Conditional** — install if the commissioning impact test confirms a bed-mode margin < 2×; M10×40 screws then required |
| Elastomeric feet (4× 100×100 damping pads under F305) | damps the sway/rail modes, floor isolation, levels the machine | Cheap, no fastener/CAD change, also serves the P1 levelling datum | **Selected (mandatory)** |
| Motor-mount stiffening (H005 plate stack) | raises the loaded cut-axis plate mode away from ball-pass/detent | A rib under H005 is a CAD change (out of scope now). Immediate action: verify the 6-bolt torque and full plate contact, and the CL86T S1=A anti-resonance tune | **Conditional** — rib only if the impact test shows the loaded mode < 2× from 500 Hz |
| Commissioning impact test | verifies all four mode families against the §3 table | Instrumented step added to the runbook (P3-5) | **Selected (mandatory)** |

Selected plan: elastomeric feet at P1 + impact-test verification (P3-5) with
acceptance "measured bed mode ≥ 2× from the 199/200/300 Hz lines and the
cut-axis plate ≥ 2× from 500 Hz at the commissioning feed"; the bed–rail
damping pad (with M10×40) and the H005 stiffening rib remain pre-approved
contingencies, not CAD changes yet. The belt stays at 190 N.

## 4. Frame static check — 1200 N blade cut force

Reaction path: blade → H004 holder (42CrMo4+QT) → H006 carriage → HGH20CA
blocks → HGR20 rails → H005 plate → H002 riser → H001 datum → F301 bed →
F302/F303 rails → F304 legs → F305 feet.

Blade-point lever above the rail plane: wire Z112 + bed 25 mm = **137 mm**.
Worst case = single-sided load (one blade carries the full 1200 N):

| Term | Model | Deflection at the blade point |
|---|---|---|
| Leg sway | δ = F/(4·12EI/L³) = 1200/8.53e6 | **141 µm** (common-mode) |
| Overturning 164.4 Nm → leg-axial pair | ΔF = M/s_y = 161 N; θ = 2·Δ/(EA/L)/s_y | 0.23 µm |
| Rail/bed composite bending | θ = M·L/(8EI) = 164.4×1.02/(8×2.14e5) | **13.4 µm** (SS; 8.9 µm fixed-end) |
| H004 holder arm | [docs/37](37-system-dynamics.md) §4 | ≈4 µm |
| **Total** | — | **≈158 µm common-mode** |

Two qualifications make this PASS:

1. **The normal cut is balanced.** The two opposed 90° V blades load the wire
   in equal and opposite directions, so the net frame force/moment is ≈ 0
   (the imbalance is bounded by the closed-loop following-error alarms and
   the 4.2 mm travel limit). The full 1200 N one-sided case is a fault/jam
   scenario.
2. **The cut loop is self-referenced.** The wire at the cut plane is located
   by the F205 guides (X38–47) and the near nips (X75.5), all bolted to the
   same bed. Common-mode frame sway therefore does **not** move the blade
   relative to the wire; the cut-point-relative deflection is the internal
   loop only: holder ≈4 µm ([docs/37](37-system-dynamics.md)) + carriage
   plate ≈4 µm ([docs/39](39-custom-cut-axis.md) §4) + H005/riser/H001/bed
   local terms ≈ **20 µm total < 0.05 mm target** → **PASS at ≈2.5×
   margin**. The blade-edge 300 µm plain-plate bound remains a
   purchase-drawing requirement ([docs/37](37-system-dynamics.md) §4, gate
   13), and the 0.05 mm bed-flatness spec ([docs/30](30-machine-assembly-sequence.md)
   P1) is a manufacturing check this deflection adds to.

Frame stress at the single-sided load: leg bending moment FL/8 = 28.5 Nm →
σ = M·c/I = 2.3 MPa vs 235 MPa S235JR yield (≈100×); leg axial ≈0.5 MPa.
Structurally safe; the 158 µm common-mode motion affects only the
machine-to-floor position and guard alignment, not cut quality.

## 5. Verdicts

| Item | Key number | Verdict |
|---|---|---|
| Excitation inventory | 13 source families; feed tooth-pass 100/200/300 Hz, electrical 199 Hz, detent 796 Hz, step train 7.96 kHz; blade 2.5 Hz + ball-pass 75–100 Hz + detent 500 Hz; PWM band not published (20–50 kHz stated); mains 50 Hz cabinet-only | Complete at the desk level |
| Frame sway / rail composite / payoff / F101 / screw whirl | 19–38 / 82–123 Hz / 13.6–39.9 Hz / 1.75 kHz / 17.3× | PASS (lowest margins ≥ 7.6× on real excitations) |
| Bed cross-span mode vs mesh/electrical lines | 245–368 Hz vs 199/200/300 Hz → margins 1.2–1.9× | **FLAG** — damping feet selected, bed–rail pad conditional, impact test mandatory (P3-5) |
| Cut-axis plate vs ball-pass/detent | 98–352 Hz vs 75–100 / 500 Hz → 1.4–2.8× | **FLAG** — bolt-torque + anti-resonance verification now; H005 rib conditional |
| Belt 3rd harmonic vs belt f₁ | 1.26× | Marginal — amplitude-weighted PASS (≈1/9 harmonic energy; conservative string model); keep 190 N |
| Frame static at 1200 N | internal loop ≈20 µm < 50 µm target; 158 µm common-mode safe and cut-irrelevant | PASS ≈2.5× |
| Belt tension / microstep / damping pad / motor-mount fixes | evaluated §3 | 190 N kept; microstep documented only; feet pads + impact test mandatory; pad/rib conditional |

## 6. Assumption register

1. Top frame mass ≈150 kg (component geometric estimates; the guard panel
   masses are sheet-metal estimates).
2. Hybrid-stepper detent ripple at 4× electrical (200 cycles/rev) for both
   the 24HS40 and 34E1K 1.8° motors — standard 2-phase hybrid analysis;
   datasheets publish the detent torque magnitude (1600 g·cm) but no
   frequency decomposition.
3. SFU1605 nut ball count 30–40 — not published for the offered nut;
   ball-pass band stated.
4. Driver PWM 20–50 kHz — stated class band; the archived CL57T-V41/CL86T-V41
   datasheets publish no switching frequency (verified by extraction).
5. Rail/bed composite EI assumes full bed participation through the bolted
   joints (no slip); weld fixity taken as the SS/fixed bounds.
6. Bed plate strip uses ν = 0.33, E = 70 GPa for 6082-T6.
7. F101 shaft modes ignore bearing/housing compliance (raises the real
   frequency — conservative as stated).
8. The 50 mm cut-point stability target is the bed-flatness/cut-point
   criterion of [docs/30](30-machine-assembly-sequence.md) P1 applied to the
   load-synchronous deflection, not a new specification.
9. "25 rev/s" in the working brief corrected to 25 rad/s = 3.979 rev/s
   (238.7 rpm at 500 mm/s).

Ledger cross-reference: desk part recorded in
[docs/33](33-gate-ledger.md) row 45 / §2.8; physical remainder is the
commissioning impact test (runbook step P3-5).
