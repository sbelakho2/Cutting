# Cost evidence ledger (G47/U04) - not a build-cost claim

Deterministic output of `analysis/cost-basis/compute.py`.

- Historical baseline register: 44 dated observations (2026-10-13), sums by currency: EUR 2894.22, USD 489.19
- Current dated observations: 14 rows, extended at BOM purchase quantity (unit x quantity), sums by currency: EUR 1037.87, USD 532.84
- Evidence rows without cost: 2 (blank is not zero - each is an open quote)
- Open cost elements: freight/duty (D1), machining quotes (D5), labour, metrology, qualification, consumables.
- ~70 % target test: NOT ESTABLISHED - both sides incomplete and like-for-like scope
  mapping (12.2) open (D6). Any saving figure at this stage would be fabricated.

Current-side sums are line extensions (unit price x BOM quantity); the
historical side remains a sum of unit observations (its quantities live in the
historical-parts mapping and are not re-derived here). Ledger facts, not basket
prices: no currency conversion without a dated conversion observation.
