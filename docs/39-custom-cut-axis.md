# Custom cut axis — S1-B-AX-R1 (replaces the HIWIN KK86)

**Revision:** `S1-B-AX-R1-2026-09-17`
**Status:** selected axis design with the **lead/driver configuration frozen**
(§3, audit F13). The audit's F14 (finish the assembly/tooling drawing as a
manufacturing definition) and F15 (component-specific ratings and the
original 2500/5000 N acceptance loads, not the reduced 1200/2400 N set)
remain open before any fabrication release; the replacement is not closed
until the real assembly passes the original requirements. The axis replaces
the HIWIN KK8610P/C-340A1F0 (USD 824.72/unit at the only priced source) with a
custom ballscrew axis built from individually priced industrial components.
The carriage plate reproduces the KK86 interface exactly, so H003, the head
CAD, the layout checks and the adapter FEM geometry are all reused unchanged
(verified by the full test suite on the unchanged adapter solids).

## 1. Components per axis (live prices, 2026-09-17)

| Line | Item | Qty/axis | Price observed | Source |
|---|---|---|---|---|
| AX-R01 | HGR20 400 mm rail + 2× HGH20CA block set | 2 sets | US$8.00–13.00 FOB/set, MOQ 1 | [Taiyin on made-in-china](https://taiyin6666.en.made-in-china.com/product/ztLrHCpjCsci/China-Hgr20-High-Accuracy-High-Qualit-Linear-Guide-Rail-Set-Hgr20-400mm-Hg20-Linear-Rail-and-Block.html) |
| AX-S01 | SFU1610-class C7 500 mm rolled ballscrew, **10 mm lead**, + flange nut + DSG16h housing + BK12/BF12 supports (end-machined option) | 1 set | US$5.00–10.00 FOB, MOQ 1 | [ALM on made-in-china](https://autolinearmotion.en.made-in-china.com/product/PtaYZwvVEukB/China-Sfu1605-500mm-600mm-800mm-C7-with-End-Machined-1605-Ball-Nut-Housing-Bk-Bf12-End-Support-Coupler-Rolled-Ball-Screw.html) — order the 10 mm-lead (SFU1610-class) variant |
| H005 | Base plate 6082-T6, 20×86×440, milled rail seats, BK12/BF12 lands, 6×Ø6.6 grid 46/100 | 1 | own shop (sheet H005-H006) | custom |
| H006 | Carriage plate 6082-T6: 60×75 pad at 46 mm height, Ø49.97 F0 register, 4×M5 PCD70, 46×46 4×M6, DSG16h pocket, 4× HGH20CA lands | 1 | own shop (sheet H005-H006) | custom |

Per-axis component cost ≈ US$26–53 + own-shop plates ≈ US$40–60 →
**≈ US$70–115 per axis vs USD 824.72** for the priced HIWIN C-grade.

## 2. Interface reproduction (the CAD-envelope contract)

The custom axis must reproduce, within the existing envelope tolerances, the
values already encoded in `shared/cutting_head_spec.hpp` (unchanged):

- total length 440 mm, rail 340 mm, width 86 mm;
- carriage pad 60×75 at 46 mm above the mounting plane, 4×M6 on 46×46;
- Ø49.97 F0 register + 4×M5 on PCD70 at the carriage (H003 interface);
- screw input end Ø8 h7 × 18 mm exposed (R+W MK2/100/50/8/14 hub);
- Ø46 coupling service cavity at the fixed end;
- travel ≥194 mm usable + 8 mm reserve.

Screw end: order the end-machined Ø8 h7 option (the made-in-china listing
offers end machining) or machine locally; do NOT clamp a raw Ø12 blank.

## 3. Lead and control mapping — frozen single configuration (audit F13)

**Selected and frozen: SFU1610-class screw, 10 mm lead.** One configuration
only; the former 400/800/2000 pulses/rev and 5 mm-lead mixtures are removed
(they are recorded as superseded by this revision and hold no build
authority).

**Frozen driver setting:** CL86T-V41 at **1000 pulses/rev** (electronic
gearing/microstep setting per the driver manual, recorded in the switch
sheet at commissioning; BOM row DR-01 carries the same interface).

**One derivation, no alternatives:**

```
steps/mm = pulses_per_rev / lead_mm = 1000 / 10 = 100 pulses/mm
```

This is the value already frozen in the firmware source, which is the
source of truth the docs align to (not the reverse):

- `shared/cutting_head_spec.hpp`: `pulses_per_revolution = 1000`,
  `catalogue::lead = 10`, `steps_per_mm = pulses_per_revolution / catalogue::lead`
  = **100** pulses/mm;
- `shared/controls_spec.hpp`: `blade_steps_per_mm = head::design::steps_per_mm`
  (100 pulses/mm) and `blade_speed_mm_s` 12.5 → 1250 Hz command rate;
- `software/machine/core/controller.cpp` uses `ctrl::axis::blade_steps_per_mm`
  in `um_to_blade_steps()`/`blade_steps_to_um()` (controller.cpp:16–19) for
  every blade command and software limit.

At 12.5 mm/s a 10 mm lead is 1.25 rev/s (75 rpm), far below the screw
critical speed class. The 194 mm travel and 4.2 mm software limit are
commanded in these 100-pulses/mm coordinates only.

**Per-axis direction polarity (audit F33):** the driver direction setting is
not left to chance. At commissioning, each of the four axes has its driver
direction DIP/parameter setting frozen in the build record, and the machine
coordinates, stock/piece travel signs and encoder counting sign are proven
per [the commissioning runbook](36-commissioning-runbook.md) steps P6-1
(staged energisation: polarity, encoder direction, drive enable) and P6-7
(encoder direction and calibration). A wrong-sign or missing-feedback result
blocks the axis.

## 4. Physics verification (all computed; sources stated)

- **Load:** working 1200 N (docs/37 blade-force bound) / proof 2400 N
  (2.0×). Audit F15: these are the estimated *drive* figures, not the shared
  acceptance envelope — the original 2500 N axial / 750 N transverse /
  375 N vertical (5000 N double proof) acceptance loads remain in force, and
  component-specific (HGR20/SFU1610/BK12) verified capacities must be
  supplied before the custom option is closed; HIWIN ratings do not transfer
  to it. HGR20 block dynamic rating ≈11.4 kN/block (supplier class,
  verified against the supplier rating at receipt) → ≥9.5× per block for the
  1200 N estimate; the four blocks share the load.
- **Screw buckling (SFU1610-class, stated root Ø≈13 mm, L=500 mm,
  pinned-pinned — root/bearing data to be confirmed against the purchased
  screw's datasheet; audit F15 keeps the component-specific rating closure
  open):**
  F_cr = π²EI/L², I=π·13⁴/64=1402 mm⁴, E=210 GPa → **≈11.6 kN** = 4.8×
  the 2400 N proof.
- **Screw critical speed:** ≈2600 rpm at 500 mm (1st mode, fixed-floating
  correction) vs the **75 rpm** max blade duty (12.5 mm/s at the frozen
  10 mm lead) → ≈35×. Ball-nut DN/duty limits to be confirmed from the
  purchased screw's datasheet (audit F15).
- **Carriage plate deflection (H006, 6082, 60×75×20 effective):**
  cantilever/beam worst case at 1200 N through the nut pocket ≈ **0.004 mm**
  — negligible against the ±0.1 mm placement budget.
- **Positioning:** C7 rolled screw accuracy ±0.05 mm/300; preloaded nut +
  ground blocks give unidirectional repeatability ±0.01–0.02 mm class. The
  cut-axis placement budget is **±0.1 mm** (re-derived: the blade must hit
  the cut point on the wire; length accuracy is measuring-wheel based) →
  C7 with margin; the old ±0.025 mm spec value was inherited from the KK86
  P-grade and is superseded by this re-derivation (recorded in
  `cutting_head_spec.hpp` comment and docs/37).
- **Life:** HGH20CA blocks at 1200 N / 30 cuts-min / 10⁴ h → L10 ≈ 10⁶ h
  class; screw C7 life at 1200 N vs 7.1 kN dynamic ≈ 10⁵ h class — consumable
  margins >100× the duty.

## 5. Assembly and qualification

1. Mill H005 rail seats flat (0.02/300) and bolt the two HGR20 rails at
   60 mm pitch; align with a gauge bar; torque M5 6 Nm.
2. Bolt H006 lands onto the four HGH20CA blocks; mount the DSG16h nut
   housing in its pocket; install the screw through BK12 (fixed, preloaded
   angular bearings) and BF12 (floating); adjust nut preload per the
   supplier setting; verify drag.
3. Mount H003 (unchanged) to the H006 pad (Ø49.97/PCD70/46×46) and the
   34E1K-85 + R+W coupling to the Ø8×18 screw end (same axial stack rules:
   ≥0.53 mm hub insertion, ≥12.58 mm end gap).
4. Run-in 1 h; re-measure block preload and repeatability (±0.02 mm target
   over the 194 mm travel, DTI); record in the build record.
5. The adapter FEM geometry is unchanged (same H003/bolts/loads); the
   previous adapter studies remain the governing record for H003.

## 6. Superseded options (recorded, not selected)

KK8610C-340A1-F0 at USD 824.72 (MISUMI VN, priced 2026-09-16) and the eBay
item 406408632737 (genuine P-grade, US$450 snippet) remain as fallback
purchase leads if the component supply fails; Tiayo/Seatyle/clone modules
were rejected on the published load ratings (axis-replacement-findings.md).
