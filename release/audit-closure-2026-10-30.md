# Audit closure record — review of completion/2026-10-29

**Review baseline:** independent review of commit `62c873b` (30 October 2026).
**Closure work:** commits on `completion/2026-10-29` responding to findings
F01-F15. This record maps every finding to its final disposition with evidence.
A finding is **FIXED** only with a passing regression or regenerated artifact;
**OPEN** items name their exact blocker and the artifact already prepared for
it. Nothing is marked done without evidence.

## Finding dispositions

| finding | disposition | evidence / blocker |
| --- | --- | --- |
| F01 1 ms spike certifies waste removal | **FIXED** | two-edge confirmed-state publication for both evidence channels; regression `sensor_noise` (trapped stub + 1 ms glitch -> fault, accepted=0; discharge glitch -> fault) in the 30-group suite |
| F02 lifetime counter regressed | **FIXED** | `pieces_total` incremented once per accepted piece with documented persistence policy; `cut_cycles` wear counter added; regression `counters` verifies numeric totals across batches and reads saved flash bytes |
| F03 target cannot run production cycle | **OPEN (hardware)** | architecture DECIDED (analysis/phase-h-io/decision.md: board respin, 5 native channels, rejected alternatives recorded); PCB/layout/bring-up/HIL remain hardware work; occurrences registered (bom/phase-h-io-occurrences.csv) |
| F04 common-mode wheel error passes | **FIXED** | independent per-piece D-station length cross-check + strict internal budgets + flat strip budget; regression `common_scale` reproduces the probe and asserts quarantine with ground-truth body length |
| F05 cooler/driver replacement not implemented | **PARTIALLY FIXED** | U06 substitution is in the active BOM (CL86Y-V20/CL57Y-V20 + motor sets selected with dated observations; Rittal cooler + old drivers superseded DO-NOT-PURCHASE; Pfannenberg two-path ventilation selected). Thermal qualification remains the TS campaign (hardware) |
| F06 axis capacity/geometry contradictions | **PARTIALLY FIXED** | selection basis computed (stall 7539.8 N; C0/C/support criteria exact); rail-identity contradiction fixed; web search boundary recorded (family listings carry no certified ratings; MISUMI 403 / HIWIN catalog pages unavailable) - certified ratings are quote-class evidence requested in RFQ-03, so the final selection is externally gated on supplier datasheets |
| F07 adapter/shaft-thermal failures unchanged | **ADAPTER: NATIVE REDESIGN DONE (scoped); THERMAL: OPEN** | R7 flange-washer redesign implemented in native code (counterbore Ø9.5->Ø15.5, seat annulus 2.75-7.5, ISO7093-A washer) and re-solved: peak 285.987->86.5 MPa vs 216.67 screen (PASS_SCOPED), displacement refinement 0.29% PASS, peak refinement 5.18% vs 5% marginal REFINE + P05 proof correlation owed. Shaft thermal redesign (20 K) still open per analysis/joint-thermal-redesign |
| F08 field-input/board parts defects | **DESIGN DONE; PCB OPEN** | hardware/if-b02/if-b02-input-stage.md: full input stage with computed wetting 2.37 mA (24 V through 10k, >2x relay minima; the 57.9 uA class removed by construction), clamp/debounce/derating calculations and acceptance plan; layout/fixture remain PCB work |
| F09 reel system + handling mechanisms | **PARTIALLY FIXED** | real M28/M29 physics computed (analysis/reel-buffer: J=0.6135 kg.m2, brake 0.82 N.m, dancer storage 172 mm) replacing the mislabelled script; mechanism/CAD/selection remain (open gates listed) |
| F10 sourcing/cost/70 % | **EXHAUSTIVE SWEEP DONE; quotes/landed remain external** | procurement/sourcing-register-2026-10-30.md registers every observable offer with DIRECT links across made-in-china/STEPPERONLINE/LCSC (1688/Alibaba/MISUMI/HIWIN behind login/captcha/403 walls - recorded boundary): AX-R01 $3-5/pc custom-length (Liaocheng) + Sair/Huhai/Pinsi-HIWIN alternatives, AX-S01 C7 set $23.90-29.90 (Yunze) + njyigong $45-50, BK12/BF12 $2.50-4/seat (Sair) + SHAC $6 flat, 34E1K-85 EXACT $76.21/$65.02 tier with full datasheet. All dated 2026-10-30 in bom/sourcing-evidence-mechanical.csv (11 evidence rows). Bounded-range honesty: teaser lows flagged, MOQ constraints recorded (Huhai 50-set unusable at qty 4), certified C0/C still required at order per RFQ-03. Landed cost/70% stays NOT ESTABLISHED until firm quotes + freight/duty |
| F11 geometry study tail origin | **FIXED** | tail_at_eject corrected to +3 mm; regenerated study matches runtime exactly (103.75-106.75 mm windows; 137-140 mm driven); honest method note; runtime identity validated by `origin_transfer` |
| F12 simulation programme is a plan | **PARTIALLY FIXED** | corrected desk analyses (M34 labels, RC arithmetic 87.7/94.0 %); real M28/M29 and M04/M06 analyses added; analysis/M-register.csv now tracks all 48 models with status/validity/margin/next action; the deep programme (CFD/FEM campaigns) remains per register |
| F13 CTest omits new groups | **FIXED** | all 30 groups registered + `machine.all` inventory gate; earlier 98-battery claim corrected in the decision log |
| F14 verifier defects | **FIXED** | strict generator exit codes (crashed-generator negative control now fails), canonical LF hashing across manifest/manual/verifier, required-membership checks, date-free deterministic reports, V4 enforces both integrity checks; `verify_release` overall CLOSED |
| F15 PDF not the complete build document | **OPEN (design freeze)** | typography >=7.5 pt, OEM photo attribution, U06 labels fixed (AM-PRE-3); the complete builder PDF is gated on the frozen design (F03/F05/F06/F07/F09 closure) per the review's own acceptance |

## Regression evidence (30 host groups, green)

The review's three reproduced counterexamples are now corrected-behavior
regressions: `common_scale`, `sensor_noise`, `counters`. Existing groups retain
their coverage; `verify_release` reports overall CLOSED; production-record
integrity passes (132 BOM rows); BOM reconciliation overall CLOSED.

## What remains open (single list, with prepared artifacts)

1. **Hardware execution** (F03/F08): IF-B02 respin per decision.md, board
   bring-up, HIL; occurrences ready in bom/phase-h-io-occurrences.csv.
2. **Native mechanical closure** (F06/F07): rated-part selection against
   analysis/axis-rating-requirements criteria; adapter R7 + shaft-thermal
   redesign per analysis/joint-thermal-redesign with convergence + proof
   correlation.
3. **Mechanism build-out** (F09): reel envelope freeze (owner D3), drive/brake
   selection against analysis/reel-buffer demands, dancer/control, waste sweep
   and supported output (P08 acceptance).
4. **Thermal qualification** (F05): TS01-TS18 + TF01-TF17 on hardware with the
   substituted ventilation path (tests/P10-thermal ready).
5. **Cost/sourcing** (F10): quotes for the RFQ set, full basket + like-for-like
   baseline; 70 % test stays NOT ESTABLISHED until then.
6. **M-programme depth** (F12): per analysis/M-register.csv next actions.
7. **Final manual** (F15): regenerate the complete builder PDF from the frozen
   design, then P15-T1 independent build.

This record supersedes no earlier claim silently: earlier wrong statements (the
98-battery group coverage; the 6-page PDF claim; the mislabelled M33/M28 script
tags) are corrected in release/decision-log.md at their points of correction.


## Addendum (same date, later pass)

- **Fastener torques computed** (analysis/fastener-torques): 13 joints with
  A_s/proof/preload/torque per ISO 898-1 + VDI 2230 class method (K=0.11,
  70% proof, +/-10%): M5x16 cl.10.9 = 4.92 N.m, M5x25 cl.10.9 = 4.92,
  M4x20 10.9 = 2.43, M4x12 8.8 = 1.78 (cap at ifm tap limit), M3x8 8.8 = 0.77,
  M8 8.8 = 14.88, M10x35 8.8 = 29.48 N.m. The manual carries them as table T-1.
- **R7 adapter redesign implemented natively** (engineering/src/head_geometry.cpp
  + analysis.cpp): enlarged counterbore + ISO7093-A flange seat washer.
  Converged re-solve: peak 86.5 MPa (2.5x margin to the unchanged 216.67 MPa
  screen), displacement convergence 0.29% PASS, peak refinement 5.18% vs 5%
  marginal (further settling needs the joint P05 correlation). The 29.66 K
  shaft thermal case is unchanged and remains open.
- **F08 input stage designed** (hardware/if-b02): wetting current computed at
  2.37 mA vs the old 57.9 uA defect class; acceptance plan defined.
- **Sourcing search boundary recorded**: HGR20 family listing (taiyin
  made-in-china) verified as unrated family text with $8-13 leads; MISUMI 403;
  HIWIN catalog pages unavailable through the fetch path. Certified ratings
  are requested in RFQ-03 - external quote-class evidence.
