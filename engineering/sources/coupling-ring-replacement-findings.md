# Coupling/ring replacement findings — R+W MK2 same-class check, A12 ring substitution, CL57T wholesale anchor

**Research date:** 2026-09-16 (all pages fetched live on this date).
**Scope:** close the last enquiry-only hardware lines: the two R+W MK2
coupling bore pairs (8/14 ×2 and 10/12 ×2), the SEEGER-ORBIS A12 ×8 rings,
and one made-in-china wholesale search for the CL57T driver.
**Evidence rule:** price/stock claims are only read from live fetched pages
(URL + price + currency + availability recorded).

## 1. Couplings — kept R+W MK2, ebmia.pl is the order path

### 1.1 Exact-bore re-searches (ebmia.pl, live 2026-09-16)

| Query | Result |
|---|---|
| `https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50%2F8%2F14` | “Brak wyników wyszukiwania” — no results |
| `https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50%2F10%2F12` | “Brak wyników wyszukiwania” — no results |
| `https://www.ebmia.pl/szukaj?controller=search&s=MK2%2F100%2F50` | 2 items, re-confirmed live: MK2/100/50/10/14 at **530,35 zł gross / 431,18 zł net** (0 stock, 7–14 days) and MK2/100/50/8/21,4 at **418,85 zł gross / 340,53 zł net** (24 h). EUR view of the 10/14 page: **EUR 122.30 gross / 99.43 net** per pc |

The exact bore combinations still do not exist as priced catalogue items
anywhere reachable. **Order path: ebmia.pl made-to-bore quote items for
MK2/100/50/8/14 ×2 and MK2/100/50/10/12 ×2 at the class price
(≈EUR 99.43 net/pc anchor); R+W direct is the fallback.** BOM rows CP-01
and TR-CP01 record this path; L2 gains the ebmia order instruction.

### 1.2 Same-class priced candidates (ebmia generic searches `sprzeglo 8/14`, `sprzeglo 10/12`, live 2026-09-16)

| Item | Bores | Dims | Torque | Price (gross/net, PLN) | Availability | Verdict |
|---|---|---|---|---|---|---|
| Z-SP-SBK19-8/14+K-98SHA (also -64SHD/-80SHA/-92SHA) | 8 / 14 | D40 × L66 | **32 Nm** | 321,88 / 261,69 … 326,33 / 265,31 | term not stated in listing | **REJECTED — jaw backlash** |
| Z-SP-SBK19-10/12-98SHA (also -64SHD/-80SHA/-92SHA; +K variants 10/12.7) | 10 / 12 | D40 × L66 | **32 Nm** | 267,96 / 217,85 … 272,41 / 221,47 | term not stated in listing | **REJECTED — jaw backlash** |
| Z-SP-SBK14-8/14-*, SBK14-10/12-* (+K variants) | 8/14, 10/12 | D30 × L35 | 5.5 Nm | 196,49–220,05 / 159,75–178,90 | 3–5 days | REJECTED — 5.5 Nm < 8.5 Nm |
| IW-SP-JM30C / JM40C 8/14, 10/12(.7) jaw sets | 8/14, 10/12(.7) | D30 / D40 | not listed (jaw class) | 53,76–97,63 / 43,71–79,37 | 3–5 days | REJECTED — jaw backlash |
| SP BKL3/10/12 (R+W bellows, backlash-free) | 10 / 12 | OD 25 (B), L 32, hub 10, bore max 12.7 | not listed; undersized class | 130,38 / 106,00 | **24 h** | REJECTED — not the 40×50×16 / 10 Nm class |
| SP-GS14/1.0/12+K L11 (KTR ROTEX GS 14 hub only) | 12 (hub) | hub only | — | 78,84 / 64,10 | 24 h | REJECTED — hub only |

Conrad.com (`/en/search?search=ROTEX GS`) HTTP 403, conrad.de search 404;
automation24 search paths (`/search?query=ROTEX`, `/search?sSearch=ROTEX`)
HTTP 404 — all recorded as attempted-blocked (consistent with
sourcing-findings-rw.md §2).

### 1.3 Torque/backlash verdict (the gate for jaw/star couplings)

Jaw-coupling acceptance requires a recorded backlash consequence and
torque ≥ 8.5 Nm. The SBK19 pairs pass torque (**32 Nm ≥ 8.5 Nm** motor
holding torque) but fail the backlash gate:

- 0.5° angular backlash at the 10 mm lead = 0.5/360 × 10 mm =
  **0.0139 mm ≈ 14 µm**.
- Cut-positioning tolerance from `shared/cutting_head_spec.hpp`
  (`positioning_mm = 0.025` = 25 µm, `repeatability_mm = 0.003` = 3 µm;
  docs/01 length target ±(0.20 mm + 0.05 %), strip ±0.20 mm): **14 µm is
  56 % of the entire 25 µm positioning budget and 4.6× the ±3 µm
  repeatability.**
- The CL86T/CL57T encoders are motor-shaft encoders: coupling dead band is
  **not closed** by the drive loop, and jaw inserts wear the gap open in
  service.
- Geometry also fails: SBK19 is L66 with 11 mm hubs versus the frozen
  49–50 mm free-length / 16 mm-hub stack (spec.hpp `coupling_length`
  50/49–50); BKL3/10/12 (backlash-free but OD 25, L 32, hub 10, bore max
  12.7) is not the 10 Nm class.

**Verdict: no priced exact-bore item passes; the zero-backlash R+W MK2
bellows selection is retained** and the exact bores are ordered from
ebmia.pl at the class price. Recorded in BOM CP-01 / TR-CP01, docs/32,
docs/35 L2, docs/33 gate 32 and catalogue-evidence.md (CP-01).

## 2. A12 rings — substituted by the priced DIN 471 12×1 Wegertseder ring

### 2.1 Selection

**Wegertseder Art. 1950-010 “12 x 1”**, DIN 471 Sicherungsringe für Wellen,
Regelausführung, **Federstahl** — Kleinpaket **100 St. 8,57 €** (= EUR
0.0857/pc incl 19 % VAT; 1.000 St. 39,98 €; 5.000 St. 183,86 €), in stock
(green availability marker), live-verified 2026-09-16:

<https://www.wegertseder.com/DIN-471-Sicherungsringe-fuer-Wellen-Regelausfuehrung-FEDERSTAHL-Diameter-12>

BOM TR-RR01 now records Wegertseder 1950-010 (USD 0.0993/pc = EUR 0.0857 ×
ECB 1.1592, 2026-09-11 conversion; NOT a USD vendor quote). docs/35 L7 is
resolved (no enquiry), docs/33 gate 37 closed, docs/28 finding 7 updated.

### 2.2 Groove-fit arithmetic (shaft groove Ø11.45±0.02)

Ring per the Wegertseder page PDF: **d3 = 11.0 mm** (free inside diameter),
**s = 1.0 mm** (−0.06 → 0.94–1.00); DIN 471 nominal groove **d2 = 11.5**
(−0.11 → 11.39–11.50); the STARZ groove 11.45±0.02 (11.43–11.47) lies
inside the DIN band.

- **Install spread:** the ring opens from 11.0 to the 12.0 shaft OD
  (1.0 mm elastic spread) and seats at the groove bottom.
- **Seated spread:** 11.43–11.47 (groove) − 11.0 (d3) = **0.43–0.47 mm ≈
  0.5 mm residual spread**. Because d3 < groove bottom in every corner,
  the ring always grips the groove — no radial float, retention
  guaranteed.
- **Radial protrusion:** seated OD ≈ 11.45 + 2 × 1.0 = 13.45 mm →
  ≈(13.45−12.0)/2 = **0.72 mm per side** above the shaft — adequate
  shoulder for the retained stack.
- **Axial float:** groove width 1.12±0.02 (1.10–1.14, DIN minimum 1.10)
  − ring thickness 0.94–1.00 → **0.10–0.20 mm** — acceptable; no function
  depends on a tight A12 seat (docs/28 finding 7). ZGA-1 pliers per the
  Seeger catalogue.

**Fit verdict: PASS.** The substitution needs no CAD change; the 100-pc
pack covers ×8 with spares. Record the receipt lot in
incoming-inspection-template.csv.

## 3. CL57T made-in-china anchor — none found

`https://www.made-in-china.com/multi-search/CL57T/F1/1.html` (live
2026-09-16): the page reports “300 results” for “CL57T”, but page 1
contains no CL57T closed-loop stepper driver product — the visible items
are unrelated (CNC lathe US$50,000, lifting clamps US$21.50–35.90, AC
motor, …). **No wholesale CL57T offer with a concrete price was found**;
nothing is added to docs/35 L6, and StepperOnline remains the selected
channel with the amazon.de class anchors (EUR 173.78 B0FYPDGC85 / 175.45 /
189.10) recorded in sourcing-findings-drive-rings.md (attempt row added
there).

## 4. Files touched (2026-09-16)

- `bom/selected-transport-and-cabinet.csv` — TR-CP01 (ebmia order path +
  rejection basis), TR-RR01 (Wegertseder substitution, priced).
- `bom/selected-cutting-head.csv` — CP-01 (ebmia order path + rejection
  basis).
- `docs/32-sourcing-list.md`, `docs/35-purchase-enquiries.md` (L2 ebmia
  order instruction; L7 resolved), `docs/33-gate-ledger.md` (gates 32/36/37),
  `docs/28-physical-contradiction-audit.md` (finding 7 arithmetic).
- `engineering/sources/catalogue-evidence.md` (CP-01, TR-03),
  `engineering/sources/sourcing-findings-rw.md` (§5),
  `engineering/sources/sourcing-findings-drive-rings.md` (CL57T attempt).
- This file.
