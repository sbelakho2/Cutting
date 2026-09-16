# Electrical document retrieval — findings (2026-09-16)

Purpose: close the EXTERNAL-INQUIRY ledger rows by retrieving the actual
manufacturer documents. Retrieved PDFs are archived in this directory.

## 1. Schneider LC1D12BD DC utilisation — TeSys Deca catalogue

- **Retrieved:** TeSys Deca / TeSys catalogue `MKTED210011EN` (2026
  edition) from `download.schneider-electric.com/files?p_Doc_Ref=MKTED210011EN`
  (HTTP 200, 53.6 MB) → archived as
  [schneider-tesys-deca-catalogue-2026.pdf](schneider-tesys-deca-catalogue-2026.pdf);
  compact page extract [schneider-tesys-dc-tables-fuse-carriers.pdf](schneider-tesys-dc-tables-fuse-carriers.pdf)
  (catalogue pages A5/120–121 and B4/3–5, B4/9–11).
- **Data** (catalogue pages A5/120–121, tables "DC-1" with L/R ≤ 1 ms and
  "DC-2 to DC-5" with L/R ≤ 15 ms, contactors LC1D09…D38, ambient ≤ 60 °C,
  rated operational current Ie in A for 1/2/3/4 poles in series). LC1D12
  column:
  - 24 V: 20 A (1/2/3 poles) in both tables;
  - 48/75 V: 20 A (1/2/3 poles) in both tables;
  - 125 V: DC-1 4/20/20 A; DC-2..5 2/20/20 A;
  - 250 V: DC-1 1/4/20 A; DC-2..5 0.5/2/8 A;
  - 300 V: DC-1 –/–/4 A; DC-2..5 –/–/2 A;
  - 460 V: – (3 poles, both tables).
- **Computation for the 36/60 V branches (3 poles in series):** 36 V and
  60 V are both inside the 48/75 V row → rated operational current **20 A**
  in DC-1 and DC-2..5. Margins: 20/9.7 = **2.06×** (K3, 36 V) and
  20/5.8 = **3.45×** (K1/K2, 60 V).
- **Load time constant:** branch cable inductance (µH/m scale) against the
  branch resistance (36 V/9.7 A ≈ 3.7 Ω; 60 V/5.8 A ≈ 10.3 Ω) gives
  L/R ≈ 0.1–0.3 µs, far inside the DC-1 row (L/R ≤ 1 ms); the DC-2..5 row
  already covers up to 15 ms. The driver-input capacitance (hundreds of µF)
  affects the make (capacitor-charge inrush, LE-350 soft-start limited),
  not the break L/R.
- **Catalogue caveat carried over:** "In DC applications, lower current
  values than the ones specified in these tables may lead to durability
  performances lower than defined in IEC 60947-4-1 (critical current)" —
  at 5.8/9.7 A vs the 20 A table value, contact durability stays a measured
  commissioning check, as does the measured K1/K2/K3 opening behaviour
  (docs/22 §8.5).
- **VERDICT: the 3-pole-series arrangement covers 36/60 V at 5.8/9.7 A per
  the TeSys Deca DC tables.** CT-K1-K3 / CB-K1-K3 move to
  `selected_DC_duty_qualified` (energisation holds unchanged).

## 2. Bernstein SLC-F-024-20/20-R4 (6018200007) + ACF-1 (3911742391) mounting documentation

- **Retrieved:** Bernstein installation/operating instructions document
  `0800000865` (issue 2022-07-12, 24 pp., DE/EN/FR) and technical datasheet
  `6018200007` (2271-21, 3 pp.). The bernstein.eu download area serves only
  series catalogues; the SLC-specific PDFs were recovered from the
  automation24-hosted manufacturer files
  (`media.automation24.com/manual/en/0800000865.pdf`,
  `media.automation24.com/datasheet/en/6018200007 en.pdf`), which carry the
  Bernstein document numbers. Archived as
  [bernstein-slc-installation-manual-0800000865.pdf](bernstein-slc-installation-manual-0800000865.pdf)
  and
  [bernstein-slc-f-024-20-20-r4-datasheet.pdf](bernstein-slc-f-024-20-20-r4-datasheet.pdf).
  The bernstein.eu safety-technology catalogue confirms the article numbers
  (SLC-F-024-20/20-R4 = 6018200007, ACF-1 = 3911742391).
- **Mounting geometry (manual p.8 drawing + datasheet):**
  - switch envelope **30 × 192 × 44 mm** (drawing width callout 30.4),
    weight ≈0.484 kg, IP67, protection class II;
  - fixing **4 × M5** (drawing hole callouts: Ø5.4 clearance; rows 161.6 mm
    apart vertically with 15.2 mm inset from the 192 mm ends; columns 22 mm
    apart on the 30 mm face; actuator opening 29 mm wide);
  - flat screw-on surface required, any orientation, form-fit mounting per
    ISO 14119; one-way screws EINWEGSCHR M5×25/M5×40 (6054999018/6054999021)
    available for tamper resistance;
  - cable entries **3 × M20×1.5**; holding force 1500 N (ISO 14119);
  - actuator alignment: minimum travel **X/Y = 29 + 3 mm**; **2 mm
    overtravel** between head and actuator; guard stop must be set inside
    **0.2–2 mm**; no lateral forces on the head; switch must not be used as
    an end stop;
  - ACF-1: stainless actuator, approach radius **R ≥ 400 mm**; actuator-head
    screws TX10 at 1.6 Nm (ACR-1 TX8 at 1.5 Nm).
- The exact hole coordinates must be transferred from the archived drawing
  (p.8) to the G106 fabrication sheet; the physical fit and locking-travel
  check stays (docs/26 §6).

## 3. Schneider DIN-rail 10×38 fuse base identity

- **From the same TeSys catalogue** (pages B4/4, B4/10, B4/11 — "Fuse
  carriers, DF type"): the modular fuse carriers for 10×38 links (DF2CNpp
  gG / DF2CApp aM) are the **DF10** family, mounted on **35 mm DIN rail
  (EN 60715)**:
  - **DF101** — 1 pole, Ith 32 A, Ui 690 V, Uimp 6 kV, IP20, 17.5 mm
    (1 module) wide, padlockable, max fuse dissipation 3 W, conditional
    short-circuit 120 kA at 400/500 V, IEC/EN 60947-3 + IEC/HD 60269-2,
    UL/CSA, DNV-GL, ambient −20…+60 °C;
  - family: DF101N (1P+N), DF102 (2P), DF103 (3P), DF103N (3P+N); LED
    "blown fuse" variants DF101V / DF101NV / DF102V / DF103V / DF103NV;
    assembly kit DF10AP (pin + clips).
- **The DIN-rail base accepting a DF2CN02/DF2BA 10×38 link is DF101.**
  Price: not observed (no shop listing reached 2026-09-16) — acceptable per
  the ledger hold.
- The design moved to the priced 5×20 solution (docs/22 §11.3); DF101 is
  recorded as the confirmed catalogue identity for ledger gate 38 and is
  not reintroduced into the wiring design.

## 4. Siemens 6EP3332-6SB00-0AY0 datasheet (24 V branch coordination)

- **Retrieved:** official Siemens data sheet from
  `apim.industry.siemens.cloud/ted/datasheet?format=pdf&mlfbs=6EP3332-6SB00-0AY0&language=en&caller=SIOS`
  (HTTP 200) → archived as
  [siemens-6ep3332-6sb00-0ay0-datasheet.pdf](siemens-6ep3332-6sb00-0ay0-datasheet.pdf)
  (5 pp., dated 2026-09-08).
- **Key numbers:** input 100–240 V AC (85–264 V) / 110–300 V DC; output
  24 V DC / 2.5 A (60 W), adjustable 22.2–26.4 V, ripple ≤200 mV, soft
  start (no overshoot), 0.5 s response; mains buffering **40 ms** at
  Vin 187 V; inrush limit **52 A** / I²t ≤3 A²s; **overload capability
  150 % of Iout rated (3.75 A) typical for 200 ms** (switch-on and normal
  operation); **short-circuit protection: constant-current characteristic,
  3.2 A typical** (enduring short-circuit current max 3.2 A); derating
  2 %/K above 55 °C up to 70 °C; efficiency 89.6 %, loss 7 W at rated
  load, 0.3 W no-load; parallel operation up to 2 units; SELV / ES1 /
  NEC Class 2; 54 × 90 × 53 mm, DIN rail EN 60715 35 mm, 20 mm top/bottom
  spacing, 0.2 kg; MTBF 2 864 520 h at 40 °C.
- **Use:** the 24 V branch protection (CB-XPS-FUSE 2 A 5×20 link, TB24-01
  distribution) is coordinated against the 3.2 A constant-current
  short-circuit limit and the 3.75 A / 200 ms overload ceiling of the PSU;
  prospective-fault coordination remains a commissioning measurement.

## Channels used

- download.schneider-electric.com (curl, Chrome UA): 200 for both document
  refs — the primary source for goals 1 and 3;
- automation24 (webfetch): product pages + lazy "AssetsGroups" download
  panel — recovered the Bernstein manual/datasheet URLs; the CAD zip on
  media.automation24.com is Cloudflare-managed-challenge 403 to curl;
- bernstein.eu (curl, Chrome UA): TYPO3/Vue app; `/en/service/downloads`
  and fileadmin catalogue PDFs reachable (complete safety catalogue
  downloaded and searched); SLC-specific PDFs not on the downloads page;
- apim.industry.siemens.cloud (curl): 200 — official Siemens data sheet;
  cache.industry.siemens.com reachable but attachment-id guess 404;
  mall.industry.siemens.com and support.industry.siemens.com 403 to
  automated fetch;
- DuckDuckGo html endpoint (webfetch): usable for locating the datasheet
  URLs; Bing/Google return bot-noise or block;
- conrad search page returns an empty JS shell to automated fetch
  (product pages themselves remain SSR-reachable).
