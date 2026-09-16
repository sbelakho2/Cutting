# F101 R6 feed-shaft submodel — pulley_key_and_MB2_slot (gmsh)

Slab window S=40.2 +-5.5 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused gmsh 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). Submodel levels: 0.02 mm sector, 0.0175 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 26 +-30 degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|fault (210 deg, signs 5, torque 0) | 0.02 | sector | 68572 | 386085 | 72.24230176 | 73.76514832 | 41.10455034 | 210 |
|fault (210 deg, signs 5, torque 0) | 0.0175 | sector | 84782 | 482233 | 74.44735185 | 75.99924868 | 42.7617955 | 210 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.0175 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |
|---|---:|---:|---:|---:|---:|---:|---:|---|
|fault | 75.13061032 | 76.74878522 | 2.961891908 | 2.939634806 | 1.65724516 | 0.0008420489446 | 0.04691561653 | inside |

No citable closed-form Kt exists for the milled key-floor perimeter blend of this region; the submodel convergence is the primary evidence and no handbook factor is asserted.

Interpolation audit: exact hits 14169, boundary projections 948, unsupported 0, maximum projection gap 0.02598477145 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
