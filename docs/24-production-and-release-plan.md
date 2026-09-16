# Master production and release plan — S1-B

**Revision:** `S1-B-PROD-R3-2026-09-16`
**Status:** source-controlled build plan only. It authorises the internal
fabrication build; energisation, commissioning and operation remain gated
by the evidence rules below and the consolidated
[warnings](#10-warnings-and-holds-consolidated).

**To build the machine, start at the [fabrication guide](31-fabrication-guide.md)**
and return here for the evidence rules, gate table and warnings. A work
package is closed only when its named evidence is complete, reviewed and has
a recorded disposition. Warnings are consolidated in
[section 10](#10-warnings-and-holds-consolidated); the body below is the
execution path.

## 1. Configuration baseline and build record

1. Assign one unique `build_id`; copy its revision set, selected source files,
   generated output manifests and approved deviations into the build folder.
2. Use [the production build record template](inspection/production-build-record-template.csv)
   to record every work package, input identity, actual result, reviewer and
   nonconformance reference. Never substitute a generic "built to print"
   statement for the required evidence.
3. Freeze the exact selections in [the BOM authority](../bom/README.md), the
   source-generated [IF-B01 draft package](../hardware/if-b01/manufacturing/draft-r2-2026-09-13)
   and this plan. A changed part, material, process, drawing or software image
   requires an approved change record and repeat of every affected check.
4. Run the repository integrity checks before releasing build inputs:
   `tools/check_production_records.py`,
   `tools/check_if_b01_gpio_mapping.py`,
   `tools/check_mechanism_outputs.py`, the IF-B01 native parity checker and
   [the link check](../tools/check_links.zsh). Their passing status establishes
   only source consistency.

## 2. Procurement and incoming control

1. Purchase only an exact manufacturer/order code from an approved selection.
   A `HOLD-` or `UNSELECTED` row is not an order line.
2. Consult [the auditable sourcing evidence](../bom/sourcing-evidence.csv).
   It separates direct-page observations from inherited BOM links and records
   where a public price was not observed. The two rows marked below displayed
   RRP compare a selling price only with the RRP printed on that same product
   page; they are **not** supplier or landed-cost comparisons.
3. Compare lower-cost offers only when the part number/revision, quantity,
   condition, tax basis, freight/duty basis, destination and lead time are
   equivalent. Record all terms before calling an offer lower cost. The
   required calculation is

   $$C_{\mathrm{landed}} = Qp + f + d + t + b,$$

   where $Q$ is quantity, $p$ unit price, $f$ freight, $d$ duties, $t$ taxes
   and $b$ any brokerage/handling cost. No such landed-cost comparison exists
   in this revision.
4. At receipt, record exact item identity, quantity, lot/serial number,
   certificate/CoC, visible condition and the incoming measurement against
   [the incoming-inspection template](inspection/incoming-inspection-template.csv).
   Quarantine mismatches; do not silently substitute or mix lots.

### 2.1 Observed merchandise price register (2026-09-13, updated 2026-09-16)

Direct public-page observations added 2026-09-13; merchandise prices only,
no freight/duty/tax terms, and never a delivered-cost claim. Sourcing rows in
[bom/sourcing-evidence.csv](../bom/sourcing-evidence.csv) carry the exact
comparison boundaries.

| BOM row | Item | Observed | Page |
|---|---|---|---|
| CT-K1-K3 | Schneider TeSys Deca LC1D12BD (×3) | EUR 70.40 ex VAT (page RRP 108.00); in stock | [automation24.com](https://www.automation24.com/main-contactor-schneider-electric-tesys-lc1d12bd) |
| TR-B01 | SKF 6001-2Z/C3 (×8) | GBP 6.32 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/6001-2z-c3-skf) |
| TR-B02 | SKF 6002-2Z/C3 (×8) | GBP 6.91 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/6002-2z-c3-skf) |
| ME-B01 | SKF 626-2Z (×4) | GBP 4.49 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/626-2z-skf) |
| TR-N01 | SKF KM 2 (×8) | GBP 7.90 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/km2-skf) |
| TR-W01 | SKF MB 2 (×8) | GBP 2.15 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/mb2-skf) |
| MC-B03 | SKF 6204-2Z/C3 (×2, payoff spindle) | GBP 8.68 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/6204-2z-c3-skf) |
| CB-PS24 | Siemens LOGO!POWER 6EP3332-6SB00-0AY0, 24 V 2.5 A (item 103609) | EUR 68.60 ex VAT (page RRP 80.60); in stock, dispatch next working day | [automation24.com/logo-power-supply](https://www.automation24.com/logo-power-supply) |
| TH-C01 | Rittal SK 3303.500 cooler, 230 V 0.55 kW | EUR 1,669.00 incl VAT; 14 in stock | [conrad.com](https://www.conrad.com/en/p/rittal-sk-3303-500-cooler-230-v-0-55-kw-w-x-h-x-d-280-x-550-x-210-mm-1-pc-s-2200714.html) |
| CB-TB24-01/02/03, CB-TB-S | Phoenix Contact PT 2,5 terminals 3209510 (grey) / 3209523 (blue) | EUR 1.29 each incl VAT; 2794/604 in stock | [3209510](https://www.conrad.com/en/p/phoenix-contact-3209510-continuity-5-2-mm-quick-connect-grey-1-pc-s-733311.html) · [3209523](https://www.conrad.com/en/p/phoenix-contact-3209523-continuity-5-2-mm-quick-connect-blue-1-pc-s-733325.html) |
| CB-TB-M1-M3 | Phoenix Contact UT 4 3044131 | EUR 2.29 each incl VAT; 289 in stock | [conrad.com](https://www.conrad.com/en/p/phoenix-contact-3044131-continuity-8-2-mm-screws-grey-1-pc-s-744402.html) |
| CB-TB-DL | Phoenix Contact 3046032 fuse serial terminal (5×20) | EUR 7.99 incl VAT; 34 in stock | [conrad.com](https://www.conrad.com/en/p/phoenix-contact-3046032-fuse-serial-terminal-6-2-mm-screws-black-1-pc-s-744510.html) |
| CB-XPS-FUSE / CB-TB-DL-K1-K3 | ESKA 522620 2 A / 522617 1 A T 5×20 fuse links | EUR 1.49 each incl VAT; 1368 / 224 in stock, fast track dispatch Fri 18 Sep | [522620](https://www.conrad.com/en/p/eska-522620-522620-dc-fuse-o-x-l-5-mm-x-20-mm-2-a-250-v-dc-time-delay-t-content-1-pc-s-1414701.html) · [522617](https://www.conrad.com/en/p/eska-522617-522617-dc-fuse-o-x-l-5-mm-x-20-mm-1-a-250-v-dc-time-delay-t-content-1-pc-s-1414698.html) |
| CB-AC-INCOMING (RCD half) | Schneider Acti9 iID 2P 25 A 30 mA type A A9Z21225 | EUR 35.99 incl VAT; 5 in stock, fast track dispatch Fri 18 Sep | [conrad.com](https://www.conrad.com/en/p/schneider-electric-a9z21225-a9z21225-rccb-a-25-a-0-03-a-240-v-1861630.html) |
| CB-ENC-ENTRY | LAPP Skintop MS-M 20 EMC 53112532 brass gland | EUR 17.99 each incl VAT; 52 in stock | [conrad.com](https://www.conrad.com/en/p/lapp-53112532-cable-gland-emc-m20-brass-brass-1-pc-s-1601941.html) |
| TR-RR01 alt. | Simply Bearings d1400-012, 12 mm external circlip (DIN471-class) | GBP 0.85 | [simplybearings.co.uk](https://www.simplybearings.co.uk/products/d1400-012) |
| AX-01 | HIWIN KK8610C-340A1-F0 complete axis (×2, C-grade substitution for KK8610P340A1F0, 2026-09-16) | VND 21,146,653 per unit (= USD 824.72, open.er-api 2026-09-16); 23–99 days, "temporarily NOT available — ask MISUMI for next ship date" | [MISUMI VN shelf](https://vn.misumi-ec.com/vona2/detail/221005545419/?HissuCode=KK8610C-340A1-F0) — live price read 2026-09-16 from the MISUMI VN catalogue API (`api.vn.misumi-ec.com/api/v1/partNumber/search`, inner code MDM00005346643); evidence in [axis-replacement-findings](../engineering/sources/axis-replacement-findings.md) |

Quality-equivalent same-designation alternatives were observed on the same
page family (FAG, KOYO); they are recorded in
[catalogue evidence](../engineering/sources/catalogue-evidence.md) and
require an approved change record before any substitution.

### 2.2 Quotation register for lines without a public price (2026-09-15)

No reachable public retail price exists for the following lines; each has an
exact quotation request defined so no procurement line is left without its
next step. All requests must record part number, quantity, unit price,
currency, freight/duty terms, lead time and validity.

| BOM rows | Exact request | Decision basis |
|---|---|---|
| TR-AX01 / AX-01 | HIWIN KK8610P340A1F0, article 10.00026, ×2 — RESOLVED 2026-09-16 (priced P→C substitution adopted) | Substituted by the genuine C-grade sibling **KK8610C-340A1-F0** on the MISUMI VN shelf ([detail 221005545419](https://vn.misumi-ec.com/vona2/detail/221005545419/?HissuCode=KK8610C-340A1-F0)): live price **VND 21,146,653** per unit (USD 824.72 converted) read 2026-09-16 from the MISUMI VN catalogue API; 23–99 days with a "temporarily NOT available — ask MISUMI" caution; identical carriage/F0/Ø8 h7/440 mm interfaces → no CAD change (accuracy analysis in [axis-replacement-findings](../engineering/sources/axis-replacement-findings.md)); verify C grade and interfaces at incoming inspection |
| TR-CP01 / CP-01 | R+W MK2/100/50/10/12 ×2 and MK2/100/50/8/14 ×2 (exact ordering codes per [catalogue evidence](../engineering/sources/catalogue-evidence.md)); exact bores have NO live price (23-channel search, [findings](../engineering/sources/sourcing-findings-rw.md)); live anchor: MK2/100/50/10/14 at EUR 99.43 net/pc on [ebmia.pl](https://www.ebmia.pl/sprzegla-mieszkowe-bkl/213877-sprzeglo-bezluzowe-mk2-100-50-10-14.html?SubmitCurrency=1&id_currency=2) | Any R+W dealer can order the exact bores at the class price; confirm OD 40 × L 50 × hub 16 dims (verified) and no substitute without a change record |
| ME-W01 | Kübler measuring wheel 8.0000.3123.0006 ×2 — RESOLVED 2026-09-16 | Substituted by ifm E60137 (200 mm, EUR 57.00 ex VAT automation24, in stock); verify the 200 mm claim and hub detail at incoming inspection |
| ME-E01 | Autonics E40S6-1024-6-L-5 ×2 — RESOLVED 2026-09-16 | Substituted by ifm RU3100 (EUR 186.40 ex VAT automation24, in stock); M201 R2 plate redesign; J8/J9 receiver review CLOSED 2026-09-16 (TTL mode via IO-Link + 2.5 V mid-bias wiring; docs/22 §6/§7) |
| TH-C01 | Rittal SK3303.500 ×1 | Request from a Rittal distributor; confirm the 230 V 50 Hz variant |
| TR-D01 | STEPPERONLINE CL57T-V41 ×2; exact string has no live price (StepperOnline 403); cheapest live class price EUR 173.78 ([amazon.de/dp/B0FYPDGC85](https://www.amazon.de/dp/B0FYPDGC85), 2–3 days) | Record the StepperOnline shop price at purchase (usually cheaper); class fallback above |
| TR-P01 | STEPPERONLINE LE-350-36 ×1; exact string absent online; cheapest live 36 V/350 W equivalent EUR 77.04 ([amazon.de/dp/B0HDN818SJ](https://www.amazon.de/dp/B0HDN818SJ)); wholesale S-350-36 class US $12.70–14.90 MOQ 1 (made-in-china) | Record the StepperOnline shop price at purchase; alternatives above |
| TR-RR01 | SEEGER-ORBIS A12 ×8; brand-exact has no live price; verified DIN 471 12×1 rings 100 pcs EUR 8.57 in stock ([Wegertseder Art. 1950-010](https://www.wegertseder.com/DIN-471-Sicherungsringe-fuer-Wellen-Regelausfuehrung-FEDERSTAHL-Diameter-12)); Amazon MZA EUR 0.62/pc; Simply Bearings GBP 0.85/pc | Any DIN 471 12×1 federstahl ring meets the groove; brand substitution needs a change record |
| Standard hardware (TR-K01; all TR-FB*/TR-FW*/TR-FD*/ME-FB*/ME-FW* rows; the ISO4762/ISO4032/ISO7092/DIN913/DIN929 rows of the mechanism BOM) | Priced anchors in stock at [Wegertseder](https://www.wegertseder.com/): ISO4762 M5×16 class 10.9, 50 pcs EUR 6.19 (Art. 2540-034); ISO 8734-class 3m6×20 hardened dowels (DIN 6325 m6), 10 pcs EUR 2.73 (Art. 1018-078) | Commodity purchase-to-requirement; record certificate and lot at receipt |
| CB-AC-INCOMING (MCB half) | Schneider Acti9 iC60N 2P C10, A9F54210 ×1 | No reachable public price 2026-09-16 (automation24 category/search unreachable to automated fetch, Conrad 1P/3P/4P iC60N only, ebmia 2P B-curve only, se.com 403); confirm on a live distributor page at purchase |
| CT-PS36 | 6EP3332-7SB00-0AX0 ×1 | Record the distributor page at order time |
| CT-IF-U1 / CT-IF-U2-U3 | Exact USB-isolator/interface identities per selected-controls.csv | Confirm availability before the board order |

Every row above remains a `HOLD-`/unpriced line until its quotation and
incoming inspection are recorded; none may be substituted silently.

**Tunisia sous-douane delivery step (2026-09-16):** send the complete
quotation-register line list (exact SKUs + quantities above) as one RFQ to
the sous-douane freight forwarder and to two distributors that ship to
Tunisia. Require each offer to quote: unit USD price, currency and payment
terms, EXW/FOB seller terms, freight to the forwarder's consolidation point,
forwarder handling, air/sea freight, destination charges and customs terms
(DDU sous-douane), lead time and validity. No public page provides this
pricing; only that RFQ can establish it. Record the offers in the build
record before placing orders.

## 3. Mechanical fabrication and inspection

1. Begin with material certification and blank identification. Machine only
   the controlled part files for the selected build.
2. Use [the feed cartridge record](12-feed-cartridge-fabrication.md) for
   F101–F112, including matched bearing measurement, the specified machining
   sequence and the still-open local-stress/preload/thermal holds. Use
   [the F101 inspection record](inspection/F101-R6.csv) without prefilled
   results.
3. Use [the measuring interface record](13-measuring-interfaces-and-calibration.md)
   for M201 and its required metrology. The full mechanism source and its
   generated manifest define the additional carrier, guide, measuring and
   frame parts; their custom-part release states remain authoritative in
   [the mechanism custom BOM](../bom/mechanism-custom-parts.csv).
4. Do not machine or accept a custom part solely because CAD exports a valid
   solid. Finish, inspect, identify and record every critical dimension,
   surface, fit and material requirement stated by its governing record.

**Mechanical gate:** halt before final assembly if a drawing/record is absent,
if a part is not within its declared acceptance criterion, or if the F101
regional stress, housing/collar contact, preload or thermal release holds are
still applicable to the proposed use.

## 4. Mechanical assembly, guards and payoff

1. Assemble cartridges only in the sequence in the feed record. Keep matched
   bearing/housing/shaft identifiers together and retain the inspection data.
2. Assemble the head, carriers, guides, measuring module and welded frame from
   the controlled generated outputs. Repeat the CAD/check source validation
   after a source change; it is not a substitute for physical free-motion,
   alignment or torque evidence.
3. The [guard/payoff selection record](26-guard-and-payoff-selection.md) adds
   explicit R2 material and component selections on top of the
   [preliminary static accommodation](25-preliminary-guard-and-payoff.md);
   build steps are in the [fabrication guide](31-fabrication-guide.md) §3.7–3.8.
4. No blade pair is procured or qualified by this plan. `T001` remains
   purchase-to-drawing and requires specialist manufacture plus loaded
   clearance verification before installation.

**Mechanical gate:** no guarded movement, blade operation or wire handling is
permitted until physical guard, containment, access, payoff/tension and blade
reviews have passed their separate records.

## 5. IF-B01 manufacture and fixture control

1. Use only the current source-generated IF-B01 package after human electrical
   and fabricator review. Its clean ERC/DRC/native-parity evidence does not
   release the panel, land patterns, assembly or interface.
2. Verify received PCB, stencil and assembled component identities against the
   package manifest and the board BOM. Record solder/process inspection and
   all deviations in the build record.
3. Test an unconnected board using the isolated procedure in
   [the IF-B01 fixture record](../hardware/if-b01/if-b01-test.md) and only
   `if-b01-factory-test.uf2`. Keep J4–J7 disconnected from production motor
   drivers throughout fixture testing.
4. Archive actual fixture logs and serial/lot identities. A missing or failed
   result prevents cabinet integration.

## 6. Cabinet construction and de-energised inspection

1. Do not purchase or install a cabinet until the explicitly held enclosure,
   PE, gland, conductor and remaining terminal identities and the
   local-regulation design review are released by competent review. The
   incoming-AC identity (iC60N 2P C10 A9F54210 + iID 2P 25 A 30 mA
   A9Z21225), the XPS/output fuse solution (3046032 terminal + 2 A 5×20
   link) and the DC branch-protection decision were selected on 2026-09-16
   and still require that review plus the commissioning checks of
   [docs/22 §10](22-controls-and-wiring.md).
2. Follow the controlled sequence in
   [the cabinet electrical assembly procedure](23-cabinet-electrical-assembly.md).
   It is a de-energised plan, not an energisation permission.
3. Preserve the hardwired safety boundary: IF-B01, firmware, USB, `GP15`,
   PIO abort and watchdog features are diagnostic/control measures, not safety
   functions. Do not create an E1/E2 guard unlock path while the dedicated
   release-permission design is unresolved.
4. Record continuity, separation, PE bonding, insulation, terminal marking,
   torque method, actual protection devices and point-to-point verification.

**Electrical gate:** cabinet energisation remains prohibited until every
applicable release hold in [the controls design input](22-controls-and-wiring.md)
and [the wiring BOM](../bom/selected-cabinet-wiring.csv) has approved,
traceable evidence.

## 7. Firmware, dry commissioning and energy checks

1. Build and identify the intended `starz-machine.uf2` production image. The
   factory UF2 is never installed as machine firmware.
2. Before connecting loads, validate the released source allocation with
   [the GPIO allocation](../hardware/if-b01/if-b01-gpio-allocation.csv). Then
   conduct the approved staged electrical procedure under competent oversight.
3. Verify actual voltages, phase/polarity, field contacts, encoder direction,
   drive enable behavior, interlocks, fault response and restart prevention.
   Record the actual hardware and instruments; do not extrapolate from host
   tests.
4. Before a guard-release function could be considered, measure K1/K2/K3
   opening behavior, DC-bus decay and regenerated-energy behavior and complete
   the safety-function review. This build plan does not authorize that work.

## 8. First article, capability and release disposition

1. Complete the mandatory first-article and energisation evidence in
   [the FAI/energisation gate template](inspection/first-article-and-energisation-gate-template.csv).
2. Perform the physical procedure in
   [physical qualification](20-physical-qualification.md): 30 pieces for each
   mandatory wire/length cell, gauge identities, force checks, environment,
   Cpk and raw-data retention. The qualification tool is intentionally
   fail-closed for blank or failing data.
3. Record nonconformances with containment, root cause, corrective action,
   reviewer and re-verification. Rework or substitution invalidates affected
   dimensional, electrical, software and qualification evidence until repeated.
4. A responsible reviewer may issue a future release only after all applicable
   evidence is complete. This repository contains no completed first article,
   physical qualification, safety validation or fabrication/energisation
   authorization.

## 9. Open simulation and evidence gates

| Area | Current source result | Required closure before relying on it for release |
|---|---|---|
| Feed-shaft FEM | Whole-shaft exterior-centroid closure (2026-09-16, [findings](../engineering/sources/centroid-closure-findings.md)): a 24-configuration probe sweep (torus feature subsets, 0.02–0.04 mm torus roots, growth 0.3–0.6, both optimizers) showed the gmsh volume optimizer cannot remove the last concave-surface sliver tetrahedra inside the unchanged 150000-node cap (best 2 exterior at 148510 nodes) while the Netgen volume optimizer eliminates same-surface tetrahedra entirely; the gmsh four-mesh series (2/.30, 1.6/.20, 1.2/.13, 1/.09 mm, `concave_fillet_tori`, growth 0.45, all loads, 48 phases, torque vertices, gravity and every gate unchanged) was rerun with the Netgen volume optimizer — 0 exterior same-surface and 0 exterior peak centroids at 129936 ≤ 150000 nodes, byte-identical to the untouched Netgen series, so the fail-closed CAD boundary audit gate passes; the [regional submodel record](28-feed-shaft-submodel-record.md) converges the ten remaining regional rows (roller shoulder, M15 relief, roller key root, pulley key/MB2 slot; working+fault; both optimizers) between 0.02% and 4.0% last-change at 0.02→0.01/0.0175 mm sector roots (≤150000 nodes/window) with exact global displacement transfer and Pilkey Kt cross-checks where applicable; M15 peak documented as a real bounded slot-wall∩groove corner; the residual whole-shaft continuous CAD-containment audit was executed 2026-09-16 ([findings](../engineering/sources/containment-audit-findings.md)): exact BRep point-in-solid classification of all 1,125,721 element centroids of the four levels at 1×10⁻⁷ mm finds **32 exterior / 0 on-boundary** (0/24/3/5 per level), all facet slivers on the concave fillet tori (faces 68, 7, 9, 61, 63, 71, 72), gaps 0.0071–5.51 µm | The ten regional raw-peak holds and the physical preload/contact/fatigue qualifications remain; `continuous_CAD_containment_verified` stays false (32 exterior centroids, reported not hidden); no gate, load, phase or node cap was relaxed |
| Adapter FEM | Resolved-pressure elastic-foundation model: the ideal uniform patch is replaced by a C¹-smooth quartic profile p(r)=p_peak·(1−s²)² (s=(r−3.625)/0.875) on each counterbore-floor washer seat r∈[2.75,4.5]mm — zero with zero slope at both annulus edges, peak376.33MPa at r=3.625mm, mean p0=200.71MPa so the4×8.0kN per-seat resultant is unchanged — and the stage-side Winkler foundation shares the same resolved profile (peak kn=94,759.7N/mm³, kt=kn/2.67=35,490.5N/mm³; total per-seat stiffness preserved at the2014.41MN/m rigid-punch half-space calibration); unchanged6/4/2.8/2/1.4/1.0mm series plus the isolated pair, supernodal CHOLMOD,150,000-node cap. The patch-edge singularity is removed (pair peak285.987MPa sits on the counterbore floor0.59mm inside the washer-OD edge, not at a patch edge), but the gate pair (1.0mm background→0.7mm within8mm) still measures18.4573% raw peak and4.9551% displacement — the resolved seat features live on the1.75mm seat annulus, the order of the pair's1.0mm background element, so the pair records feature-scale discretisation, not a singularity. A seat-scale focused pair of the same global model (0.15mm within3mm→0.1mm within2mm of the peak seat,1.5mm bulk,64,944nodes) refines321.67→312.71MPa, last-change2.86%≤5%, bounding the model peak at≈313MPa; stiffness sensitivity×0.5/×2 moves the peak+0.17% | The gate rows remain **REFINE** (never relaxed): the global pair cannot resolve the≈1mm seat features. The remaining gap is physical, not numerical — the resolved profile shape and the rigid-punch foundation stiffness are calibrated models, not measurements; the peak≈313MPa is preload seat compression below the650MPa certified yield but above the yield/3 screen216.7MPa (screen FAIL). A measured washer/joint contact stiffness or a two-body ISO7092 washer contact solve is the remaining justified step before release use ([findings](../engineering/sources/adapter-contact-findings.md), [report](../engineering/generated/adapter-r6-analysis/report.md)) |
| Bearings, joints and thermal behavior | Desk parts closed 2026-09-16 ([bearing-contact-thermal study](../engineering/generated/bearing-contact-thermal/report.md)): housing compliance — the 6 µm matched interference resolves to 17.65 MPa seat contact pressure (seat vm 22.7 MPa vs 650 MPa, ring 82.3 MPa vs ≈1700 MPa); the record's F112 bore clearance carries no fit pressure (bore-wall 0.98 MPa vs 260 MPa under the resolved 310 N); the compliant chain changes the seat stress ≤3.9 MPa absolute vs the four-mesh rigid boundary — not material (**PASS**). Thermal field — 30 W standstill holding loss gives a 29.66 K front-seat rise vs the record's 20 K shaft-above-housing envelope at the central stated seat conductance → not met (**HOLD**) | Measure/justify preload, contact, housing compliance and the seat temperature field on the actual build; instrumented cold-start/hot-run test (docs/12:38); record the motor-case temperature at rated hold (docs/28 §2.5) |
| Guard/payoff | [R2 selections](26-guard-and-payoff-selection.md) exist for panels, door, spool, spindle, bearings and friction brake; R2 CAD source changes and physical validation are pending | Complete risk assessment, Bernstein mounting evidence, load/runaway measurement, physical build and validation |
| Electrical/safety | Logical topology, source checks, branch/wiring/regen/discharge/thermal calculations, the corrected XPS fuse selection and the 2026-09-16 closures (5×20 XPS fuse set, AC incoming iC60N 2P C10 + iID 2P 25 A 30 mA, DC branch-protection decision) ([docs/22 §11](22-controls-and-wiring.md)); LC1D12BD DC duty awaits the manufacturer answer | Close DC switching confirmation, discharge measurement, PE, enclosure, EMC and safety-function evidence; confirm the unpriced incoming MCB at purchase |
| Process qualification | Procedure and fail-closed tool exist; all repository templates are blank | Execute controlled physical tests and retain raw records |

No row in this table may be closed by relaxing a numerical acceptance criterion,
deleting failed raw data or relabelling source outputs as measured results.

## 10. Warnings and holds (consolidated)

All remaining warnings, honesty boundaries and physical holds in one place:

1. **Source checks are not physical results.** A blank record, a simulated
   result or a supplier web page is a hold—not a pass. A `HOLD-` or
   `UNSELECTED` row is not an order line.
2. **Landed cost.** No such landed-cost comparison exists in this revision;
   the §2.1 register lists merchandise observations only, and §2.2 lists
   the quotation requests for unpriced lines.
3. **Safety functions.** IF-B01, firmware, USB, `GP15`, PIO abort and
   watchdog features are diagnostic/control measures, not safety functions;
   only the hardwired contactor/XPS path is the safety function, and it is
   unvalidated.
4. **Electrical.** Cabinet energisation is prohibited until every docs/22
   §10 hold has evidence; the LC1D12BD DC duty answer is outstanding; the
   XPS fuse is an ESKA 522620 2 A T 5×20 link in a Phoenix 3046032 fused
   terminal, never 10 A.
5. **Fabrication.** Torque values are not published for M5/M10 joints
   (preload study pending); bearing seats are matched to measured bores;
   the blade pair is purchase-to-drawing and edge-unqualified.
6. **Simulation state.** Adapter raw-peak gate is REFINE (pressure-patch
   edge; contact analysis with measured joint stiffness pending); the
   whole-shaft exterior-centroid audit is closed (0 exterior same-surface
   centroids in both four-mesh series, 2026-09-16) but the stricter
   continuous CAD-containment audit of every element centroid reports
   32 exterior / 0 on-boundary of 1,125,721 ([findings](../engineering/sources/containment-audit-findings.md)), so
   `continuous_CAD_containment_verified` remains false; the ten regional
   raw-peak holds and all preload/contact/fatigue/thermal qualifications
   remain held; no FEM number releases a part.
7. **Guard and payoff.** Risk assessment, Bernstein mounting evidence,
   full-spool weighing (≤30 kg until re-checked), spindle deflection and
   runaway/brake behaviour require physical measurement; the 30 mm apertures
   pass the desk distance calculation but require risk-assessment sign-off.
8. **Release.** `physical_results`, `fabrication_released`,
   `full_machine_integrated` remain false everywhere; this plan is
   source-controlled build guidance only.