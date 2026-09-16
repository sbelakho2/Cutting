# Master production and release plan — S1-B

**Revision:** `S1-B-PROD-R1-2026-09-13`  
**Status:** source-controlled build plan only. It is **not** an authority to
fabricate, energise, commission, operate, certify or sell the machine.

This plan gives one simple, evidence-driven order for converting the S1-B
source package into a future build. It joins the selected BOM records,
generated CAD, inspection templates and qualification procedure without
turning any source check into a physical acceptance result. A work package is
closed only when its named evidence is complete, reviewed and has a recorded
disposition. A blank record, a simulated result or a supplier web page is a
hold—not a pass.

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

### 2.1 Observed merchandise price register (2026-09-13)

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
| TR-AX01 | HIWIN KK8610P340A1F0, article 10.00026, ×2; request from two independent HIWIN distributors (e.g., regional HIWIN sales office and a second authorized distributor); also request the KK86 accessories needed (none selected beyond the axis) | Compare against the R1/R2 architecture cost and lead time; keep the axis unsubstituted unless a change record approves |
| TR-CP01 | R+W MK2/100/50/10/12 ×2 and MK2/100/50/8/14 ×2 (exact ordering codes per [catalogue evidence](../engineering/sources/catalogue-evidence.md) and archived [ordering page](../engineering/sources/rw-mk2-ordering.png)); rw-couplings.com shop reached but prices are not server-rendered; request from R+W or a distributor | Hub bore pairs and lengths must match the motor flats and shaft diameters; no substitute without a change record |
| ME-W01 | Kübler measuring wheel 8.0000.3123.0006 ×2 | Compare any distributor offer against the alternative of keeping the archived wheel; the 100±0.1 mm circumference claim stays unverified until receipt |
| ME-E01 | Autonics E40S6-1024-6-L-5 ×2 | Request from an Autonics distributor; no public price was observed |
| TH-C01 | Rittal SK3303.500 ×1 | Request from a Rittal distributor; confirm the 230 V 50 Hz variant |
| TR-D01 | STEPPERONLINE CL57T-V41 ×2 | Direct shop price visible to a human; record it at purchase time (site blocks automated retrieval) |
| TR-P01 | STEPPERONLINE LE-350-36 ×1 | Same as above |
| TR-RR01 | SEEGER A12 DIN471 ×8 | Any SEEGER-ORBIS distributor; the archived catalogue governs geometry |
| Standard fasteners (all TR-FB*/TR-FW*/TR-FD*/ME-FB*/ME-FW* rows) | ISO4762/7092/8734 class-specified, purchase from a traceable industrial fastener supplier | Commodity purchase-to-requirement; record certificate and lot at receipt |

Every row above remains a `HOLD-`/unpriced line until its quotation and
incoming inspection are recorded; none may be substituted silently.

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
   [preliminary static accommodation](25-preliminary-guard-and-payoff.md).
   Actual guard material, manufacturer mounting evidence, fasteners, wire
   spool, spindle/brake, retained energy and access route remain physically
   unvalidated before fabrication. Do not claim a safe guard from a
   clearance calculation.
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

1. Do not purchase or install a cabinet until the explicitly held terminal,
   PE, enclosure, gland, conductor, fuse-holder, branch-protection, incoming
   AC and thermal selections are released by competent review.
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
| Feed-shaft FEM | Torus-focused four-mesh reruns left 13 gmsh / 10 Netgen regional holds; the [regional submodel record](28-feed-shaft-submodel-record.md) then converges all eleven regional rows (roller shoulder, M15 relief, roller key root, pulley key/MB2 slot; working+fault; both optimizers) between 0.02% and 4.0% last-change at 0.02→0.01/0.0175 mm sector roots (≤150000 nodes/window) with exact global displacement transfer and Pilkey Kt cross-checks where applicable; M15 peak documented as a real bounded slot-wall∩groove corner | The four-mesh raw-peak rows are preserved unaltered; the 34-exterior same-surface centroid whole-shaft audit and the physical preload/contact/fatigue qualifications remain |
| Adapter FEM | Preloaded elastic-foundation support model re-run: 4×8.0kN F0 preload pressure on the counterbore-floor washer seats r∈[2.75,4.5]mm plus a Winkler foundation on the stage-side seat annuli (kn=50,538.5N/mm³ half-space calibration, kt=kn/2.67), supernodal CHOLMOD, unchanged6/4/2.8/2/1.4/1.0mm series plus the isolated pair; displacement refinement2.15931% passes3%; raw peak16.9981% remains **REFINE** against5%; peak219.489MPa fails the yield/3 screen (local preload seat compression, inside the640.7MPa Lamé bound); stiffness sensitivity×0.5/×2 moves the peak+0.46% | The ideal-clamp singularity is removed, but the raw peak now sits at the counterbore-floor washer-OD pressure-patch edge and still grows182.11→219.49MPa under peak-region refinement; a true contact analysis with measured joint stiffness (or a resolved seat pressure profile) is the remaining justified step |
| Bearings, joints and thermal behavior | Analytical geometry and nominal fits are present | Measure/justify preload, contact, housing compliance and thermal field for the actual build |
| Guard/payoff | [R2 selections](26-guard-and-payoff-selection.md) exist for panels, door, spool, spindle, bearings and friction brake; R2 CAD source changes and physical validation are pending | Complete risk assessment, Bernstein mounting evidence, load/runaway measurement, physical build and validation |
| Electrical/safety | Logical topology, source checks, branch/wiring/regen/discharge/thermal calculations and the corrected XPS fuse selection ([docs/22 §11](22-controls-and-wiring.md)); LC1D12BD DC duty awaits the manufacturer answer; protection, PE, enclosure identities proposed | Close protection, DC switching confirmation, discharge measurement, PE, enclosure, EMC and safety-function evidence |
| Process qualification | Procedure and fail-closed tool exist; all repository templates are blank | Execute controlled physical tests and retain raw records |

No row in this table may be closed by relaxing a numerical acceptance criterion,
deleting failed raw data or relabelling source outputs as measured results.