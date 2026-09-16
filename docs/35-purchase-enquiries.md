# 35 — Purchase enquiries: ready-to-send letters for every external-inquiry line

**Revision:** `S1-B-ENQ-R1-2026-09-16`
**Status:** ready-to-send enquiry package. Eight letters (L1–L8) cover every
EXTERNAL-INQUIRY quotation line of the [gate ledger](33-gate-ledger.md)
(gates 30–37 and 40) and of the [quotation register in docs/24 §2.2](24-production-and-release-plan.md#22-quotation-register-for-lines-without-a-public-price-2026-09-15).
Sourcing evidence behind the letters:
[HIWIN](../engineering/sources/sourcing-findings-hiwin.md),
[R+W](../engineering/sources/sourcing-findings-rw.md),
[Kübler](../engineering/sources/sourcing-findings-kuebler.md),
[drives/rings/hardware](../engineering/sources/sourcing-findings-drive-rings.md);
exact identities and quantities come from the
[BOM CSVs](../bom/README.md) ([transport/cabinet](../bom/selected-transport-and-cabinet.csv),
[cutting head](../bom/selected-cutting-head.csv),
[controls](../bom/selected-controls.csv),
[mechanism BOM](../engineering/generated/mechanism/mechanism-bom.csv)).

## Usage note

1. Fill the sender block (company name, contact person, e-mail/phone,
   invoice address) at the top of each letter before sending.
2. The delivery address type for every letter is a **Tunisia sous-douane
   company**: a named company with a Tunisian address that receives the
   goods under sous-douane (duty-suspension) arrangements; the seller's
   delivery term to be quoted is **DDU sous-douane**.
3. Send L1–L7 to the named recipient class (L1 to two independent
   distributors). Send L8 to the sous-douane freight forwarder and copy two
   distributors that ship to Tunisia.
4. Record every response (unit price USD, currency, EXW/FOB, freight to the
   consolidation point, forwarder handling, air/sea freight, destination
   charges, customs/DDU, lead time, validity) in the
   [production build record](inspection/production-build-record-template.csv)
   before ordering. A partial reply is recorded as partial and is never
   compared as a landed cost.
5. No letter authorises purchase. A line stays `HOLD-`/unpriced until its
   quotation **and** [incoming inspection](inspection/incoming-inspection-template.csv)
   are recorded and the letter's decision criteria are met (docs/24 §2).

## Common required response fields

Every reply must state, per line: unit price in **USD** and the offer
currency; currency and payment terms; **EXW/FOB** seller terms (incoterm and
named place); freight to the forwarder's **Tunisia sous-douane consolidation
point**; forwarder handling; air/sea freight; destination charges and
customs terms (**DDU sous-douane**); lead time (availability/dispatch); and
offer validity period. Each letter below repeats this list so it can be sent
on its own.

---

## L1 — HIWIN KK8610P340A1F0 precision axis, article 10.00026, ×2

**Recipient type:** authorised HIWIN distributor. Send this letter to TWO
independent distributors; candidates from the search record: **Rodavigo**
(ES), **Proax** (CA), and the HIWIN regional offices (hiwin.de / hiwin.fr)
for a distributor referral. MISUMI VN/TH is the shelf channel (see
comparison leads below).

**Buyer comparison leads (do not send):** eBay item
[406408632737](https://www.ebay.com/itm/406408632737) "Hiwin KK8610P-340
Precision Linear Ball Screw Actuator, 10 mm Lead, 215 mm Stroke" at
US $450 — snippet-only, page 403 to automation, NOT a verified price; and
MISUMI VN/TH [detail 221005545419](https://vn.misumi-ec.com/vona2/detail/221005545419)
listing the C-grade sibling KK8610C-340A1-F0 (price never read). Both are
leads to re-check from a browser, not accepted offers.

**Parts requested:**

| Part number | Manufacturer article | Quantity |
|---|---|---|
| KK8610P340A1F0 | HIWIN article 10.00026 | 2 |

The quote must confirm: genuine HIWIN KK86 single-axis robot, accuracy grade
**P**, lead **10 mm**, rail **340 mm** / overall **440 mm**, one standard
block, no cover, input shaft **Ø8 h7** with 18 mm exposed (HIWIN catalogue
PS-11-1-EN-2605-K, 2026-05-22, archived). State whether the units are new or
used.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM AX-01/TR-AX01, docs/24 §2.2, ledger gate 31):** the
axis stays unsubstituted unless a change record approves — a C-grade, clone
or different rail length is not an acceptable substitute. Compare offers
against the eBay/MISUMI leads from a browser. Verify article, lead and P
grade at incoming inspection.

---

## L2 — R+W MK2 bellows couplings: MK2/100/50/8/14 ×2 and MK2/100/50/10/12 ×2

**Recipient type:** R+W authorised dealer or R+W direct
(rw-couplings.com). Candidates: **ebmia.pl** (PL), **Archimedes sp. z o.o.**
(PL, B2B), **MISUMI** (sells MK2 per bore pair), **R+W** factory direct.

**Negotiation reference (do not send):** live class anchor on ebmia.pl —
MK2/100/50/10/14 at **EUR 99.43 net / 122.30 gross per pc** (stock 0,
dispatch 7–14 days) and MK2/100/50/8/21.4 at PLN 340.53 net (24 h). The
exact bores are quote items; use ≈EUR 100 net/pc as the class price level.

**Parts requested:**

| Part number | Quantity |
|---|---|
| R+W MK2/100/50/8/14 | 2 |
| R+W MK2/100/50/10/12 | 2 |

The quote must confirm: size 100 = **10 Nm** TKN, outer **Ø40 mm**, free
length **50 mm**, hub **16 mm**, bore range 5–24 mm, clamp screws **M4 @
4.5 Nm**, bores **8 H7/14 H7** (8/14) and **10 (motor D-flat)/12 H7**
(10/12). Additionally requested (ledger gate 30): interior clearance
data/CAD for MK2/100/50/8/14 so the hub installation clearance can be
verified before assembly.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM TR-CP01/CP-01, docs/24 §2.2, ledger gates 30/32):**
10 Nm TKN must exceed the 8.5 Nm motor holding torque; confirm OD 40 × L 50
× hub 16 dimensions; no substitute without a change record; verify the
interior clearance before hub installation.

---

## L3 — Kübler measuring wheel 8.0000.3123.0006, ×2

**Recipient type:** Kübler authorised distributor (via the kuebler.com
distributor channel). Candidates: no distributor is named for this wheel in
the search record; request a distributor referral from Kübler direct
(kuebler.com). Family price-class reference: Conrad carries the Kübler
Perambulator 05.2400.0040.1000.5045 at EUR 395 incl VAT (family reference,
not the selected part).

**Parts requested:**

| Part number | Quantity |
|---|---|
| Kübler 8.0000.3123.0006 | 2 |

The quote must confirm the catalogue identity: wheel No. **123**, natural
rubber **60 Shore A**, **Ø31.83 ±0.025** (free circumference **100 ±0.1
mm**), tread width **9.5 mm** / overall width **14.5 mm**, bore **6 mm
(H7)**, set screw **M4×6 ISO 4029**. State whether the type is stocked or
made to order.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM ME-W01, docs/24 §2.2, ledger gate 33):** compare
distributor offers against the alternative of keeping the archived wheel;
the 100 ±0.1 mm circumference claim stays unverified until incoming
inspection.

---

## L4 — Autonics E40S6-1024-6-L-5, ×2

**Recipient type:** Autonics authorised distributor. Candidates: none named
in the repository search record; automation24.com (which carries the priced
ifm alternatives below) is a candidate channel and referral point.

**Priced alternatives (buyer-side, do not send):** ifm RU3100 (Ø6, 1024,
TTL/HTL) EUR 185.50 and ifm RUP500 (Ø6 synchro, 1024, display) EUR 255.50,
both observed at automation24. Used only to judge the Autonics offer, not
automatic substitutes.

**Parts requested:**

| Part number | Quantity |
|---|---|
| Autonics E40S6-1024-6-L-5 | 2 |

The quote must confirm the exact suffix: **Ø6 shaft (−0.004/−0.016)**,
**1024 P/R**, line driver outputs **A,Ā,B,B̄,Z,Z̄**, **5 VDC ±5 %**, **3×M3
DP5 on PCD30**, pilot **Ø20 (−0.021/0)**, body Ø40×31 mm, shaft 15 + pilot 5
(EN product manual TCD210019AD, 2025-07-21).

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM ME-E01, docs/24 §2.2, ledger gate 34):** verify the
shaft and pilot tolerances at receipt against the measurement
specification (the shaft-tolerance/pilot-tolerance deltas recorded as a FLAG
in the mechanical-documents findings stay open); the ifm alternatives are
priced fallbacks only.

---

## L5 — Rittal SK3303.500 filter cooler, ×1

**Recipient type:** Rittal authorised distributor or Rittal direct
(rittal.com). Candidates: **Conrad** (channel where the merchandise
reference was observed) and the Rittal regional office for a distributor
referral.

**Buyer reference (do not send):** Conrad listing EUR 1,669 incl VAT, 14 in
stock (merchandise only, no freight/duty terms).

**Parts requested:**

| Part number | Quantity |
|---|---|
| Rittal SK3303.500 | 1 |

The quote must confirm: filter cooler **230 V, 0.55 kW**, dimensions W×H×D
**280×550×210 mm**, 280 W at L35, and the **230 V 50 Hz** variant.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM TH-C01, docs/24 §2.2, ledger gate 35):** the 230 V
50 Hz variant is required; Conrad EUR 1,669 incl VAT is the merchandise
reference only.

---

## L6 — STEPPERONLINE CL57T-V41 ×2 and LE-350-36 ×1

**Recipient type:** StepperOnline direct (official shop
omc-stepperonline.com / stepperonline.com; AliExpress storefront 1955025).

**Buyer anchors (do not send):** CL57T class EUR 173.78 (amazon.de
[B0FYPDGC85](https://www.amazon.de/dp/B0FYPDGC85), dispatch 2–3 days);
36 V/350 W class EUR 77.04 (amazon.de
[B0HDN818SJ](https://www.amazon.de/dp/B0HDN818SJ), ERP-350-36) and
wholesale US $12.70–14.90 MOQ 1 (made-in-china S-350-36).

**Parts requested:**

| Part number | Quantity |
|---|---|
| STEPPERONLINE CL57T-V41 | 2 |
| STEPPERONLINE LE-350-36 | 1 |

The quote must confirm: CL57T-V41 closed-loop stepper driver, **V4.1
revision**, 0–8.0 A, 24–48 VDC, NEMA 17/23/24; LE-350-36 PSU, **36 V /
9.7 A / 350 W**, 115/230 V. State the shop price for the exact strings and
the driver firmware/revision.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM TR-D01/TR-P01, docs/24 §2.2, ledger gate 36):**
record the official shop price at purchase (usually below the marketplace
anchors); verify driver revision and authenticity at arrival; the class
fallbacks are documented but not automatic substitutes.

---

## L7 — SEEGER-ORBIS A12 retaining rings, ×8

**Recipient type:** SEEGER-ORBIS authorised distributor (brand-exact).
Candidates: none found in the search record (brand-exact ABSENT on every
channel tried); the DIN 471 equivalents below are the buyer anchors.

**Buyer anchors (do not send):** Wegertseder Art. 1950-010 "12×1"
Federstahl, 100 pcs EUR 8.57 incl VAT, in stock; Amazon MZA
[B00IZC9GVO](https://www.amazon.de/dp/B00IZC9GVO) EUR 0.62/pc; Simply
Bearings d1400-012 GBP 0.85/pc.

**Parts requested:**

| Part number | Quantity |
|---|---|
| SEEGER-ORBIS A12 (DIN 471 for 12 mm shaft, 12×1: s = 1.0 mm, d3 = 11.0 mm) | 8 |

Request the smallest pack that covers 8 pieces; confirm spring steel
(Federstahl) and DIN 471 12×1.

**Required response fields:** unit price USD + offer currency; currency and
payment terms; EXW/FOB terms; freight to the Tunisia sous-douane
consolidation point; forwarder handling; air/sea freight; destination
charges and customs (DDU sous-douane); lead time; validity.

**Delivery address type:** Tunisia sous-douane company (DDU sous-douane).

**Decision criteria (BOM TR-RR01, docs/24 §2.2, ledger gate 37):** shaft 12,
ring 1 (−0.06), groove 11.5 (−0.11), groove width min 1.10, ZGA-1 pliers.
Any DIN 471 12×1 Federstahl ring meets the groove — brand substitution needs
a change record.

---

## L8 — MASTER RFQ to the sous-douane forwarder: one consolidated landed-cost quote for ALL lines

**Recipient type:** the **sous-douane freight forwarder** (Tunisia),
addressed to the selected forwarder; copy two distributors that ship to
Tunisia. This is the single RFQ consolidating the complete quotation-register
line list of docs/24 §2.2 (merchandise observations only exist for some
lines; no landed-cost comparison exists anywhere in the repository).

**Full SKU/quantity list:**

| # | Line | Quantity |
|---|---|---|
| 1 | HIWIN KK8610P340A1F0 (article 10.00026) | 2 |
| 2 | R+W MK2/100/50/10/12 | 2 |
| 3 | R+W MK2/100/50/8/14 | 2 |
| 4 | Kübler 8.0000.3123.0006 | 2 |
| 5 | Autonics E40S6-1024-6-L-5 | 2 |
| 6 | Rittal SK3303.500 | 1 |
| 7 | STEPPERONLINE CL57T-V41 | 2 |
| 8 | STEPPERONLINE LE-350-36 | 1 |
| 9 | SEEGER-ORBIS A12 (DIN 471 12×1) | 8 |
| 10 | Standard hardware (traceable industrial supplier), full list below | per row |
| 11 | Schneider Acti9 iC60N 2P C10 A9F54210 (AC incoming MCB half) | 1 |
| 12 | Siemens 6EP3332-7SB00-0AX0 (CT-PS36 register row) | 1 |
| 13 | Texas Instruments AM26LV32EIDR / TPL7407LDR (CT-IF-U1 / CT-IF-U2-U3) | 1 / 2 |

Line 10 — standard hardware (rows per
[selected-transport-and-cabinet.csv](../bom/selected-transport-and-cabinet.csv),
[selected-cutting-head.csv](../bom/selected-cutting-head.csv) and the
[mechanism BOM](../engineering/generated/mechanism/mechanism-bom.csv)):

| Fastener | Quantity |
|---|---|
| C45 DIN 6885-A 5×5×28 key (TR-K01) | 4 |
| ISO 4762 M5×16 class 10.9 (TR-FB01) | 8 |
| ISO 4762 M5×16 class 10.9 (HF0-B, head) | 8 |
| ISO 4762 M5×25 class 10.9 (HM-B, head) | 8 |
| ISO 4762 M4×20 class 10.9 (TR-FB02) | 16 |
| ISO 4762 M3×8 class 8.8 (TR-FB03) | 32 |
| ISO 4762 M3×10 class 8.8 (ME-FB01) | 6 |
| ISO 4762 M10×35 class 8.8 (FB306, into DIN 929 nuts) | 8 |
| ISO 4762 M4×10 class 8.8 | 16 |
| ISO 4762 M4×16 class 8.8 | 4 |
| ISO 4762 M5×16 class 8.8 | 4 |
| ISO 4762 M5×20 class 8.8 (lower + upper housing) | 32 |
| ISO 4762 M5×25 class 8.8 | 2 |
| ISO 4762 M5×35 class 8.8 | 4 |
| ISO 4762 M6×20 class 8.8 | 4 |
| ISO 4762 M6×25 class 8.8 | 4 |
| ISO 4762 M8×25 class 8.8 | 4 |
| ISO 4762 M8×55 class 8.8 | 8 |
| ISO 7092 M5 200HV 5.3×9×1 (HM-W, head) | 16 |
| ISO 7092 M4 200HV 4.3×8×0.5 (TR-FW02) | 16 |
| ISO 7092 M3 200HV 3.2×6×0.5 (ME-FW01) | 6 |
| ISO 7092 M5 200HV (mechanism BOM) | 32 |
| ISO 7092 M3 200HV (mechanism BOM) | 3 |
| ISO 8734 3m6×20 hardened dowels (TR-FD02) | 16 |
| ISO 8734 10m6×24 / 6m6×20 / 6m6×22 / 6m6×70 dowels (mechanism BOM) | 2 / 1 / 1 / 1 |
| ISO 4032 M8 class 8 lock nut | 2 |
| ISO 4032 M6 class 8 lock nut | 1 |
| DIN 913 M4×8 45H insert set screw | 2 |
| DIN 913 M6×25 45H flat-point contact-force adjuster | 1 |
| DIN 913 M8×40 45H flat-point adjuster | 2 |
| DIN 929 M10 hexagon weld nut | 8 |

**Required DDU line items:** quote EACH line separately and ONE consolidated
DDU total. Per line: unit price in **USD** and the offer currency; currency
and payment terms; **EXW/FOB** seller terms (incoterm and named place);
freight to your **Tunisia sous-douane consolidation point**; forwarder
handling; air/sea freight; destination charges and customs terms (**DDU
sous-douane**); lead time; validity. State the consolidated weight/volume
and any dangerous-goods or long-item surcharges (the HIWIN 440 mm axis).

**Delivery address type:** Tunisia sous-douane company — delivery to the
forwarder's Tunisia consolidation point with DDU sous-douane terms.

**Decision criteria (docs/24 §2.2 Tunisia step, docs/32 "Delivery to
Tunisia" section, ledger gate 40):** landed cost
$C_{\mathrm{landed}} = Qp + f + d + t + b$ (unit price × qty + freight +
duties + taxes + brokerage/handling). Compare the consolidated DDU offer
against direct DHL/FedEx economy offers from the clickable shops. Record
every offer in the build record before ordering; no public page provides
this pricing, so only this RFQ establishes it.

---

## Sources

- [docs/24 §2.2](24-production-and-release-plan.md#22-quotation-register-for-lines-without-a-public-price-2026-09-15) — quotation register and the Tunisia sous-douane step.
- [docs/32](32-sourcing-list.md) — clickable sourcing list and the Tunisia USD delivery note.
- [docs/33](33-gate-ledger.md) — gates 30–37 and 40 (closure action: send the matching letter above).
- [HIWIN findings](../engineering/sources/sourcing-findings-hiwin.md), [R+W findings](../engineering/sources/sourcing-findings-rw.md), [Kübler findings](../engineering/sources/sourcing-findings-kuebler.md), [drive/rings findings](../engineering/sources/sourcing-findings-drive-rings.md), [catalogue evidence](../engineering/sources/catalogue-evidence.md).
- BOM identity and quantity rows: [transport/cabinet](../bom/selected-transport-and-cabinet.csv), [cutting head](../bom/selected-cutting-head.csv), [controls](../bom/selected-controls.csv), [mechanism BOM](../engineering/generated/mechanism/mechanism-bom.csv).
