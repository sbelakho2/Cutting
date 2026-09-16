# Bearing contact compliance and thermal field — desk studies (F101 front seat / 6002-2Z-C3)

Desk closure of the housing-compliance and thermal-field parts of the bearings/joints/thermal gate (docs/33 gate 10; docs/12:38,68; docs/28 §2.5). Local linear FEM, ≤150000 nodes, resolved-pressure contact (the collar_contact.cpp pattern) and a scalar steady-state conduction solve. Nothing here releases fabrication or replaces the instrumented physical test.

## Study 1 — bearing/housing contact compliance

### Fit values used (from the fit record)

- Shaft seat: matched 15j5, **6 µm diametral interference** applied (record window 3–6 µm; guard-banded measured acceptance 3.5–5.5 µm, docs/12:36). Radial half = 3 µm.
- Housing: the fit record's F112 front bore is **Ø32.008–32.019 mm** against the 6002 outer-ring OD 32.000 (0/−0.009) mm — an **8–28 µm diametral clearance** (minimum thermal residual 2.432 µm per the 256-corner record). There is **no housing interference in the record** (the "K6-class" reading in the task prompt conflicts with the record's +8/+19 µm bore; the record governs). The outer ring is a clearance fit, so the housing carries **zero fit-induced contact pressure**; the housing wall is exercised instead by the resolved bearing radial load on the bore wall.
- Materials: shaft 42CrMo4+QT (E 205 GPa, ν 0.29, yield 650 MPa certified), rings 100Cr6 bearing steel (E 210 GPa, ν 0.30, ≈1700 MPa hardened-ring yield, handbook), F112 block 6082-T6 (E 70 GPa, ν 0.33, ≈260 MPa Rp0.2, handbook). Steel-steel on the press interface; the block is modelled as aluminium. The linear solve uses one stated material (E 205 GPa, ν 0.29): within a uniform region the stress field is E-independent and the Lamé fit stresses are E/ν-independent, so the per-material ring and housing values differ by ≤3 %; only displacements scale with E.

### Model and method

- Geometry: Ø15 shaft section (seat z 0–9 mm) with Ø19 shoulder and Ø12 neck, 6002 inner ring (Ø15/19.74 × 9), outer ring (Ø27.26/32 × 9) and the F112 block (46 × 37.5 × 19.02, Ø32.0135 bore). The meshed shaft seat is a Ø14.995 stand-in, the inner-ring bore a Ø15.2 stand-in and the housing bore a Ø32.3 stand-in (0.15 mm over the Ø32.0135 CAD bore), and the rings are inset 0.1 mm axially — these stated stand-ins keep the near-coincident conforming surfaces separable for the resolved-pressure patches and the gmsh import heal; the fits are applied as pressures/data, not geometry.
- Shrink fit solved as a Lamé resolved pressure p = δ / (r·(C_shaft + C_ring)) applied on both interfaces through 24 radial sectors (collar_contact resolved-pressure pattern): **p = 17.65 MPa** at the 6 µm worst-case interference.
- Compliant supports: the 6002 ball complement + aluminium bore wall in series is represented by a stated radial chain stiffness 35 N/µm on the inner-ring OD (stated; sensitivity ×0.5/×2 solved), ring end faces by soft axial springs (KM2/MB2 stack location), the housing by its bolted outer faces, and the shaft ends by soft stabilising springs. The four-mesh style reference case fixes the seat surface rigidly (distributed-resultant idealisation) with the rings pinned out of the load path.
- Load: radial 310 N = the recorded front-bearing peak resultant (feed-analysis: 309.9 N, fault-sign combination).
- Mesh: 22407 nodes / 75029 tetrahedra, ≤150000 cap.

### Results

| Case | Seat peak vm MPa | Ring peak vm MPa | Housing wall peak vm MPa | Max disp µm |
|---|---|---|---|---|
| Fit only (resolved 17.65 MPa) | 22.73 | 82.26 | 0 | 37.22 |
| Rigid seat (four-mesh style) + load | 4.28 | — | — | 0.3609 |
| Compliant chain + fit + load | 23.08 | 82.26 | 0 | 2644 |
| Chain ×0.5 (17.5 N/µm) + fit + load | 23.08 | 82.64 | 0 | 2644 |
| Chain ×2 (70 N/µm) + fit + load | 23.08 | 81.5 | 0 | 2644 |
| Bore wall resolved load | — | — | 0.9817 | 0.05972 |

The compliant-case displacements are dominated by the stated 35 N/µm chain and the soft stabilising springs (a 310 N load on a 35 N/µm chain is ≈9 µm of rigid translation plus rotation); they are reported for completeness, not as a bearing-seat deflection claim.

### Allowables and boundary effect

- Contact pressure on the shaft seat: **17.65 MPa** (resolved Lamé, 6 µm worst case; 3 µm gives 8.823 MPa). Contact pressure on the housing: **0 MPa** (record clearance fit).
- Peak von Mises in the shaft seat: **22.73 MPa** vs 42CrMo4+QT 650 MPa (margin ≈ 28.59×); inner ring 82.26 MPa vs ≈1700 MPa hardened 100Cr6 (margin ≈ 20.67×, the Lamé hoop at the ring ID is 65.87 MPa); housing wall 0.9817 MPa under the resolved 310 N bore load vs 6082-T6 260 MPa.
- Boundary effect: with the compliant chain, the radial-load-induced seat stress drops from the four-mesh rigid-boundary peak **4.28 MPa** to **0.3478 MPa** (fit+load 23.08 − fit-only 22.73) — a **91.87 %** change relative to the rigid reference. The rigid peak sits at the constraint edge (an idealisation artifact); in absolute terms the difference is 3.932 MPa, negligible against the 22.7 MPa shrink-fit pressure and the 76.6/158.2 MPa working/fault whole-shaft bounds: the seat stress is dominated by the fit, not by housing compliance.

### Study 1 verdict

**PASS** — desk part: the 6 µm matched interference resolves to 17.65 MPa on the seat (vs 650 MPa) with the ring at 82.26 MPa (vs ≈1700 MPa); the record's housing clearance means the housing carries no fit pressure, and the resolved 310 N bore-wall load leaves the 6082-T6 wall at 0.9817 MPa. Housing compliance does not materially change the seat stress (≤3.932 MPa absolute). The physical steps (measured preload/contact on the actual build, docs/33 gate 10) remain.

## Study 2 — front-half steady-state thermal field

### Inputs (stated)

- Heat source: **30 W** standstill motor holding loss (24HS40: 5.0 A/phase, 0.60 Ω/phase → 2·5²·0.6 = 30 W, docs/28 §2.5, catalogue-evidence) applied **uniformly on the Ø14 pulley-seat surface** (S31.5–64.5) as the worst-case belt-delivered bound. The S5M belt is the stated thermal break (docs/28 §2.5); applying the full motor loss directly at the seat is an upper bound, not a measured input.
- Conduction along the shaft: k = **42.7 W/mK** (42CrMo4). Still-air convection: **h = 10 W/m²K**, ambient 25 °C, on every exposed shaft surface. Rear cut face S31.5 insulated (upper bound: all 30 W must flow forward).
- Bearing seat (S73–82): conduction path to the housing at **25 °C** through a stated effective conductance **2800 W/m²K** (series chain: seat/inner-ring press contact ≈8000 W/m²K → 3.4 W/K; balls + grease ≈2 W/K at standstill; grease-filled 5 µm ring/bore gap ≈27 W/K → ≈1.2 W/K over the 424 mm² seat). Sensitivity 1000 / 8000 W/m²K solved.
- Roller surfaces (Ø10 journal S92–113 + end face): pinned at **25 °C**.

### Results

| Case (h_seat W/m²K) | Seat max °C | Seat mean °C | Rise above housing K | Power balance |
|---|---|---|---|---|
| Central (2800) | 54.66 | 44.98 | 29.66 | 0.00 |
| Sensitivity (1000) | 76.73 | 64.96 | 51.73 | 0.00 |
| Sensitivity (8000) | 40.53 | 33.13 | 15.53 | 0.00 |

### Envelope check (bearing-fit report)

The fit record's explicit temperature envelope is: housing 10–55 °C; outer ring 0–5 K above housing; inner ring 0–10 K above outer; shaft 0–5 K above inner — i.e. the shaft may sit at most **5+10+5 = 20 K above the housing**. With the housing at the stated 25 °C ambient, the front-seat rise must stay ≤ 20 K. Computed seat rise (max): **29.66 K** at the central conductance (sensitivity 51.73 / 15.53 K). Envelope met at the central stated value: **no**.

### Study 2 verdict

**HOLD** — the prescribed worst-case 30 W bound at the pulley seat does not robustly meet the 20 K shaft-above-housing envelope across the stated seat-conductance range; the instrumented cold-start/hot-run test (docs/12:38) and the measured motor-case temperature (docs/28 §2.5) are the required closure, and the belt thermal break plus the rear-half conduction path are stated mitigations, not used to relax the number.

## Files

- VTUs: vtu-study1/ (fit, rigid_load, compliant_load, bore_wall_load), vtu-study2/thermal.vtu.
- Node counts: study 1 22407, study 2 8914, both ≤150000.
