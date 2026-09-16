# F101 R6 feed-shaft submodel — roller_key_root (netgen)

Slab window S=99.6 +-6 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused netgen 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). Submodel levels: 0.02 mm sector, 0.0175 mm sector on the two-sided key-floor capsule guide (blend-root surface at the root size, graded inside; growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 204 +-45 degrees; the full ring is budget-infeasible at these roots (see docs/28 section 4).

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|working (315 deg, signs 0, torque 2) | 0.02 | sector | 65381 | 349885 | 42.0069901889 | 43.1455230984 | 101.136829619 | 315 |
|working (315 deg, signs 0, torque 2) | 0.0175 | sector | 82209 | 444496 | 42.9553267993 | 44.0974584336 | 102.997937407 | 315 |
|fault (315 deg, signs 0, torque 2) | 0.02 | sector | 65381 | 349885 | 145.743706772 | 146.882239682 | 101.136829619 | 315 |
|fault (315 deg, signs 0, torque 2) | 0.0175 | sector | 82209 | 444496 | 147.876728835 | 149.018860469 | 102.997937407 | 315 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.0175 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |
|---|---:|---:|---:|---:|---:|---:|---:|---|
|working | 43.14597027 | 44.27794927 | 2.207875682 | 2.158752016 | 1.861107788 | 0.01043178961 | 0.299315421 | inside |
|fault | 153.5742244 | 154.7062034 | 1.442697286 | 1.43415455 | 1.861107788 | 0.01043178961 | 0.299315421 | inside |

Both peaks sit on the roller key-floor side blend (x ≈ -1.42, z ≈ -3.19, radius ≈ 3.49 mm); the S-position moves 1.86 mm along the straight side blend between the levels while the values change by only 2.2%/1.4% - a documented shallow plateau along S (the four-mesh series showed the same behaviour, S99.45-100.05). The azimuth profile (azimuth-profile.csv) shows the plateau is confined to the refined sector.

No citable closed-form Kt exists for the milled key-floor perimeter blend of this region; the submodel convergence is the primary evidence and no handbook factor is asserted. Measured concentration against the roller-seat band von Mises is reported in docs/28.

Interpolation audit: exact hits 19746, boundary projections 1416, unsupported 0, maximum projection gap 0.0330226 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
