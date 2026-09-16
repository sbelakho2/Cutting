# 38 — Fatigue and life desk assessment (feed desk half)

Date: 2026-09-16. Status: desk study; nothing here releases fabrication or
establishes a physical life number. This closes the desk halves of the
fatigue/durability qualifications and leaves every physical gate unchanged.

## 1. Duty basis (stated)

| Assumption | Value | Source / justification |
|---|---|---|
| Operating day | 8 h/day, 250 days/yr, 5 years | task-stated design duty |
| Cut rate | 30 cuts/min | task-stated; sits between the docs/02 sequential cycle budget ≈28/min (100 mm) and ≈16.5/min (1000 mm) |
| Wire lengths | 200–1000 mm qualification, 200–3000 mm spec | docs/01 §2 |
| Total duty | **10,000 h**, **≈1.8×10⁷ cuts** | 30×60×8×250×5 |
| Shaft alternating cycles | ≈2–6×10⁸ (≈20–32 rev/cut × 1.8×10⁷ cuts; roller Ø20 = 62.83 mm/rev, forward+reverse) | feed geometry, docs/01 |
| Speeds | shaft ceiling 260 rpm (docs/01:35); 0.5 m/s feed at Ø20 implies 478 rpm; measuring wheel 150 rpm (0.5 m/s ÷ 200 mm circumference); payoff ≤160 rpm (Ø60 hub bound, outer-layer payout ≈24 rpm) | docs/01, docs/13 |

The duty cycle count is far beyond the S-N knee (10⁶–10⁷), so the shaft
assessment is an endurance-limit (infinite-life) check, not a finite-life
Miner sum.

## 2. F101 shaft fatigue — S-N (gate 11 desk half)

### 2.1 Stress basis

- **Whole-shaft working bound: 76.6 MPa** (feed-fem `S1-B-FEED-R6-2026-09-12`
  finest upper stress 76.58763718 MPa; torus gmsh/netgen 74.73774147 MPa).
  Fault bound **158.2 / 165.0 MPa** (feed-fem / torus). 48-phase rotating
  loads with an analytic unsampled-angle remainder bound — the values are
  alternating (R ≈ −1) envelope peaks, so σ_a = 76.6 MPa working.
- **Converged regional submodel peaks (docs/28 §5), working**: M15 washer-slot
  corner **60.4 MPa** (netgen, bound 63.5; gmsh 59.3, bound 62.5 — the worst
  working regional peak), roller shoulder **45.2 MPa** (gmsh, bound 47.9),
  roller key floor **43.0 MPa** (netgen, bound 44.1; gmsh 41.8), pulley
  slot-lip/key **37.1 MPa**. Fault regional: roller key floor **147.9 MPa**
  (netgen; gmsh 143.9), M15 corner 115.1–118.2, roller shoulder 104.7–106.9.
- The submodel peaks already resolve their notches (converged raw peaks), so
  no further Kt is applied to them; the Kt screen below is a conservative
  cross-check on the whole-shaft amplitude.

### 2.2 Material and fatigue limit (literature values with source)

- 42CrMo4+QT: yield **650 MPa** certified (docs/12:17); Rm ≈ 850–1000 MPa at
  this temper.
- Fatigue limit **σ_w = 400–450 MPa** notched-bar basis: FKM-Richtlinie
  "Rechnerischer Festigkeitsnachweis für Maschinenbauteile" (VDMA-Verlag)
  tabulates 42CrMo4 V σ_w,zd ≈ 398 MPa at Rm ≈ 1000 MPa; the classic
  0.4–0.5·Rm rule for QT steels brackets the same range.
- Notch factor at the R0.2 fillets: **Kt ≈ 1.5–2** (stated, W.D. Pilkey,
  *Peterson's Stress Concentration Factors*, 3rd ed., Wiley 2008); the
  record's own Pilkey implementation gives **2.534** (M15 runout, D/d
  15/13.5, h/r 3.75) and **2.586** (roller shoulder, D/d 12/10, h/r 5)
  (docs/28 §6).
- Reduction factors: surface b1 = 0.85 (machined/turned shaft), size
  b2 = 0.97 (Ø12–15 mm), alternating load (belt/roller loads rotate; gravity
  mean is negligible). Effective limit σ_w,eff = 400–450 × 0.85 × 0.97 =
  **329.8–371.0 MPa**.

### 2.3 Fatigue safety factor

n_f = σ_w,eff / (Kt · σ_a), σ_a = 76.6 MPa:

| Kt | n_f at σ_w = 400 MPa | n_f at σ_w = 450 MPa |
|---|---|---|
| 1.5 | 2.9 | 3.2 |
| 2.0 | 2.2 | 2.4 |
| 2.6 (record, roller shoulder) | 1.7 | 1.9 |

Against the converged regional peaks (no re-Kt): n_f = 329.8/60.4 = **5.5**
(worst working regional, M15 corner), 7.3 at the roller shoulder, 7.7 at the
key floor. Handbook cross-check at the roller shoulder (docs/28 §6): nominal
bending 12.37 MPa × Kt 2.586 = 32.0 MPa → n_f = 10.3.

**Verdict: PASS (desk).** The worst stated combination (Kt = 2.6, σ_w = 400)
keeps n_f ≥ 1.7 against the whole-shaft envelope, and ≥ 5.5 against the
converged notched regional peaks; the shaft operates in the infinite-life
regime at the design duty. `fatigue_verified` stays false; the physical
rotating-fatigue test (gate 11, docs/15) remains mandatory.

## 3. Fault-load check (5 Nm, 1000 cycles)

- Fault bounds 158.2 / 165.0 MPa vs yield 650 MPa: margin **4.1× / 3.9×** —
  fully elastic, no yielding, hence no low-cycle-fatigue/ratcheting mechanism.
- As a fatigue block: the converged 165 MPa peak is already below σ_w = 400
  MPa; even double-counting a conservative Kt = 2.6 gives 429 MPa ≈ 1.07·σ_w,
  for which an S-N slope of 5 predicts failure at ≈7×10⁵ cycles → 1000 fault
  cycles are **<0.2 % damage**.
- Margin statement: the 5 Nm fault envelope is 2.18× inside the belt capacity
  (feed-analysis) and ≤ 25 % of yield at the shaft; 1000 fault cycles are
  fatigue-benign. The physical belt proof (5 Nm, 10 s, 10 cycles, slip-marked,
  docs/12:152) remains the gate.

## 4. Bearing L10 (ISO 281, P = Fr; toothed belt → no working thrust)

Radial loads: feed-analysis peak resultants rear **321.2 N** / front
**309.9 N** (belt ±480 N + roller ±60 N worst-sign corners); the nip preload
50–75 N (docs/20 cell) is added as a conservative colinear increment and
rounded: **P = 400 N rear, 390 N front**. Wheel force 6–19 N (docs/20 cell).
Payoff ≤30 kg spool + unweighed wire ≤60 kg total.

| Bearing | C / C0 (N) | Radial load | Speed | L10h |
|---|---|---|---|---|
| 6001-2Z/C3 (rear) | 5400 / 2360 (catalogue-evidence:197) | 400 N | 260 rpm | **1.6×10⁵ h** |
| 6002-2Z/C3 (front) | 5850 / 2850 (catalogue-evidence:197) | 390 N | 260 rpm | **2.2×10⁵ h** |
| 626-2Z (wheel shaft) | 2340 / 950 (catalogue-evidence:167) | 19 N | 150 rpm | **2.1×10⁸ h** |
| 6204-2Z/C3 (payoff) | 13 500 / 6550 (SKF; C0 per docs/26:108) | 294 N (60 kg ÷ 2; 589 N at the docs/26 "≤60 kg half-load" reading → 1.3×10⁶ h) | ≤160 rpm | **1.0×10⁷ h** |

**Target 20,000 h — set here with justification:** the repo states no
bearing-life target (checked docs/01, 12, 20, 24, 26, 33); 20,000 h = 2× the
10,000 h five-year duty, consistent with SKF life-selection guidance for
machines in 8 h/day service.

**Verdict: PASS.** Every bearing clears 20,000 h; the minimum margin is
**7.9×** (6001). Sensitivity at the 478 rpm implied by 0.5 m/s feed at Ø20:
6001 → 8.6×10⁴ h (4.3×), 6002 → 1.2×10⁵ h — still clear. All loads are
envelope peaks used as constant loads, so these L10h values are conservative
against the mixed-duty profile.

## 5. Key/collar fretting and A12 ring duty

- **F109 3×4×18 key/collar** (collar-contact study numbers): face contact
  26.2 MPa FEM (25.25 ideal) vs C45 bearing practice 120 MPa; key shear
  18.5 vs 180 MPa; key peak vm 164 vs ≥300 MPa F109 min yield; aluminium
  collar-wall bearing 43.3 MPa limit → 1.71× margin. Fretting picture: fault
  key tangential force 1000 N against the clamp-friction capacity
  μN = 0.15 × 7 kN ≈ 1050 N puts the fault event **at the slip threshold** →
  micro-slip/fretting at the key-hub and collar-bore interfaces is expected
  at fault events; at working 1 Nm (≈200 N) the friction margin is ≈5×. The
  aluminium collar wall is the soft consumable fretting interface; fretting
  life is not quantified desk-side — the physical belt proof (slip-marked)
  and the incoming bore/keyway measurement (gate 1) remain the gates.
- **A12 ring (DIN 471 12×1, Wegertseder 1950-010)**: pure static axial
  locator of the rear bearing stack — no torque path (keys/collars transmit
  torque), axial load limited to stack shuffle (endfloat 0.08–0.50 mm,
  docs/12); seated spread ≈0.45 mm, protrusion ≈0.72 mm (gate 37). Duty is
  benign; the ring is a consumable replaced at bearing service. Physical
  groove-seating check at assembly unchanged.

## 6. Blade pair — literature stroke-life order of magnitude

Tool-steel shear/blanking edges on soft nonferrous stock (copper) typically
achieve **10⁵–10⁷ cuts per edge set** between regrinds (Kalpakjian & Schmid,
*Manufacturing Engineering and Technology*, 7th ed., Pearson — punch/die
life for blanking soft metals; an order-of-magnitude anchor, not a
manufacturer number). Against the ≈1.8×10⁷-cut five-year duty this implies
≈2–180 edge sets — the blade pair is a **consumable** with a scheduled
regrind/replacement and inspection interval. No qualified blade-life number
is claimed; the physical edge qualification (gate 13, docs/24 §4.4) remains
the gate.

## 7. Gate ledger impact

docs/33 gate 11 row updated (desk S-N closed; physical test remains) and
§2.6 added recording the fatigue/L10/fretting desk closures. Nothing here
releases fabrication.
