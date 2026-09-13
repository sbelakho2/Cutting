# 26 — Feed-shaft torus-focused root rerun

2026-09-13. Numerical follow-up to the [exterior-centroid diagnosis](19-exterior-centroid-diagnosis.md). **Physical authority remains head R6 / feed R6 / measurement R1. No physical CAD, load, region, acceptance limit, node cap or control state changed. No fabrication or energisation release.**

## Strategy

The diagnosis established that every exterior centroid of the optimized front mesh sits on a concave fillet torus (faces 68, 71/72, 63, 61, 7). This rerun applies that insight to the stress model: a new whole-shaft CAD feature `concave_fillet_tori` selects those six diagnosed tori plus both A12 ring-groove roots (nine faces) by torus geometry, and the unchanged four-mesh series 2/.30, 1.6/.20, 1.2/.13, 1/.09 mm is rerun with the sizing `min(bulk, root+0.45*all_root_distance, root+0.45*torus_guide_distance)` — the analytic trimmed-torus distance replaces the approximate ring/capsule offsets for exactly those tori. Both optimizers are run. Loads, eight sign corners, six torque vertices, eccentric mesh gravity, 48 phases, all ten stress regions, all 5% raw/bound last-change gates, the 0.5% volume gate and the 150000-node direct-solver cap are unchanged. The no-feature path is unchanged in code and reproduces the published all-root Netgen series byte-identically (criteria.csv and regions.csv).

## Results

| Series | Optimizer | Finest nodes / tets | Working / fault bound MPa | Displacement mm | Failed gates | vs published holds |
|---|---:|---:|---:|---:|---:|---|
| Four-mesh rerun | gmsh | 132174 / 604251 | 77.6468 / 161.817 | 0.0204893 | 13 | 16 → 12 regional + 1 geometry |
| Four-mesh rerun | Netgen | 129936 / 565476 | 74.7377 / 165.049 | 0.0206120 | 10 | 12 → 10 |

Full artifacts: [gmsh](../engineering/generated/feed-fem-torus/gmsh/report.md), [Netgen](../engineering/generated/feed-fem-torus/netgen/report.md).

**Closed holds.** The gmsh rerun closes six of the original 16 four-mesh holds (working and fault A12 groove, working neck shoulder; previously 12.1%, 11.3%, 13.3%). The Netgen rerun closes two of the 12 all-root-Netgen holds (working roller shoulder, 8.2% → 4.82%/4.97%). Both runs are deterministic (byte-identical single-level rerun).

**Opened holds.** The gmsh rerun opens two new fault pulley-key/MB2-slot holds (1.9% → 9.6%) and fails the same-surface exterior-centroid gate with 34 cells (a gate that postdates the original gmsh run); the Netgen rerun opens none and passes both exterior-centroid gates with zero cells.

**Remaining holds.** Both reruns retain: M15 relief working and fault, roller key-root working and fault; the gmsh run also retains roller-shoulder working and pulley-key working, the Netgen run roller-shoulder fault. Peak locations are stable across levels and inside the exact solid: M15 peaks sit at the Ø13.5 runout (radius ≈ 6.73–6.75 mm, S72.6–72.8), roller-shoulder peaks at the R0.20 blend root on the Ø10 seat (radius ≈ 5.0 mm, S92.1), roller key-root peaks on the R0.15 key-floor perimeter blend (radius ≈ 1.45–1.51 mm, z ≈ −3.2). The fault roller key-root peak is still climbing (126.0 → 127.0 → 140.5 → 153.6 MPa over the series). These are genuine under-resolved stress concentrations, not mesh noise: 0.09 mm elements place only two to three elements across an R0.15–0.20 mm blend radius, and the last-change rates stay at 5.5–8.9%.

## Why the strategy did not close them

The torus guides change the all-root meshes only marginally (+0.15% nodes): for the eight axis-Y shoulder/groove tori the existing ring distance is already the exact analytic torus distance, and the slot capsule guides already size the key-floor tori to root. The new refinement is confined to the six non-axis-Y washer/floor tori; the closed holds result from peak-cell re-topologization, not increased resolution. Finer resolution of the remaining R0.15–0.20 blends inside the unchanged 150000-node cap was already shown infeasible (0.02/0.009375 mm torus roots over budget, [diagnosis](19-exterior-centroid-diagnosis.md)). Growth-0.70 variants of the torus guide (the probe growth) are worse — gmsh 29 and Netgen 14 failed gates, including two exterior reported peak centroids in the gmsh run — and are retained as records ([gmsh](../engineering/generated/feed-fem-torus/gmsh-growth-0.70/status.json), [Netgen](../engineering/generated/feed-fem-torus/netgen-growth-0.70/status.json); VTUs of these two inferior variants were not retained).

## Verification and release state

Engineering suite: all 28 feed/FEM/distributed/refined/boundary tests pass; the full 81-test suite shows eight mechanism failures from the concurrent Mechanism-R3 workstream (`PO_P001` disconnection, revision mismatch), unrelated to this increment. `tools/check_production_records.py` passes; `tools/check_mechanism_outputs.py` fails only on that concurrent revision mismatch. All stress holds remain fail-closed: `numerical_gates_passed:false`, `fabrication_released:false` in every new status.json. No acceptance criterion was relaxed and no failed raw data was deleted; the historical 16-hold and 12-hold records remain in their own directories. **Not a fabrication, energisation or procurement release.**
