# Electrical sourcing closure — 2026-09-16

Closing the last three unselected electrical identities of the S1-B wire
cutting machine internal build. All prices below were read from live pages
fetched on 2026-09-16; every observation records URL, price basis, currency
and the stock text shown on the page.

## 1. AC incoming — HOLD-AC-INCOMING → CB-AC-INCOMING (selected)

**Selection: Schneider Acti9 iC60N 2P C10 10 kA MCB (A9F54210) +
Schneider Acti9 iID 2P 25 A 30 mA type A RCD (A9Z21225).**

Sizing basis: the 230 V feed draws 4.6 A (1050 W, docs/22 §11.4). The C10
MCB carries it with SMPS inrush margin (C-curve magnetic trip 5–10×In) and
protects the 1.5 mm² feed. The iID type A 30 mA covers the PELV
switch-mode supply set.

- **RCD (priced):** A9Z21225, EUR 35.99 **incl VAT**, currency EUR,
  5 pieces in stock, "Fast track dispatch on Fri, 18 Sep" —
  https://www.conrad.com/en/p/schneider-electric-a9z21225-a9z21225-rccb-a-25-a-0-03-a-240-v-1861630.html
  (recorded as the row source).
- **MCB (price not observed, honest attempts):** A9F54210. Channels
  exhausted on 2026-09-16:
  - automation24.com — category slugs (`/circuit-breakers`,
    `/residual-current-circuit-breakers`, `/automatic-circuit-breakers`),
    search routes (`/search/...`, `/en/Search`, `/catalogsearch/result`)
    return 404 or JS-only pages with no readable listings;
  - conrad.com — the full reachable Schneider iC60N/iC60H listings carry
    1P (A9F03110 10 A 230 V, EUR 5.49), 3P (A9F03325 3P B25; A9F04313 3P
    C13) and 4P (A9D67416 4P RCBO C16 30 mA, EUR 269) only; no 2P C-curve;
  - ebmia.pl — only three iC60N SKUs: A9F04602 (1P+N C2 6 kA),
    A9F03202 (2P B2 6 kA, PLN 109.97 gross), A9F03206 (2P B6 6 kA,
    PLN 93.39 gross / 75.93 net); B-curve 6 kA was rejected as a
    spec-down-grade for a three-SMPS inrush load;
  - se.com product pages return 403 to automated fetch (as already
    recorded in docs/22 §11.6).
  The MCB stays `not_observed` and must be confirmed on a live distributor
  page at purchase; the BOM row keeps the exact maker part number.

## 2. XPS fuse holder — HOLD-FUSE-HOLDER closed by switching to 5×20

**Selection: Phoenix Contact 3046032 fuse serial terminal (already
CB-TB-DL, EUR 7.99) + ESKA 522620 2 A T 5×20 link (EUR 1.49).**

Equivalence reasoning (recorded in docs/22 §11.3): the DF2CN02 2 A gG
10×38 link and the 522620 2 A T 5×20 link fuse the same ≈0.45 A contactor-
coil load at the same 2 A nominal rating; the IEC 60127 T characteristic is
the 5×20 counterpart of the gG overload class; the 250 V DC link voltage
far exceeds the 24 V DC circuit; the 6.3 A terminal rating is never
approached. The unpriced Schneider 10×38 DIN-rail holder is no longer
required.

- 3046032 terminal: EUR 7.99 incl VAT, 34 in stock —
  https://www.conrad.com/en/p/phoenix-contact-3046032-fuse-serial-terminal-6-2-mm-screws-black-1-pc-s-744510.html
- ESKA 522620 link: EUR 1.49 incl VAT, 1368 in stock, fast track dispatch
  Fri 18 Sep —
  https://www.conrad.com/en/p/eska-522620-522620-dc-fuse-o-x-l-5-mm-x-20-mm-2-a-250-v-dc-time-delay-t-content-1-pc-s-1414701.html

Rows updated: CB-XPS-FUSE (new identity), HOLD-FUSE-HOLDER removed from
both selected BOM CSVs, CT-FSR01 re-identified
("Phoenix 3046032 fuse terminal + 2 A 5x20 link"), SRC-CT-FSR01 re-written
with the new identity/price fields, docs/22 §11.3, docs/24 §2.1/§2.2/§10,
docs/32 and docs/31 §7 updated.

## 3. DC branch protection decision (docs/22 §10 hold 3 desk part)

**Decision: the 5.8/5.8/9.7 A power branches are NOT fused with 5×20
links (class ends at 6.3 A).** The K1/K2/K3 three-series-pole contactor
arrangement plus the LE-350 overload protection (self-recovery, archived
datasheet) is the branch protection, with the §11.4 wire-ampacity margins
holding. The three drive LOGIC supply feeds (PUL/DIR/ENA common-anode
rails, a fraction of an amp each) get **3× Phoenix 3046032 fused terminals
with 1 A T 5×20 links (ESKA 522617)** — new row CB-TB-DL-K1-K3; the
+24V_DRVLOGIC common feed (CB-TB-DL) uses the same 1 A link.

- ESKA 522617 link: EUR 1.49 incl VAT, 224 in stock, fast track dispatch
  Fri 18 Sep —
  https://www.conrad.com/en/p/eska-522617-522617-dc-fuse-o-x-l-5-mm-x-20-mm-1-a-250-v-dc-time-delay-t-content-1-pc-s-1414698.html
- 3046032 terminal: same page/price as §2 above.

Remaining (not desk): short-circuit coordination review, inrush and bus-
decay measurement on assembled hardware (docs/22 §10 hold 3); the
LC1D12BD DC duty manufacturer answer (unchanged).

## 4. Files changed

- `bom/selected-cabinet-wiring.csv` — CB-XPS-FUSE re-identified; CB-TB-DL
  note updated; new CB-TB-DL-K1-K3 (3×); HOLD-AC-INCOMING → CB-AC-INCOMING
  (selected); HOLD-FUSE-HOLDER removed.
- `bom/selected-controls.csv` — CT-FSR01 re-identified; HOLD-FUSE-HOLDER
  and HOLD-DC-BRANCH-PROTECTION rows removed.
- `bom/sourcing-evidence.csv` — SRC-CT-FSR01 row rewritten (identity =
  controls identity, EUR 7.99 terminal price, link price in reference
  basis, next-evidence action).
- `docs/22-controls-and-wiring.md` — §6 TB-DL row, §10 holds 3–4,
  §11.3 rewrite, §11.4 branch-protection decision, §11.6, §11.7.
- `docs/24-production-and-release-plan.md` — §2.1 register rows, §2.2
  register rows, §6.1, §9 electrical row, §10.4 warning.
- `docs/32-sourcing-list.md` — 3046032 qty 5; DF2CN02/DF2BA0200 marked
  superseded; 10×38 holder removed; RCD + fuse rows added; MCB added to
  the open list.
- `docs/31-fabrication-guide.md` — Phase B order table rows; §7 XPS fuse
  warning.
- This file.

Not touched: `shared/*.hpp`, `engineering/src/*`, and all other BOM CSVs.

## 5. Remaining honesty boundaries

- The incoming MCB price is `not_observed` (attempts above); the RCD is
  priced. The pair must pass the docs/22 §10 local-regulation review and
  the §6 competent review before purchase.
- All prices are merchandise observations incl VAT at Conrad (plus
  shipping), 2026-09-16; no landed-cost comparison exists.
- Cabinet energisation remains gated by docs/22 §10.
