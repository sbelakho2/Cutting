# 35 — Purchase enquiries: ready-to-send letters for every external-inquiry line

**Revision:** `S1-B-ENQ-R2-2026-09-16`
**Status:** ready-to-send enquiry package. Four active letters (L2, L5,
L6, L8; L1, L3, L4 and L7 resolved 2026-09-16 — L7 by the priced Wegertseder
ring substitution, L3/L4 by the priced ifm measuring-wheel/encoder
substitutions, L1 by the priced MISUMI VN C-grade axis substitution) cover
every remaining
EXTERNAL-INQUIRY quotation line of the [gate ledger](33-gate-ledger.md)
(gates 30, 32, 35, 36 and 40) and of the [quotation register in docs/24 §2.2](24-production-and-release-plan.md#22-quotation-register-for-lines-without-a-public-price-2026-09-15).
Sourcing evidence behind the letters:
[HIWIN](../engineering/sources/sourcing-findings-hiwin.md),
[axis replacement](../engineering/sources/axis-replacement-findings.md),
[R+W](../engineering/sources/sourcing-findings-rw.md),
[Kübler](../engineering/sources/sourcing-findings-kuebler.md),
[drives/rings/hardware](../engineering/sources/sourcing-findings-drive-rings.md),
[coupling/ring replacement](../engineering/sources/coupling-ring-replacement-findings.md),
[measurement replacement](../engineering/sources/measurement-replacement-findings.md);
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
3. Send L2/L5/L6 to the named recipient class (L2 to R+W dealers, L5 to a
   Rittal distributor, L6 to StepperOnline; L1, L3, L4 and L7 are resolved
   2026-09-16 — no enquiry needed). Send L8 to
   the sous-douane freight forwarder and copy two
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

## L1 — HIWIN KK8610P340A1F0 precision axis, article 10.00026, ×2 — RESOLVED 2026-09-16 (no enquiry needed)

**Resolution:** the P-grade axis has no public price on any channel (ledger
gate 31), and the priced genuine C-grade sibling **KK8610C-340A1-F0** is
adopted as the substitution. Live price read 2026-09-16 from the MISUMI VN
catalogue API (`api.vn.misumi-ec.com/api/v1/partNumber/search`,
seriesCode 221005545419, inner code MDM00005346643):
**VND 21,146,653 per unit** (USD 824.72 at 1 USD = 25,641 VND, open.er-api
2026-09-16), shelf page
[MISUMI VN detail 221005545419](https://vn.misumi-ec.com/vona2/detail/221005545419/?HissuCode=KK8610C-340A1-F0).
Availability: 23–99 days with the caution "This product is temporary NOT
available. Please ask MISUMI to check the next available ship date" —
confirm the ship date at order time. BOM row AX-01 updated; recorded in
docs/24 §2.1/§2.2, docs/32, docs/33 gate 31 and
[axis-replacement-findings](../engineering/sources/axis-replacement-findings.md).

**Substitution evaluation (accuracy grade P → C):** the C grade shares the
identical carriage, F0 flange (Ø49.97 register on PCD70), Ø8 h7/18 mm input
shaft, 46×46 M6 carriage pad, 340/440 mm rail and every load rating; only
the accuracy class changes (C axial play 0.02 mm, expected repeatability
≈±0.01 mm vs P ±0.003). The axis positions the blade, not the piece length
(length is wheel-measured, ±0.70 mm at 1 m), so the C-grade margin is ~4× on
the ±0.20 mm strip target — **no CAD, spec, drawing or check change** (full
analysis in [axis-replacement-findings](../engineering/sources/axis-replacement-findings.md) §3).

**Remaining action:** record the receipt lot and certificate in
incoming-inspection-template.csv; verify C grade, lead 10, 340/440 mm and
the Ø8 h7 shaft at incoming inspection. No letter is sent for this line.

---

## L2 — R+W MK2 bellows couplings: MK2/100/50/8/14 ×2 and MK2/100/50/10/12 ×2

**Recipient type:** R+W authorised dealer or R+W direct
(rw-couplings.com). Candidates: **ebmia.pl** (PL), **Archimedes sp. z o.o.**
(PL, B2B), **MISUMI** (sells MK2 per bore pair), **R+W** factory direct.

**Order path (2026-09-16):** ebmia.pl. Order the exact bores
MK2/100/50/8/14 ×2 and MK2/100/50/10/12 ×2 from ebmia.pl as made-to-bore
quote items at the class price; R+W direct is the fallback. The 2026-09-16
re-run of the ebmia exact-bore searches (`MK2/100/50/8/14`, `MK2/100/50/10/12`)
returned zero results again; the priced same-class alternatives found on
ebmia were all rejected (see negotiation reference and
[coupling-ring-replacement-findings](../engineering/sources/coupling-ring-replacement-findings.md)).

**Negotiation reference (do not send):** live class anchor on ebmia.pl —
MK2/100/50/10/14 at **EUR 99.43 net / 122.30 gross per pc** (stock 0,
dispatch 7–14 days) and MK2/100/50/8/21.4 at PLN 340.53 net (24 h). The
exact bores are quote items; use ≈EUR 100 net/pc as the class price level.
Rejected 2026-09-16 candidates (torque/backlash verdict recorded): ebmia
jaw couplings **SBK19-8/14** (PLN 261.69–265.31 net) and **SBK19-10/12**
(PLN 217.85–221.47 net), D40 × L66, **32 Nm ≥ 8.5 Nm (torque PASS)** but
0.5° jaw backlash = 0.0139 mm ≈ **14 µm at the 10 mm lead**, which is **56%
of the 25 µm axis positioning budget** and 4.6× the ±3 µm repeatability,
is not closed by the motor-shaft encoder, grows with insert wear, and the
L66/11 mm-hub geometry does not match the 49–50 mm free length / 16 mm hub
stack — **REJECTED**. ebmia BKL3/10/12 bellows (PLN 130.38 gross, 24 h) is
an undersized class (OD 25, L 32, hub 10, bore max 12.7) — not the
40 × 50 × 16 / 10 Nm class — **REJECTED**. Conrad (403) and automation24
(404) search remained unusable.

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
× hub 16 dimensions; no substitute without a change record (the priced jaw
alternatives were rejected on backlash — see negotiation reference); verify
the interior clearance before hub installation. The ebmia order instruction
above is the concrete order path for the exact bores.

---

## L3 — Kübler measuring wheel 8.0000.3123.0006, ×2 — RESOLVED 2026-09-16 (no enquiry needed)

**Resolution:** the brand-exact Kübler wheel has no public price or stock
channel (ledger gate 33), and a priced 100 mm-circumference substitute could
not be verified on automation24, Conrad or Amazon.de. The substitution is the
priced, in-stock **ifm E60137 measuring wheel "U:200 N:06"** — 200 mm
circumference, Ø63.66 mm, Ø6 bore, PU surface 90 Shore A on aluminium,
64.5 g — **EUR 57.00 ex VAT** (RRP 61.90), item 102756, in stock, dispatch
next working day, live-verified 2026-09-16 at
[automation24.com](https://www.automation24.com/measuring-wheel-ifm-electronic-e60137).
BOM row ME-W01 updated; recorded in docs/32, docs/33 gate 33 and
[measurement-replacement-findings](../engineering/sources/measurement-replacement-findings.md).

**Substitution evaluation:** the 200 mm circumference halves the PPR/mm
ratio (4096 counts/200 mm = 0.048828 mm/count instead of 0.024414); the
calibration model, M201/M203/fork/bracket CAD, arm-spring lug geometry and
docs/13 were re-derived for the Ø63.66 wheel envelope and every measurement,
mechanism and wire-route gate stays green. The ifm datasheet publishes no
circumference/OD tolerance and no hub/clamping-screw dimensions; verify the
200 mm claim and the hub detail at incoming inspection (runbook P0-4).

**Remaining action:** record the receipt lot and certificate in
incoming-inspection-template.csv; no letter is sent for this line.

---

## L4 — Autonics E40S6-1024-6-L-5, ×2 — RESOLVED 2026-09-16 (no enquiry needed)

**Resolution:** the brand-exact Autonics encoder has no public price (ledger
gate 34). The substitution is the priced, in-stock **ifm RU3100 incremental
encoder** — Ø6 solid shaft ×10 mm (no flat), 1024 PPR factory (IO-Link
programmable 1…10000), HTL/TTL push-pull, 4.75–30 VDC, 1000 kHz, 12000 rpm,
starting torque ≤0.01 Nm, body Ø58.6×63.5 mm, synchro flange with
3×M4×0.7×6 mm on PCD42, M12 5-pole radial/axial connector —
**EUR 186.40 ex VAT** (RRP 207.10), item 102717, in stock, dispatch next
working day, live-verified 2026-09-16 at
[automation24.com](https://www.automation24.com/incremental-encoder-ifm-electronic-ru3100)
([datasheet](../engineering/sources/ifm-ru3100-datasheet.pdf) archived;
the RUP500 display variant at EUR 255.50 was not needed). BOM row ME-E01
updated; M201 plate redesigned to R2 (60×60, Ø36.5-class synchro-flange seat,
3×Ø4.5 on PCD42, 4×M4 taps); recorded in docs/32, docs/33 gate 34 and
[measurement-replacement-findings](../engineering/sources/measurement-replacement-findings.md).

**Remaining action:** confirm the synchro-flange centering feature and shaft
fit at incoming inspection, and review the J8/J9 line-receiver wiring for
the TTL/HTL outputs (open controls item, findings file); no letter is sent
for this line.

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

## L7 — SEEGER-ORBIS A12 retaining rings, ×8 — RESOLVED 2026-09-16 (no enquiry needed)

**Resolution:** the brand-exact SEEGER-ORBIS A12 has no live priced channel
(ABSENT on every channel tried), and the priced DIN 471 12×1 Federstahl
equivalent is adopted as the substitution: **Wegertseder Art. 1950-010
"12 x 1"**, Kleinpaket **100 pcs EUR 8.57 incl 19 % VAT** (= EUR 0.086/pc),
in stock, live-verified 2026-09-16 at
[wegertseder.com](https://www.wegertseder.com/DIN-471-Sicherungsringe-fuer-Wellen-Regelausfuehrung-FEDERSTAHL-Diameter-12).
BOM row TR-RR01 updated; recorded in docs/32, docs/33 gate 37 and
[coupling-ring-replacement-findings](../engineering/sources/coupling-ring-replacement-findings.md).

**Fit verdict (groove Ø11.45±0.02):** ring d3 = 11.0 mm, s = 1.0 (−0.06).
Install spread 1.0 mm (11.0 → 12.0 shaft); seated spread ≈0.45 mm at the
groove bottom — since d3 < groove bottom (11.43–11.47) always, the ring
grips the groove (no radial float). Seated OD ≈13.45 mm → ≈0.72 mm radial
protrusion per side. Axial float = groove width 1.12±0.02 (1.10–1.14) −
ring 0.94–1.00 → **0.10–0.20 mm** — acceptable; no function depends on a
tight A12 seat (docs/28 finding 7). ZGA-1 pliers per the Seeger catalogue.

**Remaining action:** record the receipt lot and certificate in
incoming-inspection-template.csv; no letter is sent for this line. The
smallest pack (100 pcs) covers the ×8 requirement with spare.

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
| 1 | HIWIN KK8610C-340A1-F0 (C-grade substitution, MISUMI VN detail 221005545419, VND 21,146,653/unit) | 2 |
| 2 | R+W MK2/100/50/10/12 | 2 |
| 3 | R+W MK2/100/50/8/14 | 2 |
| 4 | ifm E60137 measuring wheel (U:200 N:06) | 2 |
| 5 | ifm RU3100 incremental encoder | 2 |
| 6 | Rittal SK3303.500 | 1 |
| 7 | STEPPERONLINE CL57T-V41 | 2 |
| 8 | STEPPERONLINE LE-350-36 | 1 |
| 9 | Wegertseder 1950-010 DIN 471 12×1 Federstahl rings (A12 substitution, 100-pc pack) | 8 |
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
| ISO 4762 M4×12 class 8.8 (ME-FB01) | 6 |
| ISO 4762 M10×35 class 8.8 (FB306, into DIN 929 nuts) | 8 |
| ISO 4762 M4×6 class 8.8 (FB301) | 4 |
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
| ISO 7092 M5 200HV (mechanism BOM) | 32 |
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
- [docs/33](33-gate-ledger.md) — gates 30–36 and 40 (closure action: send the matching letter above; gate 37 closed by substitution).
- [HIWIN findings](../engineering/sources/sourcing-findings-hiwin.md), [R+W findings](../engineering/sources/sourcing-findings-rw.md), [Kübler findings](../engineering/sources/sourcing-findings-kuebler.md), [drive/rings findings](../engineering/sources/sourcing-findings-drive-rings.md), [coupling/ring replacement findings](../engineering/sources/coupling-ring-replacement-findings.md), [catalogue evidence](../engineering/sources/catalogue-evidence.md).
- BOM identity and quantity rows: [transport/cabinet](../bom/selected-transport-and-cabinet.csv), [cutting head](../bom/selected-cutting-head.csv), [controls](../bom/selected-controls.csv), [mechanism BOM](../engineering/generated/mechanism/mechanism-bom.csv).
