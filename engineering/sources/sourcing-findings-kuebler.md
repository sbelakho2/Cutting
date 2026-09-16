# Sourcing findings — Kübler 8.0000.3123.0006 measuring wheel

**Date:** 2026-09-16 · **Searcher:** main session (the delegated agent returned no output)
**Verdict: no public price found; distributor-only. All attempts below were executed live.**

| # | Channel | URL / query | Result |
|---|---|---|---|
| 1 | Kübler category | https://www.kuebler.com/en/products/measurement/linear-measuring-systems/measuring-wheel-systems | Live page: describes 100/200/300 mm wheels and coatings; **no part numbers or prices server-rendered** (product finder is JS) |
| 2 | Kübler sitemap | https://www.kuebler.com/sitemap.xml | Returns the HTML homepage (soft redirect); no product URLs exposed |
| 3 | Kübler shop | https://kuebler.com/shop, https://shop.kuebler.com | Country-selector shell / domain does not resolve |
| 4 | Conrad | https://www.conrad.com/en/search?search=8.0000.3123.0006 | No results |
| 5 | Conrad (family) | https://www.conrad.com/en/search?search=Kubler%20measuring%20wheel | Kübler Perambulator 05.2400.0040.1000.5045, **EUR 395.00** (family price class reference) |
| 6 | Amazon.de | https://www.amazon.de/s?k=Kuebler+Messrad | No encoder wheel; surveying wheels (Bosch/Nedo/Stanley) and generic encoder wheels 200/300/360 mm, EUR 79.49 / 2 pcs |
| 7 | Amazon.de exact | https://www.amazon.de/s?k=8.0000.3123.0006 | Reviewed: no Kübler SKU listing |
| 8 | Motion Control Products | https://motioncontrolproducts.com/search?q=8.0000.3123.0006 | Live: "No exact matches" |
| 9 | eBay.de | https://www.ebay.de/sch/i.html?_nkw=Messrad+8.0000 | HTTP 403 (bot wall) |
| 10 | Automation24 | https://www.automation24.com/kuebler | 404; encoder category checked earlier contains ifm/Turck/Baumer etc., no Kübler wheels |
| 11 | Farnell | search rate-limited after earlier queries | Not obtainable this session |
| 12 | Simply Bearings | Shopify suggest API for wheel-class terms | Sells bearings only, no encoder wheels |

**Conclusion.** The exact Kübler 8.0000.3123.0006 exists in Kübler's catalogue but its
price is not published on any channel reachable from this environment. The priced
adjacent options for the measurement-wheel function are recorded in the BOM
evidence (Kübler Perambulator family EUR 395; generic Amazon encoder wheels
EUR 79.49/2 pcs; ifm encoder alternatives priced at Automation24). Buying the
selected Kübler wheel requires the Kübler distributor inquiry defined in the
master plan §2.2.
