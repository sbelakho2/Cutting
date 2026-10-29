# Cost evidence ledger (G47/U04) - not a build-cost claim

Deterministic output of `analysis/cost-basis/compute.py`.

- Historical baseline register: 44 dated observations (2026-10-13), sums by currency: EUR 2894.22, USD 489.19
- Current dated observations: 10 rows, sums by currency: EUR 483.99, USD 190.52
- Evidence rows without cost: 2 (blank is not zero - each is an open quote)
- Open cost elements: freight/duty (D1), machining quotes (D5), labour, metrology, qualification, consumables.
- ~70 % target test: NOT ESTABLISHED - both sides incomplete and like-for-like scope
  mapping (12.2) open (D6). Any saving figure at this stage would be fabricated.

Currency sums are arithmetic sums of single observations at actual purchase
quantities; they are ledger facts, not basket prices. No currency conversion is
performed without a dated conversion observation.
