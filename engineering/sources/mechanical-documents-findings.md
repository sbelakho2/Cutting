# Mechanical documents findings — 2026-09-16

Fetched live on 2026-09-16 to close the external-inquiry document gates
(ledger rows 30–34). Evidence rule: every number below was read from a
document fetched in this run (curl with Chrome UA for plain/PDF URLs;
webfetch for HTML), or from an existing repo archive whose SHA256 was
re-verified byte-identical against a fresh fetch of the live manufacturer
endpoint.

## 1. HIWIN KK8610P340A1F0 (ledger: HIWIN axis enquiry) — FOUND

Channels: hiwin.de product record (live, HTML), hiwin.tw KK page (live),
and the current HIWIN catalogue linked from the product record.

Archived: [hiwin-kk86-catalogue-2026.pdf](hiwin-kk86-catalogue-2026.pdf) —
HIWIN "Precision axes and precision systems" catalogue
**PS-11-1-EN-2605-K**, created 2026-05-22, 137 pages, 9.45 MB, SHA256
`1e75988ae59502d4…`, fetched from
`hiwin.de/medias/sys_master/hiwinDocumentMedia/h3e/h9a/14754699182110/…`
(`?attachment=true`).

Verified against the repo records (`shared/cutting_head_spec.hpp` and
`catalogue-evidence.md` AX-01). All agree; **no contradiction found**.

- Order code (§5.2): KK86 / lead 10 / precision class **P** (axial play
  0.005 mm; C class is 0.02 mm) / rail 340 / block **A** (standard) / 1
  block / flange **F0** / without cover / without limit switches.
  The catalogue now prints the model as `KK8610P0340`; same identity.
- Table 5.19 (KK86 without cover, standard block), `KK8610P0340`:
  L0 = **340** (rail), L1 = **440** (overall), lead **10**, maximum
  travel block A1 **194**, reserve stroke Lr **8**, G 70, n 3, m 2,
  mass **5.7** kg (A1).
- Section drawing (PDF page 38): **input shaft Ø8 h7, exposed 18 mm**
  (28 mm including locknut); fixed-end cavity **Ø46**; F0 locating bore
  **Ø50 +0.05/+0**; F0 attachment **4×M5×0.8×10DP on PCD70**, separate
  PCD60 pattern; block 116 mm overall, pad 60 × 75, 4×M6×1P×12DP,
  2×2-M4×0.7P×10DP; base 2×n-Ø6.6 through / Ø11×6DP at 46 transverse,
  100 longitudinal, 70 from the ends; 13 mm rail-end thickness;
  carriage 46 mm / screw axis 32 mm above the mounting plane.
- Tables 5.1–5.5: vmax **740 mm/s**, amax 15 m/s², positioning 0.025,
  repeatability ±0.003, guide parallelism 0.015, starting torque
  **150 Nmm = 0.150 Nm**; ballscrew Ø15, Cdyn 7144 / C0 12642 N; guide
  Cdyn 31458 / C0 50764 N; moments 1507 / 622 / 622 Nm; fixed bearing
  C0 axial 9240 N, **Fmax axial 6320 N**; inertias Ix 74450 / Iy
  1,134,000 mm⁴. New value not previously recorded: **max drive torque
  2.79 Nm** (Table 5.1).

**Shaft diameter answer: Ø8 h7** — the R+W MK2/100/50/**8**/14 choice
(8 mm stage-side bore) is confirmed; the 14 mm motor-side bore matches
the 34E1K-85 Ø14 shaft. The travel discrepancy (210 mm on the old 2007
drawing vs 194 usable + 8 reserve today) is already recorded in
`catalogue-evidence.md`; the 2026 catalogue confirms the 194/8 values.

## 2. R+W MK2 100 50 (ledger: R+W exact bores) — FOUND

Channels: rw-america.com June-2026 datasheet (fetched), rw-america.com
2023 datasheet (fetched), rw-couplings.com MK2 page (live, no PDF
links).

Both fresh fetches are **byte-identical to the existing archives**:
`rw-mk2-datasheet-2026.pdf` (SHA256 `f6c663d7…`) and
`rw-mk2-coupling.pdf` (SHA256 `68977013…`). No duplicate file added.

Size 100 row of the MK2 table:

| Quantity | Catalogue value | Repo record | Verdict |
|---|---:|---:|---|
| Rated torque TKN | **10 Nm** | 10 Nm | ✓ |
| Overall length A-1 | 50 or 60 mm (selected 50) | 50 mm | ✓ |
| Outside diameter B | **40 mm** | 40 mm | ✓ |
| Fit length C | **16 mm** | 16 mm | ✓ |
| Bore range D1/2 | **5–24 H7** | 8/14, 10/12 | ✓ both in range |
| Fastening screw / torque | M4 ISO4762 / **4.5 Nm** | M4 / 4.5 Nm | ✓ |
| Inertia / mass | 160 g·cm² / 120 g | 160 g·cm² / 120 g | ✓ |
| Torsional stiffness | 9050 Nm/rad | 9050 Nm/rad | ✓ |
| Misalignments | axial 1 / lateral 0.2 / angular 1.5 | same | ✓ |

Torque verdict: **10 Nm catalogue rated torque > 8.5 Nm motor holding
torque** (`cutting_head_spec.hpp`); the coupling is not a torque-limiting
clutch, as already recorded. The datasheet still does **not** dimension
the bellows interior — ledger row 30 (interior clearance) remains open.

## 3. Kübler 8.0000.3123.0006 (ledger: Kübler wheel) — FOUND

Channels: kuebler.com docu-finder (live HTML), measuring-wheel-systems
category page, and the product-finder detail page
`…/product-finder/product-details/measuring_wheels`, which exposed
`/pdf?Messrad_en.pdf`. Fetched `Messrad_en.pdf` is **byte-identical**
(SHA256 `65197ac8…`) to the archived `kuebler-messrad-en.pdf`
(2026-05-01, InDesign 21). No duplicate file added.

Newly archived: [kuebler-messradsysteme-2025.pdf](kuebler-messradsysteme-2025.pdf)
("Measuring wheel systems", D000090344, EN, 28 pages) from the category
page — system-level brochure (MWE11/21/31/41/61), supplementary context,
not the discrete-wheel authority.

Verified from the live `Messrad_en.pdf`:

- Order structure `8.0000.3XXX.00XX`: 100 mm circumference + natural
  rubber 60 Shore A = wheel No.**123** + bore `06 = 6 mm` ⇒
  **8.0000.3123.0006**, 16 g. Not listed among the stock types (page 3).
- Page 4: circumference **100 ±0.1 mm**, tread 9.5 mm; No.123 drawing
  **Ø31.83 ±0.025**, overall 14.5, bore **H7**, hub Ø16.5, end land
  0.75, **1 screw M4×6 ISO4029**, screw centre 2.5 from the hub end.
  No.113 (aluminium knurl) keeps its separate 4×M3×5 callout.

All values match `shared/measurement_spec.hpp` and docs/13 §1; the
wheel gate's dimension evidence is confirmed. Price remains unobserved.

## 4. Autonics E40S6-1024-6-L-5 (ledger: Autonics) — FOUND

Channels: autonics.com model page (live, server-rendered specs), the
model page's own data API `POST /us/model/data/init`
(modlCode `A2500001080`, seriesSn `3000482`) which returned the
download list, then `/common/download/<atchSn>`.

Archived: [autonics-e40s-datasheet-2025.pdf](autonics-e40s-datasheet-2025.pdf)
— E40 series **product manual TCD210019AD, 2025-07-21** (12 pp,
532,644 bytes, the EN attachment `E40_EN_TCD210019AD_20250721_MANUAL_W.pdf`),
and [autonics-e40s-2dcad.zip](autonics-e40s-2dcad.zip) — the E40S
2D-CAD attachment (6.26 MB, DWG family drawings; note the zip does not
contain the exact 1024 model name, but the shaft-type body geometry is
shared across the family).

Confirmed (live page + manual):

- Ø6 mm shaft; **1024 P/R**; **line-driver** output A,Ā,B,B̄,Z,Z̄;
  5 VDC ±5%; 300 kHz; 5000 rpm; starting torque max 40 gf·cm
  (**0.00392 N·m**); IP50; radial cable type, Ø5 mm 8-wire shielded;
  supplied Ø6 coupling standard (~120 g incl. packaging).
- Order code: `E40` + S shaft type + `6` Ø6 + `1024` + `6` (A,Ā,B,B̄,Z,Z̄)
  + `L` line driver + `5` (5 VDC) + no mark (radial cable). ✓
- Dimensions (manual drawing): 3-M3 **DP 5** on **Ø30** (PCD 30);
  pilot **Ø20 −0.021/0**; body **Ø40 × 31**; shaft **15** beyond the
  **5** mm pilot (tip **20** from the mounting face); shaft tolerance
  **Ø6 −0.004/−0.016**; flat **5** across, **10** long; coupling
  Ø15 × 22, bore Ø6 +0.1/0, 4-M3, screws 2.8 from the ends.

### FLAG — mismatch vs shared/measurement_spec.hpp (not edited)

The M201 plate check itself is **unchanged**: the pilot maximum stays
20.000 mm (M201 bore 20.040–20.060 keeps 0.040–0.080 diametral
clearance) and the 3-M3/PCD30 pattern is identical. Two tolerance
values in `measurement_spec.hpp` (transcribed from the older catalogue
page F-20) differ from the current 2025-07-21 manual:

| Field | spec.hpp | Current manual (2025-07-21) | Delta |
|---|---:|---:|---|
| encoder shaft min/max | 5.985 / 5.990 mm | Ø6 −0.004/−0.016 → 5.984 / 5.996 mm | band 0.005 → 0.012 mm |
| encoder pilot minus | 0.02 mm | 0.021 mm | −1 µm at minimum pilot |

Neither changes the M201 plate check; both are recorded here and in
`catalogue-evidence.md` as flags. `shared/*.hpp` was not edited.
Price remains unobserved.

## Channels tried and failed (context only)

- hiwin.it / hiwin.es product paths: 404 / redirect to shop home.
- rw-couplings.com: MK2 page has no PDF links; no per-bore pages.
- Kübler docu-finder page itself is a JS shell; the PDF was reached
  through the product-finder detail page instead.
- Autonics model page is JS-driven; the download list required the
  page's own `/us/model/data/init` POST with `seriesSn` (empty
  response without it).
