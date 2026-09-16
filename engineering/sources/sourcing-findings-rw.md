# Sourcing findings — R+W MK2/100/50/8/14 and MK2/100/50/10/12 miniature bellows couplings

**Date:** 2026-09-16 (all pages fetched live on this date) · **Scope:** qty 2 each · **Evidence rule:** price valid only if read from a live fetched page (URL + price + currency + stock).

## 1) Verdict

**MK2/100/50/8/14 — ABSENT. No live page with a price found.** Live catalogue pages for the
series exist (misumi MX, archimedes.pl, zofa-industry), but no page for this bore combination
carries a readable price; ebmia.pl search for the exact code returns zero results.

**MK2/100/50/10/12 — ABSENT. No live page with a price found.** ebmia.pl search `MK2/100/50`
returns exactly two catalogue items — `MK2/100/50/10/14` and `MK2/100/50/8/21,4` — so 10/12 is
not listed anywhere retrievable.

**Closest genuine R+W MK2 100 50 items WITH public live prices (different bores; candidates / quote anchors):**

1. **MK2/100/50/10/14 — ebmia.pl (PL), per piece: 122,30 € gross / 99,43 € net (530,35 zł gross / 431,18 zł net)**
   - URL (EUR view): https://www.ebmia.pl/sprzegla-mieszkowe-bkl/213877-sprzeglo-bezluzowe-mk2-100-50-10-14.html?SubmitCurrency=1&id_currency=2
   - URL (PLN default view): https://www.ebmia.pl/sprzegla-mieszkowe-bkl/213877-sprzeglo-bezluzowe-mk2-100-50-10-14.html
   - Stock: **0 "W magazynie"** · Dispatch: **7–14 days** · Producer: R+W · Weight 0.13 kg
   - Dims from page: B (OD) 40 mm, A (length) 50 mm, C 16 mm, G 5 mm, D1 10, D2 14, bore min 5 / max 24.
2. **MK2/100/50/8/21,4 — ebmia.pl, per piece: 418,85 zł gross / 340,53 zł net**
   - Product URL: https://www.ebmia.pl/sprzegla-mieszkowe-bkl/127772-sprzeglo-bezluzowe-mk2-100-50-8-214.html
   - Price read on live listing: https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50 (shows both items)
   - Availability: **24 h** · Dims: B 40, A 50, D1 8, D2 21.4
3. **MK2/100/50/10/14 — Archimedes sp. z o.o. B2B (PL), price not public**
   - URL: https://odoo.archimedes.pl/en/shop/e85-230-981-mk2-100-50-10-14-backlash-free-bellows-coupling-r-w-mk2-100-50-10-14-235685
   - Live page: "Ask for price" / "Log in to see the price", SKU E85-230-981; main-warehouse items ship same business day.

**Manufacturer publishes no prices.** rw-couplings.com MK2 page shows *"auf Anfrage"* (on request);
WooCommerce Store API is 404 (disabled) and no per-bore product URLs exist (sitemap contains only
base model pages). No R+W web shop found.

**Geometry flag (affects the selection, not just sourcing):** the R+W model is designated
**MK2 100 50** — catalogue data (R+W page JSON and ebmia drawing data) gives **length 50 mm,
outer Ø 40 mm**, bore range **5–24 mm**. The "100" is the size/torque class, **not a 100 mm OD**.
The brief's "100 mm OD" should be re-verified against the intended coupling before ordering.

**Buy path for the exact bores:** they are orderable as made-to-bore items but only via quote
(R+W direct, ebmia.pl, archimedes.pl, or Misumi where the SKU is sold per bore pair); qty 2 at
list price is only possible for the stocked/sub-catalogue variants above (2 × 122,30 € = 244,60 €
gross for 10/14; 2 × 418,85 zł = 837,70 zł gross for 8/21,4).

## 2) Attempt table (all channels, live on 2026-09-16)

| # | Channel | URL / query | Result (evidence) |
|---|---|---|---|
| 1 | R+W manufacturer (MK2 page) | https://www.rw-couplings.com/product/mk2/ | HTTP 200 (curl, Chrome UA). Product page contains **"auf Anfrage"**, no price markup; configurator JSON lists series MK2 5/10/15/20/45/100 and models incl. "MK2 100 50" / "MK2 100 60" with attrs length=50, bore-range 5–24, B/OD=40; **no price keys** in page data |
| 2 | R+W shop | https://www.rw-couplings.com/shop/ | HTTP 404 — no web shop |
| 3 | R+W site search | https://www.rw-couplings.com/?s=MK2 | 200; only `/product/mk2/` — no per-bore pages |
| 4 | R+W WP REST | /wp-json/wp/v2/product?search=MK2 ; /wp-json/wc/store/v1/products?search=MK2 | HTTP 404 both (wc Store API disabled, product type not REST-exposed) |
| 5 | R+W sitemaps | /sitemap_index.xml , /rw_product-sitemap.xml | 200; product sitemap lists base models only (mk2, bkl, bk1…) — no per-bore URLs |
| 6 | R+W America | https://www.rw-america.com/product-overview-precision/product-overview-precision-miniature-bellows-couplings/ | Live; catalogue description only, no prices |
| 7 | **ebmia.pl (PL)** | https://www.ebmia.pl/sprzegla-mieszkowe-bkl/213877-sprzeglo-bezluzowe-mk2-100-50-10-14.html (+EUR view) | **LIVE PRICE: 530,35 zł / 122,30 € gross, 431,18 zł / 99,43 € net per pc; stock 0; 7–14 days** |
| 8 | **ebmia.pl (PL)** | https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50 | Live listing: exactly 2 items — 10/14 (530,35 zł) and 8/21,4 (**418,85 zł, 24 h**) |
| 9 | ebmia.pl exact search | https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50%2F8%2F14 | "Brak wyników wyszukiwania" — **exact bore not in catalogue** |
| 10 | Archimedes B2B (PL) | https://odoo.archimedes.pl/en/shop/e85-230-981-mk2-100-50-10-14-backlash-free-bellows-coupling-r-w-mk2-100-50-10-14-235685 | Live page, "Ask for price" / "Log in to see the price" — no public price |
| 11 | MISUMI EU/UK | https://uk.misumi-ec.com/vona2/result/?Keyword=MK2%2F100 ; ...Keyword=R%2BW ; /vona2/detail/221302477749/?HissuCode=MK2/100/50/8/14 ; .../10/12 | **429 / 403 (blocked, 4 attempts webfetch + curl)** |
| 12 | MISUMI MX | https://mx.misumi-ec.com/en/vona2/detail/221302477749/?HissuCode=MK2/100/50/8/14 | **429 (blocked, webfetch + curl)** — search snippets prove Misumi catalogues R+W MK2/100/50/14/14, /14/17, /8/22, /13/14 (no 8/14 nor 10/12 seen) |
| 13 | automation24.com | /search?sSearch=MK2 ; /search?search=MK2 ; /search?sSearch=R%2BW+MK2 | HTTP 404 on all — search URL unusable; brand list has no R+W |
| 14 | automation24.de | /search?sSearch=MK2 ; /search?search=MK2 | HTTP 404 (curl homepage 403 bot wall) |
| 15 | Conrad | https://www.conrad.com/en/search?search=MK2%2F100 | 1.620 unrelated results (Omnitronic speaker, Fischer pin strips) |
| 16 | Conrad | https://www.conrad.com/en/search?search=R%2BW+Kupplung | 1 result, unrelated (LAPP EPIC cover) — no R+W |
| 17 | Amazon.de | https://www.amazon.de/s?k=R%2BW+MK2+bellow+coupling | "No results for your search query" |
| 18 | Amazon.de | https://www.amazon.de/s?k=R%2BW+Kupplung+MK2 | 1 unrelated result (car pedal rubber) |
| 19 | reichelt.de | https://www.reichelt.de/de/de/shop/search?search=MK2 | Live: 79 hits, all unrelated (ERSA i-CON MK2, Meder reed sensors…) |
| 20 | Farnell | https://uk.farnell.com/search?st=MK2%2F100 | HTTP 403 (bot wall / rate-limited) |
| 21 | Octopart | https://octopart.com/search?q=MK2%2F100%2F50 | Live: 3 "similar matches", all Fischer Elektronik pin headers — no R+W |
| 22 | Made-in-China | https://www.made-in-china.com/multi-search/bellows+coupling/F1/1.html | Live: 13,000+ **generic** bellows couplings, FOB **US$2.50–25** — market context only, not R+W |
| 23 | Motion Control Products | https://motioncontrolproducts.com/search?q=R%2BW+MK2 | Live: "0 results" (no couplings stocked) |
| 24 | Voelkner | https://www.voelkner.de/search.html?keywords=MK2%2F100 ; /search.html?q=MK2%2F100 ; /search/index.html?keywords=MK2 | 404 / 404 / 403 — search unusable |
| 25 | Pollin | https://www.pollin.de/search?query=MK2 | Live: 105 hits, all consumer (Denver cam MK2, Logitech MK295…) |
| 26 | Mercateo | https://www.mercateo.com/en/search.html?q=R%2BW+MK2 | HTTP 404 (URL/service retired) |
| 27 | zofa-industry (CN reseller) | http://www.zofa-industry.com/NewsDetail/2236787.html | Live R+W brand page; lists R+W part numbers (incl. MKH/100/50/8/8, MK2/20/44/…) — **no prices**, no MK2/100/50 exact |
| 28 | Brave Search | "MK2/100/50/8/14" (+ price operators); 2 later queries | Worked (discovery: ebmia, Misumi MX, Archimedes); later HTTP 429 rate limit |
| 29 | Bing | MK2/100/50/8/14 (quoted and plain) | Junk results (Bromcom/VLC); no match for the part |
| 30 | DuckDuckGo (html + lite) | "MK2/100/50/8/14" ; "MK2/100/50/10/12" | Bot captcha challenge |
| 31 | Mojeek / Ecosia / SearX | same exact codes | JS challenge / HTTP 403 / anti-bot challenge |

**Search strings used:** `"MK2/100/50/8/14"`, `"MK2/100/50/10/12"`, `"MK2/100/50" + R+W/MK2 + cena/price/Preis`, site:misumi-ec.com, plus channel-native queries (see table).

## 3) Market context (bore/length class, not the selection)

- Generic Chinese bellows couplings of comparable small size (D20 L32 etc.): FOB US$2.50–25 on Made-in-China (live 2026-09-16) — not R+W, not the 100-size class.
- ebmia.pl sibling series BKL/3 (much smaller): ~24–30 € net per pc — context for store pricing level only.

## 4) Recommendation

Treat the exact codes as **quote items**: request 2 pcs each of MK2/100/50/8/14 and
MK2/100/50/10/12 from R+W / ebmia.pl / archimedes.pl with the price anchors above as the
reference level (~100 € net/pc for the 100-size class, PLN/EUR list ≈ 99–122 €). Re-confirm the
required OD (40 mm, not 100 mm) and that only bores within 5–24 mm are orderable.
