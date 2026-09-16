# Whole-shaft continuous CAD-containment audit: result

2026-09-16. Execution of the residual `continuous_CAD_containment_verified`
audit of the final whole-shaft torus four-mesh series
(`engineering/generated/feed-fem-torus/gmsh`, Netgen volume optimizer,
129936 nodes / 565476 tetrahedra at the finest level). Generated artifacts:
[per-level counts](../generated/feed-fem-torus/gmsh/containment-audit.csv),
[audit JSON](../generated/feed-fem-torus/gmsh/containment-audit.json),
[violating centroids](../generated/feed-fem-torus/gmsh/containment-violations.csv).

## Question

Does every tetrahedron centroid of every level of the final series lie
strictly inside the exact nominal CAD solid? The earlier gates checked only
same-surface tetrahedra (0) and reported peak centroids (0); the residual
flag required a complete all-cell audit. Criterion: 0 outside and 0
on-boundary-within-tolerance over all four levels, at the fixed
1×10⁻⁷ mm classifier tolerance, by exact BRep point-in-solid
classification — never a tolerance-scaled approximation.

## Tool added

- `engineering/include/containment_audit.hpp` +
  `engineering/src/containment_audit.cpp` (`starz_containment_audit`):
  fail-closed driver that reuses the existing VTU reader and the exact
  BRep classification machinery. It requires the four working/fault VTU
  pairs to carry the one shared mesh (exact node/tet equality per level),
  requires every level to match `mesh-quality.csv` exactly, validates the
  solid against the recorded CAD volume (relative 1×10⁻⁹), classifies all
  element centroids with `BRepClass3d_SolidClassifier` at 1×10⁻⁷ mm, and
  only flips `continuous_CAD_containment_verified:true` in status.json on
  zero violations (single field, everything else byte-unchanged). On
  violations it leaves status.json untouched and writes the exact
  centroids, states, nearest trimmed faces, gaps and projections.
- `engineering/tests/test_containment_audit.cpp`: synthetic box-series
  regression (inside/on/outside classification, flag flip, idempotent
  re-audit) plus 12 fail-closed rejections (missing VTU, working/fault
  geometry mismatch, count mismatch, non-refining levels, missing/incomplete
  status, absent/duplicated/malformed flag, wrong optimizer identity, wrong
  CAD volume, empty VTU). Registered as CTest
  `feed_containment_audit_benchmarks` (passes).

## Run

All four levels, working and fault VTUs verified byte-identical per level,
counts matching `mesh-quality.csv` (22142/88073, 38453/158261, 73575/313911,
129936/565476), solid volume 14390.416746433 mm³ vs the recorded
14390.4167475 mm³ (relative difference ≈7.4×10⁻¹¹). 1,125,721 centroid
classifications:

| Level | bulk/root mm | Tets | Inside | Outside | On-boundary |
|---:|---:|---:|---:|---:|---:|
| 1 | 2/0.30 | 88073 | 88073 | **0** | 0 |
| 2 | 1.6/0.20 | 158261 | 158237 | **24** | 0 |
| 3 | 1.2/0.13 | 313911 | 313908 | **3** | 0 |
| 4 | 1/0.09 | 565476 | 565471 | **5** | 0 |
| total | — | 1125721 | 1125689 | **32** | **0** |

## Result

**32 exterior centroids, 0 on-boundary centroids.** The check does not
pass; `continuous_CAD_containment_verified` **remains false** and no gate,
load, phase, mesh, cap or raw value was changed. No tolerance was weakened.

Every violating centroid is a linear-facet sliver whose flat boundary
triangle spans a concave curved patch (nodes on adjacent surfaces of a
blend), so its centroid falls micrometres outside the solid. None is a
same-surface tetrahedron — that is why the closed same-surface gate (0)
did not see them. Governing trimmed faces, all concave fillet tori
(per `sizing-faces.csv`):

| Face | Feature (torus origin mm, axis, R/r mm) | Level 2 | Level 3 | Level 4 |
|---|---:|---:|---:|---:|
| 68 | rear bearing shoulder R0.20 blend (0,31.3,0), +S, 6.2/0.2 | 7 | 0 | 0 |
| 7 | roller key-floor rear blend (0,95.97,−3.35), +Z, 1.37/0.15 | 5 | 0 | 0 |
| 9 | roller key-floor front blend (0,111.03,−3.35), +Z, 1.37/0.15 | 2 | 0 | 0 |
| 61 | pulley key-floor front blend (0,59.03,4.15), −Z, 2.37/0.15 | 5 | 0 | 0 |
| 63 | pulley key-floor rear blend (0,35.97,4.15), −Z, 2.37/0.15 | 3 | 0 | 0 |
| 71 | MB2 washer-entry floor blend (−1.6,31.7,5.65), −Z, 0.35/0.15 | 1 | 1 | 3 |
| 72 | MB2 washer-entry floor blend (+1.6,31.7,5.65), −Z, 0.35/0.15 | 1 | 2 | 2 |

Gaps (nearest trimmed-face distance) 7.139×10⁻⁶ … 5.509×10⁻³ mm, all
unambiguously above the 1×10⁻⁷ mm classification tolerance. The largest is
level 2 element 64820 on face 68 (0.0055089 mm); the smallest are level 4
elements 542842/542841 on faces 71/72 (7.1/12.7 µm). Counts are non-monotone
across levels (24→3→5) for the same reason as the earlier sweep: which
coarsest blend contributes a sliver is a local-sizing lottery, and the
level-2 blend tori are the coarsest.

## Interpretation

- Established: exact per-level counts, states, faces, gaps and projections
  for every exterior centroid of the final series; all are concave-fillet
  facet slivers, not CAD geometry defects, and their 0.0071–5.51 µm gaps
  are far below any machining tolerance but above the strict 1×10⁻⁷ mm
  containment criterion.
- Not established: continuous containment (`continuous_CAD_containment_verified`
  stays false), and no stress, gate or release conclusion changes. This is a
  mesh-quality audit result, not a fabrication blocker; the flag remains a
  recorded hold, and the ten regional raw-peak holds plus all physical
  qualifications are unchanged.

## Verification

`feed_containment_audit_benchmarks` passes (classification, flag flip,
idempotence, 12 fail-closed rejections). The audit refuses to modify
status.json on violations and the production run left
`engineering/generated/feed-fem-torus/gmsh/status.json` byte-identical to
its pre-audit content apart from the three additive audit artifacts.
