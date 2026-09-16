# HIWIN KK8610P340A1F0 (article 10.00026) — live sourcing findings

Research date: 2026-09-16. Target identity: genuine HIWIN KK86-series precision
linear actuator, **KK8610P340A1F0**, HIWIN article **10.00026**, P accuracy
grade, 10 mm lead, 340 mm rail / 440 mm overall, single standard block, no
cover. Quantity of interest: 2.

Evidence standard applied: a price is valid only if it was read on a live page
that was actually fetched during this run. Prices visible only in search-engine
snippets, or on pages that returned 403/JS-shell/CAPTCHA, are recorded as
**non-valid evidence** and are not claimed as quotes.

## 1. Verdict

**ABSENT — no reachable channel publishes a concrete price for the genuine
HIWIN KK8610P340A1F0 / article 10.00026.**

- The manufacturer (hiwin.de, hiwin.fr, hiwin.tw) publishes the product record
  and CAD/data sheets but **no price element** logged out.
- All eight requested webshop channels and all distributor/e-commerce channels
  probed either return zero matches for the exact ordering code, show a
  placeholder/`N/A` price with no stock, or are unreachable from this
  environment (403/429/JS-shell).
- **Best candidate (does NOT meet the evidence standard):** eBay item
  [406408632737](https://www.ebay.com/itm/406408632737), titled
  "Hiwin KK8610P-340 Precision Linear Ball Screw Actuator, 10mm Lead, 215mm
  Stroke", **US $450.00** — the closest genuine-identity listing found (P
  grade, 340 rail; 215 mm physical stroke matches the older KK86 drawing that
  the repository already records for this part). The item page returned
  403 Forbidden on direct fetch, `curl` with a Chrome UA and via the r.jina.ai
  text proxy; the price exists only in a Brave Search snippet, so it cannot be
  admitted as a verified price. It remains the single best link to re-check
  from an unblocked browser.
- **Secondary candidate (genuine HIWIN, different accuracy grade):** MISUMI
  lists the C-grade sibling `KK8610C-340A1-F0` on
  [vn.misumi-ec.com detail 221005545419](https://vn.misumi-ec.com/vona2/detail/221005545419)
  (and `th.misumi-ec.com`). Every MISUMI fetch this run was 403/429
  rate-limited, so no price was read. This is the one shelf channel that in
  the repository's history has produced real HIWIN-family prices.
- Exact-identity page with no usable price:
  [unitech.com.ua](https://unitech.com.ua/ua/precizionnyy-aktuator-kk8610p340a1f0-hiwin/)
  — genuine "Прецизійний актуатор KK8610P340A1F0 HIWIN" (code UT001252484,
  brand country Taiwan) but the price is the placeholder **0 грн /шт з ПДВ**
  and all three stocking locations (Київ / Україна / Європа) read
  **"Немає в наявності"** (not in stock). Not a priced offer.

Channels attempted: **31 named channels / 55 URL fetches**, each recorded below.

## 2. Attempt table — every channel and URL tried

| # | Channel | URL(s) fetched live 2026-09-16 | Result | Evidence |
|---:|---|---|---|---|
| 1 | Google Shopping | `https://www.google.com/search?tbm=shop&q=KK8610P340A1F0` | BLOCKED | Error shell: "If you're having trouble accessing Google Search…" — no results served |
| 2 | Bing Shopping | `https://www.bing.com/shop?q=KK8610P340A1F0` | NO RESULTS | Query not honoured; page returned unrelated AET/education results |
| 3 | Bing web search | `https://www.bing.com/search?q=%22KK8610P340A1F0%22` | NO RESULTS | Unrelated results (Google Mail); quoted part not honoured |
| 4 | Brave Search (discovery) | `https://search.brave.com/search?q=KK8610P340A1F0` | WORKED | Returned hiwin.de record, unitech.com.ua, eBay items, MISUMI VN/TH, Proax, Accio; later queries 429 rate-limited |
| 5 | DuckDuckGo HTML (discovery) | `https://duckduckgo.com/html/?q=%22KK8610P340A1F0%22+price` | WORKED ONCE | Returned eBay 405999449274 (US) / 306882268428 (UK), hiwin.de, unitech.com.ua; subsequent queries CAPTCHA |
| 6 | DuckDuckGo Lite | `https://lite.duckduckgo.com/lite/?q=%22KK8610P340A1F0%22` | BLOCKED | CAPTCHA ("Select all squares containing a duck") |
| 7 | Mojeek | `https://www.mojeek.com/search?q=%22KK8610P340A1F0%22` | BLOCKED | CAPTCHA / JavaScript required |
| 8 | Ecosia | `https://www.ecosia.org/search?q=%22KK8610P340A1F0%22` | BLOCKED | HTTP 403 |
| 9 | Startpage | `https://www.startpage.com/sp/search?query=%22KK8610P340A1F0%22` | BLOCKED | Anubis JS challenge ("Verifying your request…") |
| 10 | HIWIN DE product record | `https://www.hiwin.de/en/Products/Precision-axes-%26-precision-systems/Precision-axes-KK-KF/KK/KK8610P340A1F0/p/10.00026` | LIVE, NO PRICE | Page lists article 10.00026, type KK8610P340A1F0, block count 1/2, cover options; zero `€`/price elements in fetched HTML |
| 11 | HIWIN FR product record | `https://www.hiwin.fr/fr/Produits/Axes-de-precision-et-systemes-de-precision/Axes-de-precision-KK-KF/KK/KK8610P340A1F0/p/10.00026` | LIVE, NO PRICE | Same shop system in French; no price |
| 12 | HIWIN IT | `https://www.hiwin.it/it/Prodotti/Assi-di-precisione-e-sistemi-di-precisione/Assi-di-precisione-KK-KF/KK/KK8610P340A1F0/p/10.00026` | 404 | Italian domain does not serve the product path |
| 13 | HIWIN ES | `https://www.hiwin.es/es/Productos/Ejes-de-precision-y-sistemas-de-precision/Ejes-de-precision-KK-KF/KK/KK8610P340A1F0/p/10.00026` | REDIRECT to shop home | No product page served |
| 14 | HIWIN TW | `https://www.hiwin.tw/products/single_axis_robot/kk.aspx` | LIVE CATALOGUE, NO PRICE | KK series spec page (0.02 mm accuracy, ±0.003 mm repeatability, 1,250 mm/s) |
| 15 | HIWIN online shop (legacy) | `hiwinonline.com` | PARKED PLACEHOLDER | Previously recorded 2026-09-13 in `catalogue-evidence.md` |
| 16 | Automation24 | `https://www.automation24.com/search?q=…`; `/search?sSearch=…`; `/linear-axes`; `https://www.automation24.de/search?sSearch=…`; homepage | 404 ×4; homepage LIVE | No HIWIN product page; site: query in Brave shows no KK8610/HIWIN axis slug; no price path for this part |
| 17 | Conrad | `https://www.conrad.com/en/search.html?search=KK8610P340A1F0` | 0 RESULTS | Conrad no-results shell ("Don't give up yet! … Use our Procurement Service") |
| 18 | Farnell UK | `https://uk.farnell.com/search?st=KK8610P340A1F0`; `https://uk.farnell.com/w/search?st=KK8610P340A1F0` | BLOCKED | HTTP 403 both; rate-limited early in the run |
| 19 | Octopart | `https://octopart.com/search?q=KK8610P340A1F0` | 0 RESULTS | "No exact matches found. View 0 Results." |
| 20 | Amazon.de | `https://www.amazon.de/s?k=KK8610P340A1F0` | 0 MATCHING PRODUCTS | Only sponsored unrelated items (descalers, sofa) |
| 21 | Simply Bearings (exact code) | `https://www.simplybearings.co.uk/search/suggest.json?q=KK8610P340A1F0&resources[type]=product` | 0 RESULTS | `{"resources":{"results":{"products":[]}}}` |
| 22 | Simply Bearings (family) | same API, `q=HIWIN KK` | 0 HIWIN PRODUCTS | Only an NKE imperial bearing at 0.00 (status POA) returned |
| 23 | Made-in-China (exact code, live) | `https://www.made-in-china.com/products-search/hot-china-products/KK8610P340A1F0.html` | 0 RESULTS | "Sorry! No matches were found for \"KK8610P340A1F0\"" |
| 24 | Made-in-China (clone record, prior) | HZMotion `TKK8610C-340A1F0` page | CLONE ONLY | US$800–2,500 FOB non-committal range (repository record 2026-09-16) |
| 25 | MISUMI (UK/VN/TH/DE/IT/US) | `uk.misumi-ec.com/vona2/result/?Keyword=KK8610P340A1F0`; `uk…/detail/221005545419/?HissuCode=KK8610P340A1F0`; same detail on vn/th/de/it/us | ALL BLOCKED | HTTP 403 ×3 (UK/US) and 429 ×4 (UK/VN/DE/IT); Brave snippet shows genuine HIWIN `KK8610C-340A1-F0` listed on `vn.misumi-ec.com/vona2/detail/221005545419` — price never read |
| 26 | Proax (authorized HIWIN distributor, CA) | `https://proax.ca/en/product/2131016/hiwkk8610c340a2fecs0` | LIVE, PRICE N/A | Page text "Price N/A", "Not in stock"; related item `KK8610P-440A1-F0` confirms P = accuracy grade. Product search `proax.ca/en/search?q=KK8610P` → 0 products |
| 27 | Rodavigo (ES) | `https://www.rodavigo.net/en/p/guide-rail-ref-hiwin-kk86-340a1-f0/110KK86340A1F0` | BLOCKED | HTTP 403 via webfetch and via `curl` Chrome UA; Brave snippet lists "HIWIN KK86-340A1-F0, ref 110KK86340A1F0" |
| 28 | Unitech UA | `https://unitech.com.ua/ua/precizionnyy-aktuator-kk8610p340a1f0-hiwin/` | LIVE, EXACT IDENTITY, NO PRICE | "0 грн /шт з ПДВ" placeholder; Київ/Україна/Європа all "Немає в наявності"; code UT001252484 |
| 29 | EU Automation | `https://www.euautomation.com/uk/`; `/search?q=…`; `/uk/search?searchTerm=…`; `/uk/sitemap` | LIVE, NO HIWIN | Manufacturer list contains no HIWIN; search 404/redirects to home; `curl` 403 |
| 30 | Motion Control Products | `motioncontrolproducts.com/search.php?search_query=…`; homepage | 404; HTTP 500 | Site unusable for search |
| 31 | Motion Control Products UK | `motioncontrolproducts.co.uk/search?q=KK8610`; `/catalogsearch/result/?q=KK8610` | 404; 410 | No result |
| 32 | Sodim | `https://www.sodim.com/recherche?q=KK8610`; `https://www.sodim.com/` | 404; redirect to Körber | sodim.com is Körber tobacco machinery, not a bearing distributor |
| 33 | Rexel | `https://www.rexel.com/en/search?q=KK8610P340A1F0` | 404 (corporate site) | Rexel webshops are country-specific; no product search served |
| 34 | Distrelec | `https://www.distrelec.com/en/search?q=…`; `https://www.distrelec.de/en/search?q=…` | Redirect | /en → Distrelec Group country page; /de/en → RS Germany homepage (RS blocked); no results |
| 35 | hbzhan.com | `https://www.hbzhan.com/search?keyword=KK8610` | BLOCKED | HTTP 403 |
| 36 | 1688.com | `https://s.1688.com/selloffer/offer_search.htm?keywords=KK8610P340A1F0` | JS SHELL | Only "批发_供应_阿里巴巴" served |
| 37 | Global Sources | `/search?q=KK8610`; `/search/KK8610.htm`; `/search/KK8610` | 404 ×3 | No reachable search endpoint |
| 38 | hisupplier.com | `/product-search/KK8610/`; `/search/?q=KK8610` | 404 ×2 | No reachable search endpoint |
| 39 | RobotDigg | `https://www.robotdigg.com/search?type=product&q=KK8610` | LIVE, NO MATCH | Page rendered generic catalogue (extruders, biofilter media, etc.); no KK8610 items |
| 40 | CNC4You | `https://www.cnc4you.co.uk/index.php?route=product/search&search=KK8610` | LIVE, NO MATCH | "There is no product that matches the search criteria." (site sells HIWIN carriages, not KK axes) |
| 41 | DamenCNC | `/search?q=KK8610`; `/en/search?q=KK8610` | 404 ×2 | No reachable search endpoint |
| 42 | Chronos | `https://www.chronos.ltd.uk/search?q=KK8610` | CLOSED | Banner: "THIS WEBSITE IS NOW CLOSED" |
| 43 | Mizen Automation | `https://mizenautomation.com/parts/hiwin-microsystems/KK8610C-940A1-F0` | BLOCKED | HTTP 403; Brave snippet: "Enquire Today" |
| 44 | Sorotec (DE CNC) | `https://www.sorotec.de/shop/advanced_search_result.php?keywords=KK8610` | LIVE, 0 MATCHES | Page loaded; 0 occurrences of "KK8610" in served HTML |
| 45 | Dold Mechatronik (DE) | `https://www.dold-mechatronik.de/search?q=KK8610` | LIVE, 0 MATCHES | Shop shell loaded; search term not applied; 0 occurrences of "KK8610" |
| 46 | Bearing King | `https://www.bearing-king.co.uk/search/suggest.json?q=KK8610…` | 0 RESULTS | `{"products":[]}` |
| 47 | Quality Bearings Online | `https://www.qualitybearingsonline.com/search.php?search_query=KK8610` | HTTP 200, 0 RESULTS | "0 results" |
| 48 | Mercateo | `https://www.mercateo.com/search?q=KK8610P340A1F0` | 404 | Path guess only; not pursued further |
| 49 | DirectIndustry | `https://www.directindustry.com/search?q=KK8610` | 404 | Not pursued further |
| 50 | ncbtmall | `https://ncbtmall.com/goods.php?id=18407` | TRANSPORT ERROR | Unreachable; Brave snippet lists HIWIN KK8610P-540A1-F0 page with no price |
| 51 | eBay (all regions) | items `405999449274`, `306882268428`, `406408632737`, `335963711129` | BLOCKED | HTTP 403 via webfetch, `curl` + Chrome UA, and `r.jina.ai` text proxy. Prices exist only as Brave/DDG snippets (below) |
| 52 | bestsuppliers.com | `https://www.bestsuppliers.com/industrial-linear-guideway-rail-kk-module-hiwin-kk8610c-940a1-f0-for-robot` | 404 | URL guess; Brave snippet lists same title at $500 (KK8610C-940A1-F0, not the selected 340/P part) |
| 53 | Accio (aggregator) | `https://accio.ai/find-product/kk86-hiwin` | LIVE, CLONE/BROKER RANGES | "Hiwin Kk86 Series Kk8610 Linear Module" $486.91–496.91 (Asia FON, MOQ 1 set); "Original Hiwin KK86 Series Aluminum Linear Module" $164.85–319.65 (Suzhou Junjie Hongsheng) — none proven genuine P-grade 340 |
| 54 | Alibaba (prior, same day) | repository record 2026-09-16 | BLOCKED | Empty shell (recorded in `catalogue-evidence.md`) |
| 55 | eBay global — search-grade | `https://www.ebay.com/itm/406408632737` (Brave snippet) | SNIPPET ONLY | "Hiwin KK8610P-340 Precision Linear Ball Screw Actuator, 10mm Lead, 215mm Stroke — Price: $450.00" (closest identity; page 403) |

## 3. Genuine-HIWIN listings found (not the selected P/340/1F0 combination) — market context

These are genuine HIWIN ordering identities but **different configurations**
(accuracy grade, cover, length, flange); they are not the selected part and
were not priced from a live page in this run.

| Title as published | URL | Observed price | Status |
|---|---|---|---|
| "KK8610C-340A1-F0 | Single Axis Robot KK86 Type (Mounting Surface Height: 46, Width: 86) | HIWIN" (MISUMI VN/TH) | `https://vn.misumi-ec.com/vona2/detail/221005545419` | not read (403/429) | Genuine HIWIN, C accuracy grade, 340 rail |
| "KK8610C-340A2-FECS0 … Double Block, Custom Flange, Cover Kit, and Sensor Rail (Proax)" | `https://proax.ca/en/product/2131016/hiwkk8610c340a2fecs0` | "Price N/A" | Live page read; not in stock |
| "KK8610P-440A1-F0 Single Axis Stage, Type KK86, Accuracy Grade P, Lead 10mm, Rail Length 440mm" (Proax related item) | `https://proax.ca/en/product/2131016/hiwkk8610c340a2fecs0` (related list) | not shown | Confirms the P letter = accuracy grade |
| HIWIN KK86-340A1-F0, ref 110KK86340A1F0 (Rodavigo ES) | `https://www.rodavigo.net/en/p/guide-rail-ref-hiwin-kk86-340a1-f0/110KK86340A1F0` | not read (403) | Snippet only |

## 4. Clone / broker listings — explicitly NOT the selected part

Recorded only as market context, with exact published titles.

| Published title | URL | Published price | Note |
|---|---|---|---|
| "China Hzmotion Original Factory Tkk8610c-340a1f0 Tkk Low Profile Aluminum Base Kk Type Linear Actuator…" | made-in-china.com (HZMotion store) | US$800–2,500 FOB (range) | Clone `TKK8610C-340A1F0` (repository record 2026-09-16) |
| "Hiwin Kk86 Series Kk8610 Linear Module" (Asia FON Tianjin) | `https://accio.ai/find-product/kk86-hiwin` | $486.91–496.91 / set | Broker listing; no HIWIN authenticity/grade evidence |
| "Original Hiwin KK86 Series Aluminum Linear Module for Packaging Machines…" (Suzhou Junjie Hongsheng) | `https://accio.ai/find-product/kk86-hiwin` | $164.85–319.65 / piece | Broker range across unspecified configs |
| "Industrial linear guideway rail KK Module hiwin KK8610C-940A1-F0 for robot" | bestsuppliers.com (URL not resolved) | $500.00 (snippet) | 940 mm C-grade; not selected |
| "1pc New KK8610C-940A1-F0 module" / "1Pc New HIWIN KK8610C-940A1-F0 module" | eBay items (snippet-sourced) | US $1,644.96 (was $1,788) / $1,606.00 | C-grade 940 mm; pages 403 |
| "NEW Hiwin Linear Actuator - KK8620P-640A1-F0CS0 w/Warranty" | eBay (snippet-sourced) | $606.00 | Different size/length |
| "HIWIN KK50 KK60 KK86 Replacement Linear Module,High Rigidity Single Axis Robot" | `https://www.ebay.com/itm/335963711129` | $244.83 | "Replacement" wording; selectable models; not a genuine-identity offer |
| Canmeng KK86D20C/D10C 940 mm clones | made-in-china.com | US$50–200 | Clone (prior repository record) |

## 5. Notes on search coverage

- Brave Search was the only discovery engine that consistently returned
  product-level hits and is how the unitech.com.ua, eBay, MISUMI VN/TH, Proax,
  Accio and Rodavigo leads were found. It then rate-limited (429) mid-run.
- DuckDuckGo `html.duckduckgo.com` worked exactly once, then served CAPTCHAs to
  every endpoint variant tried; Mojeek, Ecosia and Startpage all blocked
  automated retrieval.
- No public catalogue page observed in this run publishes a price for the
  genuine P-grade HIWIN KK8610P340A1F0. The two observed exact-identity
  surfaces are HIWIN's own unpriced product record and Unitech UA's
  zero-placeholder/out-of-stock page. The closest genuine-identity priced
  listing found anywhere is the blocked eBay item 406408632737 at US $450.00
  (snippet only, not admissible as a verified price).
