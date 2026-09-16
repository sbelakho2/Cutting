# Physical contradiction audit and analytic closure — S1-B-AUDIT-R1

**Revision:** `S1-B-AUDIT-R1-2026-09-15`
**Status:** desk audit of the checked CAD against every archived datasheet,
plus analytic closure of the remaining desk-computable items. Physical
measurement gates are listed with their exact procedures and remain open.

## 1. Findings summary

| # | Area | Finding | Severity | Resolution |
|---|---|---|---|---|
| 1 | Payoff posts | R2 posts were 10 mm thick in X while the Ø20.5 spindle bore (along Y) is 20.5 mm wide in X: the boolean cut severed each post into two solids (code reject "Disconnected mechanism part"). Physically the bore would remove the full plate thickness | high | FIXED: posts redesigned as 18 mm plates in Y (70×18) with coaxial Ø47 H7 pocket (14 deep) + Ø20.5 bore; mechanism R3, 507 occurrences (R4 2026-09-16, 504 occurrences after the measuring-part substitution); all checks pass |
| 2 | XPS output fuse | DF2CN10 (10 A gG) cannot protect the XPSUAF13AP 5 A DC-1 / 3 A DC-13 relay outputs for the ≈0.45 A contactor-coil load | medium | FIXED 2026-09-15: DF2CN02 (2 A gG) selected; [docs/22 §11.3](22-controls-and-wiring.md), BOM and sourcing rows updated. 2026-09-16: identity moved to Phoenix 3046032 terminal + ESKA 522620 2 A T 5×20 link (priced), closing the holder hold |
| 3 | Spool/hub interface | R2 first draft had a Ø60 hub on a spindle boss that did not exist in the CAD; face-touch fusion of flanges to hub was not boolean-safe | high | FIXED: hub tube passes through both flanges (true welded joint); single connected solid |
| 4 | Guard panel claims | "5 mm" side/rear/roof descriptions conflicted with the selected DC01 2 mm panels | low | FIXED in R3 descriptions (2 mm walls, 5 mm PC door only) |
| 5 | Belt/pulley kinematics | HTBN225S5M-250 (225 pitch / 45 teeth / 62.5 mm centres) vs `nominal_centres=62.5` | none | Confirmed consistent; 20-tooth pulleys, 5 mm pitch, 25 mm belt all agree |
| 6 | Encoder shaft | ifm RU3100 shaft Ø6 into a 6 mm coupling bore (2026-09-16 substitution) | none | Consistent (6 h6 class into coupling); M202 measuring shaft 6 h6 into ifm E60137 6 mm wheel bore |
| 7 | A12 ring vs groove | Substituted ring (2026-09-16): Wegertseder 1950-010 DIN 471 12×1 Federstahl, d3 = 11.0 mm, s = 1.0 (−0.06). Groove Ø11.45±0.02 (11.43–11.47) sits inside the DIN 471 groove band d2 = 11.5 (−0.11). Install spread 1.0 mm (11.0 → 12.0 shaft); seated spread ≈0.45 mm — d3 < groove bottom always, so the ring grips the groove (no radial float). Axial float = groove width 1.12±0.02 (1.10–1.14) − ring 0.94–1.00 → 0.10–0.20 mm; seated OD ≈13.45 mm → ≈0.72 mm radial protrusion per side. Fit verdict PASS (coupling-ring-replacement-findings.md) | low | Accepted with assembly verification (below); no function depends on a tight A12 seat |
| 8 | UMCL collar | Bore 10.000–10.030 on 10 h6 shaft: 0–0.039 mm clearance, key-transmitted | none | Consistent with docs/12; no interference-fit claim |
| 9 | CB-PS05 identity | The cabinet row claims a 5 V output from a Siemens LOGO!POWER identity (a 24 V product family) | medium | Row corrected 2026-09-16: second 24 V PSU feeding the 5 V rail via the IF-B01 on-board regulator; exact 6EP3310 identity and the 5 V derivation to be confirmed at purchase. The 6EP3332-6SB00-0AY0 (CB-PS24) identity was verified against a live Automation24 listing (EUR 68.60, in stock) |
| 10 | STEPPERONLINE shop prices | CL57T-V41 and LE-350-36 prices are only visible in a browser | low | Purchase-time record step defined in the master plan §2.2 |

## 2. Analytic closures

### 2.1 Payoff spindle bending (P002, Ø20 C45, span 560 mm between 6204 seats)

δ = W·L³/(48·E·I), σ = W·L/(4·Wₛ), E=210 GPa, I=π·20⁴/64=7854 mm⁴,
Wₛ=π·20³/32=785 mm³.

| Load case | W | Deflection | Bending stress | Verdict |
|---|---|---|---|---|
| Spool only (flanges 5.92 kg + hub 0.57 kg) | 64 N | 0.14 mm | 11 MPa | fine |
| 30 kg full spool bound | 294 N | 0.65 mm | 52 MPa | fine; commissioning limit |
| 61 kg absolute bound | 600 N | 1.33 mm | 107 MPa | < C45 yield (≈340 MPa); deflection changes spool tangent height — do not exceed 30 kg reel without a re-check |

C45 yield 340 MPa is a nominal handbook value; the actual material certificate
governs at procurement. The payoff is not a precision axis: the deflection
affects payout geometry, not length accuracy (the measuring wheel is
independent).

### 2.2 Guard aperture safety distance (EN ISO 13857 reach-through)

The Ø30 wire apertures are slots through the 2 mm side panels at
X ±(345…347); the nearest hazard (blade pair at X0) is ≈345 mm behind the
panel. EN ISO 13857 Table 4 (regular openings, slot row) gives a finger-reach
of ≤120 mm for slot widths up to 30 mm. With the wire installed the residual
annular opening is ≤12.75 mm (still less reach). **345 mm > 120 mm: the
aperture distance passes with ≈2.9× margin.** The evaluation must be
reconfirmed against the current standard edition and the completed risk
assessment (physical sign-off); the calculation does not itself license the
aperture.

### 2.3 Key capacity (DIN 6885-A 5×5×28 on Ø14, fault torque 5 Nm)

F = 5000 Nmm / 7 mm = 714 N. Shear τ = 714/(5·28) = 5.1 MPa; bearing
p = 714/(2.5·28) = 10.2 MPa. Against C45 key allowables (shear ≈180 MPa,
bearing ≈120 MPa in clamped steel hubs): margins ≈35× / ≈12×. PASS.
The UMCL roller key on Ø10 (3×3) at the same torque: τ = 1000/(3·20) =
16.7 MPa — also a large margin.

### 2.4 Belt capacity (S5M 25 mm, 20-tooth pulleys, 62.5 mm centres)

Design torque: running 1.0 Nm × 2.5 service = 2.5 Nm; fault 5.0 Nm. The
archived MISUMI S5M capacity records place a 25 mm S5M belt above ≈150 W at
the roller speed range (≈25 rad/s ≈ 240 rpm → ≈6 Nm equivalent); fault torque
5 Nm stays inside the belt rating with margin. PASS on catalogue capacity;
belt tension is a commissioning setting.

### 2.5 Bearing thermal input (F101 seats)

Holding-mode motor loss: 24HS40 5.0 A/phase, 0.60 Ω/phase → 2·5²·0.6 =
30 W worst-case at standstill, delivered to the shaft through an insulating
S5M belt (heat path through pulleys is the only conduction route; the belt is
a thermal break). Roller friction heat at 50–75 N nip is negligible
(<1 W). The seat temperature field is therefore bounded by the motor case
temperature, which the archived datasheet does not publish; the closure path
is: measure motor case temperature at rated hold, then apply the conduction
calculation in [the bearing-fit record](../engineering/generated/bearing-fit/report.md).
Recorded as calculated-input/measurement-defined, not closed.

### 2.6 Regeneration and discharge

Closed with numbers in [docs/22 §11.2](22-controls-and-wiring.md):
worst-case ≤0.1 J per stop against ≥0.18 J bus storage — regeneration is not
a design driver; discharge estimate and the bleed-resistor proposal are
recorded there.

## 3. Interface-fit register cross-check

The full pairwise register lives in
[docs/30 §3](30-machine-assembly-sequence.md). This audit re-verified every
pair against the archived datasheets in `engineering/sources/` and found no
additional contradiction beyond findings 1–8 above.

## 4. Remaining physical gates (unchanged, procedures defined)

Matched bearing measurement, weld inspection, flatness/runout, spring-force
settings, belt tension, blade edge qualification, stop-time and bus-decay
measurement, thermal mapping, risk-assessment sign-off, full-spool weighing
and cycle-time confirmation. Each has a defined procedure, template and
acceptance criterion in docs/12–14, docs/20, docs/22–24, docs/26 and
docs/30; none can be honestly closed without hardware.
