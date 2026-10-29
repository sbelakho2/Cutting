# Cutting: task for the next engineering agent

Read `ENGINEERING_HANDOFF.md` first. This package contains instructions, specifications, inventories, photos and fresh test evidence. **No PDF is being supplied by the handoff agent.** The completion agent must deliver exactly one canonical, complete visual `Cutting_Assembly_Manual.pdf`, generated from the final selected native design. That final PDF cannot be called complete until every purchased and made part is found, costed, specified and represented, and all design contradictions are resolved.

Repository: https://github.com/sbelakho2/Cutting

Freshly checked upstream main on 29 October 2026: `0f28aa3289cf052409ad48f54dc5d23b130fd441`.
Only `main` was listed. Inspect a fresh HEAD before editing; preserve later user changes if it moves.

The earlier local R4/R7 development checkout and unsaved source/evidence were removed by workspace maintenance. They were **not** merged or pushed. This kit does not contain those lost Gerbers, UF2s, CAD, patch or raw FEM fields. Never claim that cloning main restores them. Reconstruct the detailed candidates in this handoff, or improve them, then regenerate and independently verify everything. Reported earlier numerical successes are leads, not present release evidence.

Recoverable context: the three saved 13 October audit artifacts. This kit includes their extracted historical tables, with dates and limits. The earlier audit covers commit40b8b2d...; current main is the subsequent fix wave0f28aa3. Preserve the source fixes that are actually present. Do not present the older audit's already-fixed findings as current without checking.

Fresh evidence in this kit: upstream source inventory and CSV shape review;21 host test groups pass with ASan/UBSan; a new executable stock-side encoder counterexample; independent feed/stall/thermal/regeneration arithmetic. No physical machine test or fresh native CAD/FEM solve was performed for this handoff. The evidence JSON files state scope.

User priorities: every original goal; robust and easy assembly; lowest adequate complete landed cost; target about70% reduction without reducing quality or loads; China/Alibaba/Made-in-China/LCSC as well as western sources; replace the EUR1669 cooler with appropriately rated drivers and adequate filtered ventilation; handle small and large reels; one self-contained visual assembly PDF. Do not return another mostly-'BLOCKED' audit as the finished engineering task. Implement corrections and produce the actual purchasing/fabrication/assembly/test package.

Start with the sensor/material-state contradiction and missing real handling. It invalidates passing software simulations and influences I/O, PCB and metrology placement. Then finish the real cut axis, feed thermal/metrology budget, powered payoff/buffer, protection and cabinet design. The order and all acceptance gates are in the handoff.

No purchases, supplier messages, PR merge or deployment have been authorised. Draft exact RFQs if needed; do not send them without explicit user instruction. Reversible local development and reviewable branches are within scope. No requirement may be silently weakened. Do not import constraints from unrelated projects.

The kit contains no complete duplicate repository. Clone the pinned source and use the tool scripts below. Native generated parts/drawings and firmware must be regenerated from the final revision and hashed. Persist actual progress in git and retain reviewable checkpoints so the source is not lost again.

The detailed specification covers all47 original goals,48 linked model work packages,P00-P15 physical test families,18 planned cabinet steady-state records and17 planned thermal/permission faults. The data directory includes goal/model mappings and exact part/evidence record templates. The three photographs and recovered historical audit texts are included. Read data/history status notes before using old prices or status fields.

For package validation (this validates the handoff, not the machine):

```bash
python3 tools/validate_handoff.py
```

For the actual pinned host tests and deliberate sensor counterexample:

```bash
bash tools/run_host_checks.sh /absolute/path/to/Cutting /absolute/path/to/new-evidence
```

Start the completion agent with section20's copy-ready instruction and supply this entire kit. Do not give it only a status summary or mistake the numerical work instructions for executed simulations. Its one final visual assembly PDF is a later deliverable, governed by section17.
