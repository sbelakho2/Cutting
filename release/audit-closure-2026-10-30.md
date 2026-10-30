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
| F06 axis capacity/geometry contradictions | **PARTIALLY FIXED** | analysis/axis-rating-requirements computes the real selection basis (stall 7539.8 N declared bound; C0/C criteria); rail-identity contradiction fixed in the register; the rated-part selection + native CAD re-solve remain (criteria are now exact) |
| F07 adapter/shaft-thermal failures unchanged | **OPEN (native re-solve + hardware)** | analysis/joint-thermal-redesign/requirements.md fixes the redesign work order and recomputed demands (7539.8 N joint load; unchanged 216.67 MPa/20 K screens); redesign + convergence + correlation remain |
| F08 field-input/board parts defects | **OPEN (hardware)** | folded into the IF-B02 respin work order (decision.md item 1: bias/wetting network, J11/P11/Keystone identities); firmware cannot fix wetting current |
| F09 reel system + handling mechanisms | **PARTIALLY FIXED** | real M28/M29 physics computed (analysis/reel-buffer: J=0.6135 kg.m2, brake 0.82 N.m, dancer storage 172 mm) replacing the mislabelled script; mechanism/CAD/selection remain (open gates listed) |
| F10 sourcing/cost/70 % | **PARTIALLY FIXED** | ledger extends by quantity (DR-01 = 261.54 at qty 2); V20/ventilation observations registered with dates; full basket/baseline/landed cost remains quote-dependent (RFQs drafted) |
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
