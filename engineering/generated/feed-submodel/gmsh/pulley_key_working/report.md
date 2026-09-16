# F101 R6 feed-shaft submodel — pulley_key_and_MB2_slot (gmsh)

Slab window S=61.3 +-5.5 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused gmsh 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). Submodel levels: 0.02 mm sector, 0.0175 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 0 +-30 degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|working (255 deg, signs 1, torque 3) | 0.02 | sector | 66229 | 377929 | 37.09025504 | 39.36351208 | 59.23963372 | 240 |
|working (255 deg, signs 1, torque 3) | 0.0175 | sector | 80115 | 461967 | 37.0820378 | 39.35363023 | 59.12882823 | 240 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.0175 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |
|---|---:|---:|---:|---:|---:|---:|---:|---|
|working | 38.72132026 | 41.05820025 | 0.02215963412 | 0.0251103865 | 0.110805495 | 0.08693340877 | 0.3822337785 | inside |

No citable closed-form Kt exists for the milled key-floor perimeter blend of this region; the submodel convergence is the primary evidence and no handbook factor is asserted.

Interpolation audit: exact hits 11331, boundary projections 972, unsupported 0, maximum projection gap 0.02445503444 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
