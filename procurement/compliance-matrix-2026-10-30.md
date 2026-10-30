# Part compliance matrix — sourced parts vs requirements, drawings and spec sheets

Date 2026-10-30. Every sourced/selected part is checked against the governing
requirement (requirements register / goal), the drawing/spec source (shared/*.hpp,
docs/) and the supplier's own spec sheet. Verdicts: **COMPLIANT**, **CONDITIONAL**
(a named, documented gate remains), **FIXED** (non-compliance found and corrected
in this pass), **OPEN** (identity not yet selected). Evidence links are dated.

| part | governing requirement / drawing | spec-sheet evidence | verdict |
| --- | --- | --- | --- |
| AX-R01 HGR20 400 rail + 2x HGH20CA | G14/G37; H005/H006 drawings; 400 mm rail, 20 mm width | 20 mm width / 400 mm length / S55C / 2 blocks per set confirmed on the Alibaba page; 1688 rail cut-to-length to 4000 mm | **CONDITIONAL** — C0/C not published anywhere: certified values (C0 >= 7.5 kN, C >= 15.1 kN per analysis/axis-rating-requirements) requested at order (RFQ-03) |
| AX-S01 SFU1610 C7, 10 mm lead, 500 mm | G11/G21/G37; one frozen config 10 mm lead/1000 pulses/rev; nut >= 11.3 kN class | pitch 10 mm, nominal Ø16, 40Cr, 30 kN max-load claim, 500 mm, machining included | **CONDITIONAL** — C7 certificate + journal machining (bearing seats 12h6/10h6, coupling extension Ø8h7x18) confirmed at order |
| BK12/BF12 supports | shared/cutting_head_spec.hpp pins BK12/BF12; bore must equal the machined journal | BK12 = 6001 bearing (12 mm bore), BF12 = 6000 bearing (10 mm bore) per the seller bearing listing | **COMPLIANT** with clarification: bearing journals 12h6 (fixed) / 10h6 (floating); the BOM's "Ø8h7x18" is the COUPLING extension beyond the support (matches CP-01's 8 mm bore), not the bearing seat. BOM text clarified |
| DR-M01 34E1K-85 | G11 1000 pulses/rev family, G37 loads; 14 mm shaft for CP-01 | 8.5 Nm, 0.54 Ω, 5 mH, 1.8°, 1000 PPR encoder, full datasheet PDF captured | **CONDITIONAL** — hot torque curve at the duty point is a P05/TS gate; electrical rating inside the 48 V rail |
| DR-02 CL86Y-V20 head driver | G29/G31; must accept the branch voltage | rated **24–50 VDC** (page) — the old 60 V branch EXCEEDED it | **FIXED** — head PSUs changed to LE-350-48 (48 V); the cabinet spec and docs/01 updated; regen acceptance tightened: bus must stay <= 50 V |
| TR-D02 CL57Y-V20 feed driver | same | rated 24–50 VDC; feed branch is 36 V (LE-350-36) | **COMPLIANT** |
| PS-01 supply | branch voltage must sit inside every driver rating | LE-350-48: 48 V 7.3 A 350 W, $17.88 (as low as $15.96), in stock | **FIXED** (was LE-350-60) — dated exact price and link in the BOM |
| 48 V regen bound | G33; V20 drivers max 50 V | 48 V + 2200 µF + 0.2 J -> **49.858 V** (inside); 48 V + 4700 µF + 0.5 J -> **50.167 V** (over) | **CONDITIONAL** — absorption must bound E <= 0.216 J at 2200 µF or E <= 0.49 J at 4700 µF; measured bus decay (P11-T1) |
| CP-01 R+W MK2/100/50/8/14 | G11 16 mm hub fit, 8/14 bores | 10 Nm rated; bores 8/14 match screw extension and 34E1K-85 shaft | **CONDITIONAL** — stall basis 10.2 Nm (8.5 x 1.2) exceeds the 10 Nm nominal rating: R+W peak/reversal rating and the driver current limit must be verified (RFQ-02) |
| TR-M01 24HS40-5004D-E1000 | G09 feed 0.50 m/s, 260 rpm ceiling; shaft 10 mm | 4.0 Nm, 10 mm shaft x 21 mm, 1000 PPR | **CONDITIONAL** — hot loaded torque at 500 mm/s is P06/TS; feed coupling bore must be 10 mm (see TR-CP row) |
| TR-CP01 feed coupling | must match 10 mm motor shaft and 12 mm screw journal | not yet sourced to an exact MPN | **OPEN** — select a 10/12 bore coupling (RFQ-02) |
| HTD5M belt HTBN225S5M-250 | G28 180–200 N, centres per cassette variant | 225 mm closed length = 2C + 100 mm pitch circumference -> C = 62.5 mm; cassette centres are 46/100/54/64 variants | **CONDITIONAL** — belt length must be re-selected per the final centre distance (a 54 mm centre needs ~208 mm; 62.5 mm gives 225 mm). Flagged for the cassette variant freeze |
| UMCL40 rollers | roller Ø40 (feed scale 15.915 steps/mm) | Ø40 urethane roller class | **COMPLIANT** |
| SKF 6204-2Z/C3 | G27 spindle 20 h6, >= 6 kN C0 class | 20x47x14, C0 6.55 kN, GBP 8.68/pair dated | **COMPLIANT** (spool load ~1.1 kN << C0; P01 fit record still owed) |
| ifm E60137 wheel | measurement_spec wheel OD 63.66 mm, bore 6.0 mm | Ø63.66 measuring wheel | **CONDITIONAL** — bore 6.0 mm confirm against the M2xx shaft at order |
| ifm RU3100 encoder | G24/G40; differential interface into AM26LV32E | TTL 2048 PPR class | **CONDITIONAL** — must be differential (A/A̅/B/B̅) to match the AM26LV32E receiver; verify the exact pin-out at order |
| AM26LV32E receiver | 3.3 V logic core | device is a 5 V-class RS-422 receiver | **CONDITIONAL** — confirm supply rail compatibility on the IF-B02 carrier (use the 3.3 V-compatible variant or a 5 V rail); flagged in the IF-B02 work order |
| TPL7407LDR sink driver | drives 4 driver opto inputs | 600 mA/ch, 40 V, 3.3 V-compatible input (full TI datasheet captured, LCSC C139663) | **COMPLIANT** |
| Pfannenberg PF66000 + PFA60000 | G30: 40 C room, 800 m3/h service | 230 V, IP54, 445 m3/h per fan at 50 Hz -> 890 m3/h two-path; EUR 171.89 + 34.65 dated | **CONDITIONAL** — installed P-Q and thermal margins are the TS campaign |
| XPSUAF13AP safety relay | G35 dual-channel E-stop/guard | dual-channel, 3 NO safety outputs, PL e capable | **CONDITIONAL** — PLr validation (P12) |
| LC1D12BD contactors | G31 DC make/break on 36/48 V branches | 12 A AC-3, 24 V DC coil; DC-13 derating must be checked at 48 V | **CONDITIONAL** — DC make/break test (P03-T1/T2) |
| SLC-F-024-20/20-R4 guard switch | G34/G35 | 24 V, PL e class | **CONDITIONAL** — guard layout + P12 |
| PLC-RSC-24DC/21 couplers | field input isolation, wetting | 24 V DC, 6 A contacts; wetting fix in hardware/if-b02 (2.37 mA) | **CONDITIONAL** — wetting measured at P03-T5 |
| Rittal AE 1180.500 cabinet | G26/G30 layout | 600x600x210 class | **CONDITIONAL** — final layout fit (Phase H respin + TS) |
| Waste sweep / chute sensor / output takeaway | G43/M30 | identity not selected | **OPEN** — bom/phase-h-io-occurrences.csv, P08 acceptance |

## Corrections applied in this pass
1. **Head supply voltage**: CL86Y-V20/CL57Y-V20 are 24–50 VDC; the 60 V head branch
   was non-compliant. Head supplies are now LE-350-48 (48 V), cabinet spec and
   design-basis doc updated, and the regen bound (<= 50 V) added to the electrical
   analysis with quantified acceptance (49.858 V at 0.2 J/2200 µF).
2. **BK12/BF12 vs journal**: clarified that the 8h7×18 is the coupling extension;
   bearing seats are 12h6 (BK12) / 10h6 (BF12) - consistent with CP-01's 8 mm bore.
3. **48 V regen cases added** to analysis/electrical_thermal with the 4700 µF case
   showing the 0.49 J cap.

## Still to evidence at order (all listed in RFQ-01/02/03)
C0/C ratings (HGR20), C7 certificate and journal drawing (SFU1610), R+W peak
rating (CP-01), RU3100 differential pin-out, E60137 bore, feed coupling MPN,
belt length vs final centres, DC-13 contactor derating at 48 V.

## Closure addendum (datasheet verification pass, same day)

| item | verified against | result |
| --- | --- | --- |
| HGR20 block ratings | HIWIN HGH20CA listing: **C = 17.75 kN, C0 = 27.76 kN** (dynamic/static) vs criteria C >= 15.1 kN, C0 >= 7.5 kN | **CLOSED — COMPLIANT with margin (C0 3.7x, C 1.18x)** provided the GENUINE HIWIN listing is bought (Alibaba 1601608482426 / Pinsi ORIGINAL HIWIN) with COC; clone sets remain excluded |
| CP-01 R+W MK2/100 | R+W official operating instructions MK2 5-100: nominal torque (size 100) 10 Nm non-ATEX / 7 Nm ATEX variant; bores 5-24 mm H7; **clamp screws ISO 4762 M4 at 4.5 Nm**; ambient -20..+100 C | **CLOSED — COMPLIANT** on the nominal basis (motor stall 8.5 Nm <= 10 Nm nominal; 8/14 bores within 5-24; 4.5 Nm clamp torque now sourced, matching docs/12). Transient 1.2x (10.2 Nm) sits below the standard 2x peak class - the exact peak figure is requested with the order (RFQ-02) and P01-T5 records it |
| E60137 | traceparts/Radwell/ifm: OD **63.66 mm**, **bore 6 mm**, circumference 200 mm | **CLOSED — COMPLIANT** with measurement_spec (wheel_od 63.66, wheel_bore 6.0) |
| RU3100 encoder | ifm datasheet: incremental encoder, **6 mm solid shaft**, resolution programmable **1-10,000** (factory 1024), **M12 5-pin push-pull/IO-Link**, magnetic | **CLOSED — with interface correction**: RU3100 is a 24 V push-pull/IO-Link encoder, NOT differential - the AM26LV32E differential receiver was the wrong device. Encoder channels now use a passive 10k divider + BAT54S clamp + 1 nF (10 us time constant, supports the 20.5 k edges/s = 48.8 us period) per channel; AM26LV32E removed from the design |
| AM26LV32E | TI: 3.3 V low-voltage RS-422 receiver | **REMOVED** (no longer required - RU3100 is single-ended); any future differential encoder would use it |
| Belt HTBN225S5M-250 | 20T/5 mm: L = 2C + 100 -> C = **62.5 mm exactly** = feed_drive_spec nominal_centres; +/-range is tensioner travel | **CLOSED — COMPLIANT** |
| Encoder count basis | RU3100 factory 1024 vs spec 2048 PPR (8192 counts/rev) | **CLOSED** - RU3100 programmable to 2048 via IO-Link; commissioning must set it and record (P04-T4) |

## Remaining items and their closure instrument (no longer "open" - each has an enforceable instrument)

1. **LC1D12BD DC-13 at 48 V** - Schneider's DC-13 table must be read at order; PO clause: "DC-13 rating at 48 VDC >= 7 A required, datasheet page supplied with delivery"; acceptance P03-T1/T2.
2. **SFU1610 C7 certificate + journal drawing** - PO clause: C7 inspection certificate + journals 12h6/10h6 + Ø8h7x18 per drawing AX-S01; receipt inspection P00-T2.
3. **HGR20 authenticity** - PO clause: genuine HIWIN HGH20CA with COC/lot traceability; reject clones (they cannot evidence C0 27.76 kN).
4. **Waste sweep / chute sensor / output takeaway** - specification + acceptance in bom/phase-h-io-occurrences.csv and P08; the parts are RFQ lines with defined interfaces (24 V actuator, 24 V optical sensor, powered takeaway), so the design is closed even though no price is quoted yet.
