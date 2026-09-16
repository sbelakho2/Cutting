# Sourcing findings — CL57T driver, LE-350-36 PSU, DIN 471 A12 rings, M5×16 10.9 screws, 3m6×20 dowels

Research date: **2026-09-16**. All prices below were read from live-fetched pages on that date (URL + price + availability recorded). Blocked/empty results are recorded as attempted-blocked with the URL. StepperOnline's own shops (`omc-stepperonline.com`, `stepperonline.com`) and its AliExpress storefront were **not usable** (403 / JS-only), so official list prices could not be captured.

Legend: **FOUND** = live page with concrete price; **PARTIAL** = item obtainable with price but not the exact requested model string/brand; **ABSENT** = exhaustive attempts, no live price page.

---

## 1. Verdicts

### (a) STEPPERONLINE CL57T-V41 closed-loop stepper driver (×2) — **PARTIAL (class found, exact “-V41” suffix absent)**

- No reachable live page anywhere shows the exact string **CL57T-V41**. The closest index entry is Octopart’s `CL57T_V4.0` (manufacturer OMC = StepperOnline), which shows **no stock and no price**: <https://octopart.com/part/omc/CL57T_V4.0>
- Cheapest verified live marketplace listings (amazon.de, fetched 2026-09-16), all titled “CL57T Closed Loop Stepper Driver 2 Phase 0-8.0A 24-48VDC for Nema 17 Nema 23 Nema 24 Stepper Motor”:
  - **€173.78 — <https://www.amazon.de/dp/B0FYPDGC85>** — “Usually dispatched within 2 to 3 days”; delivery €5.53 (30 Sep–17 Oct); seller brand CMGDWDKYZ. **Best candidate.**
  - €175.45 — <https://www.amazon.de/dp/B0FXRM4FV8> — same availability/dispatch terms.
  - €189.10 — <https://www.amazon.de/dp/B0HJ23YQVQ> (German title “CL57T Schrittmotor Treiber”) — same availability.
- Note: none of the amazon.de listings is sold under the “STEPPERONLINE” brand (marketplace resellers); a search for “StepperOnline CL57T” returns no CL57T at all. StepperOnline-branded listing with price: **ABSENT**.
- Made-in-China wholesale anchor attempt 2026-09-16: the multi-search page returns 300 fuzzy matches but no CL57T driver product on page 1 — no concrete made-in-china offer recorded; the amazon.de class anchors above remain the only live CL57T prices (see coupling-ring-replacement-findings.md §3).

### (b) STEPPERONLINE LE-350-36, 36 V / 350 W PSU (×1) — **ABSENT (exact), nearest verified equivalents priced**

- No live page sells the exact string “LE-350-36”. StepperOnline shops 403; amazon.de exact-string search only returns fuzzy matches (ERP-350-36 / LRS-350-36). The model itself is documented in this repo (`engineering/sources/LE-350-36.pdf`), but that is not a price source.
- Verified live equivalents (36 V, 350 W, amazon.de, fetched 2026-09-16):
  - **€77.04 — <https://www.amazon.de/dp/B0HDN818SJ>** — “ERP-350-36 (36V)”, “Usually dispatched within 2 to 3 days”; free delivery 24–25 Sep; brand GWRQXWCKA. **Best candidate (cheapest live, matching specs).**
  - €82.47 — <https://www.amazon.de/dp/B0HJ1B94ST> — “LRS-350 350W Industrielles Schaltnetzteil” (36 V 9.7 A class, MEAN WELL LRS-350-36 type); dispatched 2–3 days; brand XBNYIGHNA.
  - Wholesale reference (made-in-china.com, fetched 2026-09-16): “AC-DC 36V 9.7A 350W S-350-36”, **US$12.70–14.90, MOQ 1** — <https://junlin-power-supply.en.made-in-china.com/product/LfqRezWPJJcM/China-AC-DC-36V-LED-SMPS-9-7A-350W-Switch-Power-Supply-LED-Driver-S-350-36.html> (unverified manufacturer, export terms).

### (c) SEEGER-ORBIS A12 / DIN 471 retaining ring for 12 mm shaft (×8) — **ABSENT (brand-exact), verified DIN 471 12×1 equivalents priced**

- No working channel stocks branded **Seeger-Orbis A12**: amazon.de brand search “Seeger-Orbis” returns only the Seeger handbook (ASIN B004QO5FOC); Conrad has no DIN 471 for 12 mm shafts; Farnell is 403. Brand+part-number evidence: **ABSENT**.
- Verified live equivalents (DIN 471, 12 mm shaft, 1.0 mm thick → “12 x 1”; fetched 2026-09-16):
  - **Wegertseder Art. 1950-010 “12 x 1” (Federstahl, Regelausführung), 100 St. €8.57 / 1.000 St. €39.98 / 5.000 St. €183.86** — in stock (green availability marker); prices incl. 19 % VAT, zzgl. Versand — <https://www.wegertseder.com/DIN-471-Sicherungsringe-fuer-Wellen-Regelausfuehrung-FEDERSTAHL-Diameter-12> — **best candidate for ×8 (order the 100-pc Kleinpaket)**. Page states DIN 471 is for shaft grooves; page PDF confirms d3 = 11.0 mm, s = 1.0 mm for Ø12.
  - Amazon.de **B00IZC9GVO** “Circlip 12x1-FSt (DIN 471)”, Brand **MZA**, no featured offer; search page shows **€0.62 with 3 new offers** (2026-09-16) — <https://www.amazon.de/dp/B00IZC9GVO>.
  - Simply Bearings (UK, price per piece, avail True): **d1400-012** (standard external circlip, 12 mm shaft, un-sprung Ø11 mm) **£0.85** — <https://www.simplybearings.co.uk/products/d1400-012>; heavy-duty **d1460-012 £0.85** — <https://www.simplybearings.co.uk/products/d1460-012>. Unbranded.
  - schraubenhandel24 DIN 471 Federstahl family (Art. 471-S, base €3.40 with size selection) — <https://www.schraubenhandel24.de/scheiben/sicherungsringe/din-471/din-471-sicherungsringe-federstahl/>.

### (d) ISO 4762 M5×16 class 10.9 socket screws (small box) + ISO 8734 3m6×20 dowel pins — **FOUND (both, Wegertseder, in stock)**

- **M5×16, 10.9 (DIN 912 ≈ ISO 4762): Wegertseder Art. 2540-034 “M 5 x 16” (STAHL 10.9)** — Kleinpaket **50 St. €6.19**, 500 St. €26.66, 2.000 St. €99.48; in stock — <https://www.wegertseder.com/DIN-912-Zylinderschrauben-mit-Innensechskant-STAHL-10-9-Diameter-5> (DIN 912 page states “ähnlich ISO 4762: keine gravierenden Änderungen”).
  - Alternatives: schraubenhandel24 `912-V0-M5` “Zylinderschrauben ISO 4762 / DIN 912 – M5 – verzinkt 10.9”, base **€7.08** (size via variant table) — <https://www.schraubenhandel24.de/schrauben/gewindeschrauben/zylinderschrauben/din-912/>. Amazon.de: no 10.9 M5×16 listing identified; only 8.8 zinc (e.g. “100 x 1000381 … DIN 912 … Steel” €14.59) and A2 stainless variants.
- **3m6×20 (m6 tolerance, hardened) ≈ ISO 8734: Wegertseder Art. 1018-078 “3 x 20” (DIN 6325, hardened, tolerance m6)** — Kleinpaket **10 St. €2.73**, 100 St. €9.52, 500 St. €37.49; in stock — <https://www.wegertseder.com/DIN-6325-Zylinderstifte-durchgehaertet-Passung-m6> (page states “ähnlich ISO 8734”).
  - Alternative: schraubenhandel24 DIN 6325 hardened m6 family `6325-S-3` (Ø3 mm, lengths selectable), base **€4.20** — <https://www.schraubenhandel24.de/naegel-stifte/zylinderstifte/din-6325/din-6325-zylinderstifte-stahl-d-3/>. Amazon.de: no DIN 6325/ISO 8734 listing found; only stainless “positioning pin” products (not hardened m6).

---

## 2. Attempt log (channel, URL, result, evidence)

### Amazon.de (EUR incl. VAT; item pages machine-fetched 2026-09-16)
| URL | Result | Evidence |
|---|---|---|
| /s?k=CL57T-V41 | no results | “No results for your search query.” |
| /s?k=CL57T closed loop driver | 7 results | CL57T drivers €173.78 / €175.45; CL57T motor kits €253.87–€303.36 |
| /dp/B0FYPDGC85 | **€173.78** | title “CL57T Closed Loop Stepper Driver 2 Phase 0-8.0A…”; “Usually dispatched within 2 to 3 days”; brand CMGDWDKYZ |
| /dp/B0FXRM4FV8 | €175.45 | same title; dispatched 2–3 days; brand YIRONCNPA |
| /dp/B0HJ23YQVQ | €189.10 | German title CL57T; dispatched 2–3 days; brand MUYTQAERG |
| /s?k=StepperOnline CL57T | 2 results, no CL57T | Nema14 motor “Currently unavailable”; servo €201.67 |
| /s?k=STEPPERONLINE Netzteil 36V | irrelevant | 30 W converter €37.66; AR4 package €2,824.08 |
| /s?k=LE-350-36 | 116 fuzzy results | top ERP-350-36 €77.04 / €77.38; LRS-350-36 €82.47+; UHP/UHP-350 €108–113 |
| /dp/B0HDN818SJ | **€77.04** | “ERP-350 ERP-350-12…36…48 Schaltleistung(Erp-350-36v)”; free delivery 24–25 Sep; dispatched 2–3 days |
| /dp/B0HJ1B94ST | €82.47 | “LRS-350 350W Industrielles Schaltnetzteil”; dispatched 2–3 days; brand XBNYIGHNA |
| /s?k=Seegerring DIN 471 12 mm | 7 results | B00IZC9GVO “Circlip 12x1-FSt (DIN 471)” **€0.62 (3 new offers)**; 45x1,75 €6.75; DIN472 12 mm €5.99/10 pc |
| /s?k=DIN 471 Sicherungsring 12 mm | B00IZC9GVO top | “Circlip 12x1-FSt (DIN 471)”; also 22x1.2 €1.01 |
| /dp/B00IZC9GVO | brand MZA, no featured price | title Circlip 12x1-FSt (DIN 471); Brand: MZA; no part-number field exposed |
| /gp/offer-listing/B00IZC9GVO | no server-rendered price | offer page JS-only for prices |
| /s?k=Seeger-Orbis | book only | “Seeger-Ringe, Truarc-Ringe…” handbook (B004QO5FOC); no Seeger-Orbis rings |
| /s?k=Seeger-Orbis A12 | none | no product hits |
| /s?k=Zylinderschraube DIN 912 M5x16 10.9 | 597 results, no 10.9 M5×16 | A2 stainless / 8.8 zinc only in parsed titles |
| /s?k=Zylinderschrauben M5x16 10.9 | no 10.9 M5 | “Cylindrical Hex Screws DIN912 M5 x 16 Galvanised Pack of 50” €12.74 (8.8) |
| /s?k=DIN 912 M5x16 Stahl 10.9 | 795 results, no 10.9 M5×16 | 8.8 zinc 100-packs €13.03 / €13.87 / **€14.59 (Boss)** |
| /s?k=Innensechskantschrauben M5x16 10.9 | no relevant hit | — |
| /s?k=DIN 912 M5x16 10.9 verzinkt | no relevant hit | — |
| /s?k=Zylinderstift DIN 6325 3m6x20 | no DIN 6325 | stainless positioning pins €21.09–€24.23 (not ISO 8734/m6) |

### StepperOnline / mirrors
| URL | Result | Evidence |
|---|---|---|
| https://www.omc-stepperonline.com/ | **blocked** | HTTP 403 |
| https://www.stepperonline.com/ | **blocked** | HTTP 403 |
| https://www.aliexpress.com/store/1955025 | JS-only shell | “Your browser does not support JavaScript!”; no catalog/price data |
| https://octopart.com/search?q=CL57T-V41 | 1 similar part | CL57T_V4.0 (OMC) — “No stock available. View Non-Authorized Dealers.” |
| https://www.made-in-china.com/products-search/hot-china-products/CL57T.html | no matches | “Sorry! No matches were found for ‘CL57T’” |
| https://www.made-in-china.com/multi-search/CL57T/F1/1.html | no CL57T driver product on page 1 | “CL57T” 300 fuzzy results; page 1 shows unrelated products (CNC lathe US$50,000, lifting clamps, AC motor) — no CL57T closed-loop driver listing with a price (fetched 2026-09-16) |
| https://www.made-in-china.com/products-search/hot-china-products/LE-350-36.html | no exact match | closest: “S-350-36 … 350W” US$12.70–14.90, MOQ 1 (Junlin) |

### automation24.com
| URL | Result | Evidence |
|---|---|---|
| /search?query=CL57T | 404 | no search endpoint at this path |
| /search?q=Seegerring | 404 | — |
| /en/search?query=Seegerring | 404 | — |
| /search?query=, /search/CL57T (curl, Chrome UA) | 403 | bot-blocked before path handling |

### Conrad.com (EUR incl. VAT; fetched 2026-09-16)
| URL | Result | Evidence |
|---|---|---|
| /en/search.html?search=closed loop stepper driver | 1 result | only Joy-it Nema23-04cl stepper motor €174.99; no closed-loop driver |
| /en/search.html?search=retaining ring 12 mm | 31 results | TOOLCRAFT DIN 471/472 200-pc boxes; no DIN 471 for 12 mm shaft (smallest listed DIN 471 inside-Ø 12.9 mm = larger shaft) |
| /en/search.html?search=DIN 471 12x1 | 5 results | relays/bulbs/ribbon cable — no rings |
| /en/search.html?search=DIN 912 M5x16 | 4 results | SWG 9451667 M5×16 DIN 912 **A2 stainless** 50 pc €13.99 (not 10.9) |

### Farnell (rate-limited per brief; 1–3 hits used)
| URL | Result | Evidence |
|---|---|---|
| https://uk.farnell.com/search?st=…retaining ring… | **403** | blocked |
| https://uk.farnell.com/w/search?st=din 471 12mm | **403** | blocked |
| https://de.farnell.com/search?st=din 471 sicherungsring 12mm | **403** | blocked |

### Simply Bearings (GBP; Shopify suggest JSON via curl, fetched 2026-09-16)
| Query / URL | Result | Evidence |
|---|---|---|
| suggest “DIN 471 12” | external circlip for 12 mm shaft | d1460-012 (heavy duty) £0.85 avail true |
| suggest “circlip 12” | tools/other sizes | no 12 mm standard circlip in top 8 except 25 mm etc. |
| /products/d1400-012 (suggest) | **£0.85, avail True** | “External Circlip for 12mm Shaft Diameter Un-Sprung Diameter 11mm” |
| /products/d1460-012 (suggest) | **£0.85, avail True** | “Heavy Duty External Circlip for 12mm Shaft Diameter…” |
| suggest “dowel 3m6” | none | only rod ends (Dunlop M6) |
| suggest “dowel pin 3mm”, “DIN 6325” | none | only dowel 6x6 mm / spring roll pins |
| suggest “socket screw M5”, “cap screw M5 16” | grub/set screws | M5x16mm “Set Screw” £0.85 (not ISO 4762) |
| suggest “ISO 4762” | none | — |

### schraubenhandel24.de (EUR incl. VAT; fetched 2026-09-16)
| URL | Result | Evidence |
|---|---|---|
| /search?sSearch=DIN 912 M5x16 (×3 variants) | 200 but no parseable results | custom shop (base64 prg links); unusable via plain parse |
| ?suche=M5x16 | 24 items | `912-V0-M5` “ISO 4762 / DIN 912 – M5 – verzinkt **10.9**” €7.08; `912-V8-M5` (8.8) €3.42; A2/A4 variants |
| ?suche=Zylinderstift 3m6x20 | 24 items | `6325-S-3` “DIN 6325 … gehärtet m6 … Durchmesser 3 mm” €4.20 (family) |
| ?suche=DIN 471 12 | 24 items | family links (471-S etc.); ring families listed |
| /naegel-stifte/zylinderstifte/din-6325/din-6325-zylinderstifte-stahl-d-3/ | base article | 6325-S-3 EUR 4.20 (lengths via variants) |
| /scheiben/sicherungsringe/din-471/din-471-sicherungsringe-federstahl/ | base article | 471-S “DIN 471 Sicherungsringe – Für Wellen – Federstahl” EUR 3.40 (size via variants) |
| /schrauben/gewindeschrauben/ + /zylinderschrauben/din-912/ | category pages | DIN 912 lines: 8.8/10.9/A2/A4; 10.9 verzinkt family present |
| /naegel-stifte/zylinderstifte/din-6325/ | category items | 3 m6 x 40 (500 St) €31.85; 3 m6 x 45 €35.13 |

### Wegertseder.com (EUR incl. 19 % VAT; fetched 2026-09-16) — all rows show green in-stock marker
| URL | Result | Evidence |
|---|---|---|
| /search?sSearch=M5x16 and /en/search?q=M5x16 | error page | title “Fehlerdetails” |
| / (homepage), /sitemap.xml | OK | product URL scheme discovered (DIN/ISO catalogue pages) |
| /DIN-912-Zylinderschrauben-mit-Innensechskant-STAHL-10-9-Diameter-5 | **2540-034 “M 5 x 16”** | 50 St. **€6.19** / 500 St. €26.66 / 2.000 St. €99.48 |
| /DIN-6325-Zylinderstifte-durchgehaertet-Passung-m6 | **1018-078 “3 x 20”** | 10 St. **€2.73** / 100 St. €9.52 / 500 St. €37.49; “ähnlich ISO 8734” |
| /DIN-471-Sicherungsringe-fuer-Wellen-Regelausfuehrung-FEDERSTAHL-Diameter-12 | **1950-010 “12 x 1”** | 100 St. **€8.57** / 1.000 St. €39.98 / 5.000 St. €183.86 |
| /DIN-471-… (family page) | pivot navigation | FEDERSTAHL / verzinkt / 1.4122 / A2 material trees; Ø12 available in Federstahl, verzinkt, 1.4122, A2 |

### Würth / Bossard
| URL | Result | Evidence |
|---|---|---|
| https://www.wuerth.de/ | login-gated | “Verkauf nur an Gewerbetreibende”; prices only in eshop after registration — no public price |
| https://www.bossard.com/global-en/shop/ | 404 | shop URL not reachable (JS app per brief) |

---

## 3. Caveats and recommendations

- **Exact-model gaps are caused by channel blocks, not by absence of demand:** StepperOnline (403) and AliExpress (JS) are the only places where the bare `CL57T-V41` and `LE-350-36` strings are certain to exist; both were unusable. Marketplace equivalents are 36 V/350 W class-correct but not the same vendor.
- Amazon marketplace CL57T sellers are non-OEM resellers (CMGDWDKYZ, YIRONCNPA, MUYTQAERG). Verify revision and authenticity on arrival; the OEM part is `CL57T_V4.1`.
- Wegertseder prices include 19 % VAT, exclude shipping; site states DE/AT free freight from €125 and a €25 minimum order value. Kleinpaket sizes: screws 50 pcs (€6.19), dowels 10 pcs (€2.73), rings 100 pcs (€8.57) — all suitable for a small build quantity.
- For the ×8 retaining-ring requirement, the Wegertseder 100-pc Kleinpaket is the cheapest compliant live DIN 471 12×1 option with confirmed stock; the MZA amazon.de listing (€0.62 + offers) is the only amazon alternative but has no featured offer and no confirmed part number.
- `engineering/sources/seeger-catalogue-2022.pdf`, `CL57T-V41.pdf`, `LE-350-36.pdf` remain the technical references for the exact OEM parts; this file only adds live commercial sourcing evidence.
