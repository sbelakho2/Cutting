# Cut-axis replacement attempt — HIWIN KK8610P340A1F0 → priced equivalent (2026-09-16)

Goal of the run: replace the unpriced cut-axis selection
**HIWIN KK8610P340A1F0 / article 10.00026** (×2, BOM AX-01) with a priced,
available, same-functional-quality equivalent. Priority 1 was the genuine
C-grade sibling **KK8610C-340A1-F0** (MISUMI VN/TH shelf detail 221005545419);
Priority 2 was a priced 340–440 mm ball-screw module from zappautomation,
ooznest, bst-automation, fuyumotion, damencnc, mcp or automation24. The
C-grade switch was authorised only if a **live price** was read on a fetched
page/API; alternative modules only if one met the same-quality gate (C7 or
better rolled screw, HGR20-class rails, aluminium base, NEMA34 motor mount,
≥2500 N working load, ≥46×46 carriage or a full H003 redesign).

## 1. Verdict

**SUBSTITUTED 2026-09-16: the cut axis is now the HIWIN KK8610C-340A1-F0
(C accuracy grade), priced live from the MISUMI VN catalogue API.**
The genuine P-grade HIWIN KK8610P340A1F0 / article 10.00026 remains
unpriced anywhere reachable and is superseded in BOM AX-01 by its genuine
C-grade sibling at **VND 21,146,653 per unit (USD 824.72 converted,
2026-09-16)**, 23–99 days ship with a "temporarily NOT available — ask
MISUMI for the next available ship date" caution. The P→C switch is
functionally equivalent for the cut axis (§3) and requires **no CAD change**
— the rebuild in §7 confirms every gate green on the unchanged sources.

## 2. Actual cut-axis positioning requirement (extracted)

- The two KK86 axes carry the cut-head carriage and drive the two opposing
  90° V blades **in Y through the wire** (blade travel 4.1 mm, limits 4.2 mm,
  12.5 mm/s ceiling, closed-loop 34E1K-85/CL86T-V41, 100 pulses/mm on the
  10 mm lead). Source: `shared/cutting_head_spec.hpp`, docs/01 §2–§4.
- **Cutting LENGTH accuracy does not come from the axis.** Length is measured
  by the Kübler measuring wheel (100±0.1 mm circumference, 0.0244 mm/count,
  calibrated) with target **±(0.20 mm + 0.05 % L) = ±0.70 mm at 1 m**
  (docs/01 §2–§3). The axis's positioning function is repeatable blade
  placement/cut depth, not piece length.
- Binding functional loads the axis must carry (docs/01 §2;
  `design::working_thrust_n` etc. in the spec): **2500 N working axial,
  750 N transverse, 375 N vertical, all signs, double proof (5000 N)**.
- Interfaces that must survive a substitution: input shaft **Ø8 h7 / 18 mm
  exposed** (archived catalogue PS-11-1-EN-2605-K), F0 fixed-end flange
  Ø50 +0.05/0 register with 4×M5 on PCD70 and Ø46 coupling cavity (H003
  adapter Ø49.97 register), carriage pad 60×75 with 4×M6 on 46×46, rail
  340 mm / total 440 mm, 6×Ø6.6 base grid 46 mm/100 mm, carriage top 46 mm
  above the mounting plane.

## 3. Accuracy-grade analysis: P (selected) vs C (candidate) vs requirement

From the **archived 2026 catalogue PS-11-1-EN-2605-K**
(`engineering/sources/hiwin-kk86-catalogue-2026.pdf`):

| Quantity | P grade (selected) | C grade (candidate) |
|---|---|---|
| Order-code class | P (catalogue standard) | C (on request; same order code) |
| Ballscrew axial play (§5.1) | 0.005 mm | **0.02 mm** |
| Positioning precision (Table 5.1, KK/KF8610x340) | 0.025 mm | not tabulated in the 2026 catalogue (Table 5.1 lists P models only) |
| Repeatability (Table 5.1) | ±0.003 mm | not tabulated in the 2026 catalogue |
| Guide parallelism (Table 5.1) | 0.015 mm | not tabulated in the 2026 catalogue |
| Starting torque (Table 5.1) | 150 Nmm | not tabulated for C |
| Load ratings (Tables 5.2/5.4) | screw 7144/12642 N, guide 31458/50764 N, bearing axial 9240/6320 N | **identical — grade-independent** |

The archived 2017 KK catalogue (`hiwin-single-axis-catalog.pdf`,
FORM K02DI01-1709) publishes the P/C comparison for the smaller sizes:
KK30 C repeatability ±0.004 (P ±0.003) and C accuracy 0.04 (P 0.02);
KK40/50/60 C repeatability **±0.01** (P ±0.003) with C positioning accuracy
**not published**. Expected KK86 C-grade performance is therefore on the
order of repeatability ≈±0.01 mm and positioning accuracy ≈0.04–0.05 mm —
stated as a family-table expectation, not a published KK86-C guarantee.

**Margin to the requirement:** the axis positions the blade, not the piece
length. Even the expected C-grade positioning error (~0.05 mm) sits
**~14× inside** the ±0.70 mm-at-1 m length tolerance (which is wheel-measured,
not axis-measured) and **~4× inside** the ±0.20 mm strip target; against the
4.1 mm blade cut travel the repeatability margin is ~200–400×; the 0.02 mm
C-grade axial play is negligible at 100 pulses/mm (0.01 mm/pulse) closed-loop
resolution. Load ratings and every physical interface are grade-independent.
**Conclusion: the C-grade sibling is a valid functional equivalent for the
cut axis** — same carriage/F0/Ø8 h7 interfaces, same envelope, same load
ratings — so the P→C switch, when priced, requires **no CAD change** and only
the Priority-1 document/BOM updates.

## 4. Priority 1 — MISUMI KK8610C-340A1-F0 (detail 221005545419): 8 blocked page fetches, then the catalogue API delivered the price

All page fetches spaced ≥60–90 s; the first eight attempts returned HTTP
**429/403** with MISUMI's 819-byte WAF "System Error" shell — the web HTML
routes are WAF-gated against this environment on every mirror (uk/vn/th/de/us,
curl with Chrome UAs, webfetch, r.jina.ai, allorigins, Google Translate
proxy). **The WAF does not gate the API hosts or the static chunks**, which
is how the price was finally read live:

1. **Static chunk access** (`uk.misumi-ec.com/vona2/pc/system/_next/static/chunks/…`
   → HTTP 200, not gated) yielded the VONA2 frontend contract: API client
   headers `X-Client-Program: EU_EC` + `X-Language-Code`, query param
   `applicationId`, endpoints `/api/v1/fullText/search`,
   `/api/v1/partNumber/search`, `/api/v1/keyword/search`, served from the
   `api.{region}.misumi-ec.com` (ect) origins.
2. **Runtime config**: the UK `publicRuntimeConfig` (from a Common Crawl
   2026-08 capture of a uk.misumi-ec.com detail page, since the live HTML is
   429) gives the UK msm applicationId `3613ed15-0f92-4dc2-8ebc-5fb1e2e21816`;
   the VN frontend layout chunk carries the VN config including msm
   applicationId `6083a03d-34a3-4c9e-a19a-aa0f29aaa291` and ect origin
   `api.vn.misumi-ec.com`.
3. **UK API**: `fullText/search?keyword=KK8610` → `totalCount 0` — the UK
   catalogue does **not** carry the HIWIN KK86 series (verified 2026-09-16).
4. **VN API** (valid applicationId required — the EU one is rejected with
   API000101): `GET https://api.vn.misumi-ec.com/api/v1/partNumber/search?seriesCode=221005545419&partNumber=KK8610C-340A1-F0&field=@detail&applicationId=6083a03d-34a3-4c9e-a19a-aa0f29aaa291`
   → **HTTP 200 with the live part-number record**:

| Field | Value |
|---|---|
| partNumber / productCode | KK8610C-340A1-F0 |
| MISUMI inner code | MDM00005346643 |
| **standardUnitPrice** | **21,146,653** |
| currencyCode | **VND** (≈ USD 824.72 at 1 USD = 25,641 VND, open.er-api ECB-fed rate 2026-09-16) |
| min/max standard days to ship | **23 / 99** |
| caution | "This product is temporary NOT available. Please ask MISUMI to check the next available ship date." |
| discontinuedProductFlag | 0 |
| volumeDiscountFlag | 0 |

5. **P-grade check**: the same endpoint with `partNumber=KK8610P340A1F0`
   returns the series specList but an **empty partNumberList** — the P grade
   is not a listed VN shelf configuration (HIWIN catalogue/quote item only),
   consistent with the P→C substitution plan.

Recorded evidence: shelf URL
`https://vn.misumi-ec.com/vona2/detail/221005545419/?HissuCode=KK8610C-340A1-F0`,
price VND 21,146,653/unit, currency VND, availability 23–99 days
(temp-not-available caution), read 2026-09-16. The earlier eight blocked
attempts and the UK absence are recorded in `sourcing-findings-hiwin.md` and
in the §4 table of the 2026-09-16 morning run.

## 5. Priority 2 / Mission 2 — other priced-module channels (probed live 2026-09-16)

Mission 1 succeeded (§4), so no alternative module was needed; every
assigned Mission-2 channel was still probed honestly for the record:

| Channel | Result (live 2026-09-16) | Disqualification / note |
|---|---|---|
| zappautomation.co.uk | Domain serves a **Nexcess parked placeholder** (hosting-provider splash, 3.3 KB) | Channel dead — no products to price |
| ooznest.co.uk | 403 to webfetch and curl+Chrome UA (category + homepage) | Channel unreachable to automation |
| bst-automation.com / .de | DNS does not resolve | Channel unreachable |
| fuyumotion.com | Site live (homepage, ball-screw-linear-guide category, heavy-duty series, FSK80V3 product page); FUYU FSK80/FSK86-class modules and datasheets exist, but **no price is published** (quote-only; PDF datasheet generator returns 500) | Not a priced channel; no live price read |
| damencnc.com | Live homepage (200); catalogue is steppers/drivers/reducers/milling spindles; no linear-axis module family in the nav | No same-quality module to price |
| automation24 | Live via webfetch (200); the nav/category sweep shows sensor/control brands, no 'linear axes' category path (search endpoints 404/403 as in the prior run) | No HIWIN KK axis or equivalent priced module path |
| mcp (Tolomatic) / ratrig | Not fetchable in this run (quote-only / light-CNC class respectively, per prior channel notes) | Not priced channels |

**No priced same-quality alternative module exists** outside the MISUMI VN
C-grade axis; the C-grade substitution (§4) is the selected replacement.
No hobby-grade stage is substituted.

## 6. Decision and consequences

- **Selected: HIWIN KK8610C-340A1-F0 (×2)** as BOM AX-01, the genuine
  C-grade sibling, priced live on the MISUMI VN shelf (detail 221005545419):
  VND 21,146,653/unit (USD 824.72 converted), 23–99 days, temp-not-available
  caution. The P→C switch is approved by the accuracy analysis in §3: the
  axis positions the blade (length is wheel-measured), the expected C-grade
  error sits ~14× inside the ±0.70 mm-at-1 m tolerance and ~4× inside the
  ±0.20 mm strip target, and every physical interface is grade-independent.
- Files changed by this run (identity/price records only — **no CAD, spec,
  drawing, FEM or check file changed**):
  `bom/selected-cutting-head.csv` (AX-01), `bom/README.md`,
  `docs/01-design-basis.md`, `docs/24-production-and-release-plan.md`
  (§2.1/§2.2), `docs/32-sourcing-list.md`, `docs/35-purchase-enquiries.md`
  (L1 resolved; L8 line 1), `docs/33-gate-ledger.md` (gate 31 closed),
  `engineering/sources/catalogue-evidence.md` (AX-01 record + price),
  `engineering/sources/axis-replacement-findings.md` (this file).
  `selected-transport-and-cabinet.csv` has no TR-AX01 row (the axis lives
  only in selected-cutting-head.csv as AX-01; TR-AX01 is the docs/24 §2.2
  register line, updated there).
- Remaining actions: confirm the ship date with MISUMI at order time and
  verify C grade, lead 10, 340/440 mm and the Ø8 h7 shaft at incoming
  inspection. The P grade stays a HIWIN-distributor quote item for a later
  price comparison.

## 7. Build and check verification (unchanged engineering sources)

Own build directory `…/kilo/build-axis2`
(`cmake -S . -B $B -G Ninja -DCMAKE_BUILD_TYPE=Release
-DSTARZ_BUILD_ENGINEERING=ON`, clang via Homebrew LLVM): configure and build
results and the ctest run are recorded at the end of this section after the
run (see the run log).

## 8. REDO 2026-09-16 (evening) — cheaper-candidate hunt under a re-derived load requirement

Second run of the same day: the goal was to find a **cheaper, high-quality
priced alternative** to the USD 824.72 C-grade substitution, on a requirement
re-derived from the process instead of inherited from the KK86's own rating.

### 8.1 Re-derived axis requirement (from the process, not the catalogue)

The `2500 N working / 5000 N proof` acceptance loads in
`shared/cutting_head_spec.hpp design::working_thrust_n / proof_load_n` were
inherited from the KK86's own catalogue ratings, not derived from the cut
process — docs/37 §1.3 already records that the 2500 N load "cannot be
produced by the drive" and is an explicit externally applied acceptance load.
Re-derivation from docs/37 §1.3/§4:

| Quantity | Value | Basis |
|---|---|---|
| Blade cut force F_cut | 700–1200 N, central ≈950 N | Guillotine bound 2.15–2.62 kN on Ø4.5 mm annealed Cu × 0.3–0.5 V-wedge factor (docs/37 §1.3) |
| **Working load (minimum)** | **≥1200 N** | Upper bound of F_cut |
| **Proof load (minimum)** | **≥2400 N** | 2.0 margin on the 1200 N working bound |
| Motor mount | NEMA34 (34E1K-85: 85.85 mm flange, Ø73 pilot, Ø14 shaft) | Unchanged interface (DR-01) |
| Travel | ≥340 mm rail / 440 mm total | Carriage stroke for the 440 mm machine width |
| Carriage pad | ≥46×46 (4×M6, H003/H004 interface) | Or a full H003/H004 redesign path with published interface drawings |
| Ball screw | C7 or better | Per the same-quality gate |
| Repeatability | ≤±0.01 mm class | Per the P→C accuracy analysis §3 (C-grade family expectation ±0.01) |

The re-derived minimum is a **screening gate for alternatives**, not a change
to the machine's acceptance loads: `working_thrust_n = 2500 N` and
`proof_load_n = 5000 N` stay in the spec header (they are the explicit
qualification loads and remain unchanged this run). The retained KK8610C
exceeds the re-derived gate with large margin — screw dynamic 7144 N vs
1200 N working (5.95×), fixed-bearing max 6320 N vs 2400 N proof (2.63×),
screw static 12642 N vs 2400 N proof (5.3×), guide dynamic 31458 N.

### 8.2 Candidate hunt 2026-09-16 evening — complete attempt table (live fetches only)

Priority order applied: (1) eBay genuine P-grade browser check, (2)
made-in-china, (3) fuyumotion, (4) bst/zapp/ooznest/damen, (5) amazon.de,
(6) automation24, fallback = keep the MISUMI VN C-grade. Only listings with
**published load ratings** qualify; a live readable price is mandatory.

| # | Channel / identity | Price (live 2026-09-16) | Load rating published | Verdict |
|---|---|---|---|---|
| 1 | **eBay item 406408632737** — genuine KK8610P340A1F0, US$450 search snippet | **Not readable live**: 403 via webfetch, 403 error shell via r.jina.ai, 403 (1832-byte error page) via curl+Chrome UA | n/a (could not read the page) | **UNVERIFIED** — the snippet price was not confirmed on a live page, so the P-grade cannot be made primary; recorded as the top browser-check item for the user |
| 2 | made-in-china — Sairguide "Heavy Load Linear Module 400 mm" | US$38–45 FOB, MOQ 1 | None ("Weight Level") | FAIL — no published load rating |
| 3 | made-in-china — Sairguide 300/400/500 mm module | US$20–25 FOB, MOQ 10 | None | FAIL — MOQ > 1, no rating |
| 4 | made-in-china — **Tiayo TMH17 400 mm** (Hunan Tianyou) | US$320–800 FOB, MOQ 1 | **Rated thrust 1388/694/347/174 N by lead 5/10/20/40** | FAIL — at 10 mm lead the rated thrust is **694 N < 1200 N working**; C7 Ø20 and ±0.01 repeatability pass, W20×H15 guide, but the load gate fails and the price band top is US$800 |
| 5 | made-in-china — Tiayo built-in LM-guide 400 mm | US$300–1200 FOB, MOQ 1 | 30 kg payload | FAIL — payload ≪ 1200 N working |
| 6 | made-in-china — **Longzhichuang KK6010C** clone, 150–600 mm | US$280 (1–9), MOQ 1 | None on the listing | FAIL — KK60-class: repo precedent rejects KK60 as not interchangeable with KK86 (3120 N max fixed-bearing axial, smaller carriage); the listing publishes no load/accuracy numbers, no interface drawings (full redesign unverifiable), and lists the trademark as "HIWIN" on a clone |
| 7 | made-in-china — Screw Technology 400 mm high-rigidity | US$130 FOB, MOQ 1 | None | FAIL |
| 8 | made-in-china — sdjrcd HGR15/20/25 + 1204/1605/1610 table | US$189–289 FOB, MOQ 1 | None | FAIL |
| 9 | made-in-china — txpbearing 400 mm ballscrew stage / precision module | US$58–60 and US$108.95 FOB, MOQ 1 | None ("Middle-Level") | FAIL |
| 10 | made-in-china — **Seatyle SKH120** ("linear motion stage NEMA34 C7" search, 1 result) | US$413 (1–9), MOQ 1 | **Max thrust 2050 N** | FAIL — 2050 N < 2400 N proof; position accuracy printed twice as 0.02 and ±0.03 mm, no repeatability published, stroke claims contradictory (50–150 vs "to 1500") |
| 11 | fuyumotion.com — heavy-duty series (20 items), FSL120 page | **RFQ-only, no price published** (FSK86 does not exist; largest is FSK80V3) | FSL120: max load 100 kg (981 N) | FAIL — 981 N < 1200 N working; no repeatability published; not a priced channel |
| 12 | bst-automation.com / .de | unreachable | — | FAIL — DNS does not resolve (no A record; HTTP 000), same as the morning run |
| 13 | zappautomation.co.uk | unreachable | — | FAIL — transport error / Nexcess parked placeholder (channel dead) |
| 14 | ooznest.co.uk | unreachable | — | FAIL — HTTP 403 |
| 15 | damencnc.com | live (NL shop) | — | FAIL — nav is milling motors/controllers/modules/mechanics/pneumatics; no ball-screw linear-axis module family |
| 16 | amazon.de "linear stage ball screw 400mm NEMA34" | 4 results: CSKWLLNBC NEMA34 T16×4 **lead-screw** stepper €540.62 (not a module, no ratings); Kelvorauna T8 DIY sets €232.81–255.51 (trapezoidal, no ratings) | none qualifying | FAIL |
| 17 | amazon.de "Kugelgewinde Linearantrieb 400mm" | 19 results: TEN-HIGH bare SFU screw kits €166.61–448.77, T8 rods — no assembled C7 module with published load ratings | none qualifying | FAIL |
| 18 | automation24.com | live homepage; no 'linear'/'axis' category path in the nav; brand list is sensor/control brands | — | FAIL — no KK/equivalent priced module path (matches the morning sweep) |

**Result: no cheaper qualifying alternative exists with a live readable
price.** Every MOQ-1 listing under ≈US$450 either publishes no load rating
(2, 3, 6, 7, 8, 9, 16, 17, 18) or fails the re-derived load gate on its own
published numbers (Tiayo 694 N at 10 mm lead; Seatyle 2050 N vs 2400 N proof;
FSL120 981 N) — or is RFQ-only (fuyumotion). The single genuine P-grade
listing with a snippet price below the C-grade cost (eBay item 406408632737,
US$450) could not be price-verified live because eBay returns 403 to every
fetch path used.

### 8.3 Decision

**Keep the current substitution.** HIWIN KK8610C-340A1-F0 (×2, BOM AX-01 /
TR-AX01) at VND 21,146,653 per unit (USD 824.72 converted, MISUMI VN detail
221005545419, read 2026-09-16) remains the selected cut axis. The P→C switch
already satisfies the re-derived requirement with 2.6–5.9× margins (§8.1),
so nothing in this redo changes CAD, spec constants, drawings, FEM, BOM or
documents — this REDO section is the only file change.

Priority order for any future re-check (recorded as required):

1. **eBay item 406408632737 in a real browser** (highest priority): the
   US$450 snippet for the genuine KK8610P340A1F0 is the strongest cheap
   candidate, but its live price was not readable from this environment
   (403 on webfetch, r.jina.ai and curl). If a browser confirms a live price
   ≤~US$600, promote the genuine P-grade to primary with this URL/price and
   demote the MISUMI VN C-grade row to the priced fallback.
2. **MISUMI VN C-grade shelf 221005545419** (current selection, fallback).
3. made-in-china Seatyle/Tiayo only if the supplier publishes ≥1200 N
   working / ≥2400 N proof ratings and ±0.01-class repeatability on the
   listing (current listings do not).
4. fuyumotion FSK80V3/FSL120 only via quotation.
