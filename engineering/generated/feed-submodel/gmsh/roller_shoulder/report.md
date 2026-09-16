# F101 R6 feed-shaft submodel — roller_shoulder (gmsh)

Slab window S=92.1 +-6 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused gmsh 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.02 mm on the faceted boundary). Submodel levels: 0.02 mm full-ring, 0.02 mm sector, 0.01 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 0 +-45 degrees with a 15-degree peak-margin guard; the full-ring level cross-checks the sector restriction.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|working (135 deg, signs 2, torque 2) | 0.02 | full | 102080 | 423725 | 46.13721072 | 48.55431744 | 92.12113837 | 112.5 |
|working (135 deg, signs 2, torque 2) | 0.02 | sector | 33392 | 141859 | 45.35032993 | 47.93846826 | 92.11745972 | 157.5 |
|working (135 deg, signs 2, torque 2) | 0.01 | sector | 87820 | 359525 | 45.16357966 | 47.85056181 | 92.11808099 | 300 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.01 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak shift mm | Peak CAD state |
|---|---:|---:|---:|---:|---:|---|
|working | 39.30888854 | 41.63428977 | 0.4134974923 | 0.1837103791 | 3.964254845 | inside |

Peterson/Pilkey shoulder-fillet cross-check (W.D. Pilkey, Peterson's Stress Concentration Factors, 3rd ed., Wiley 2008, Table 3.3, bending, 2<h/r<=20; Kt=C1+C2(2h/D)+C3(2h/D)^2+C4(2h/D)^3): D=12 mm, d=10 mm, r=0.2 mm, h/r=5

Case working (phase 135 deg): nominal band at S=93.5 (section radius 4.939906268 mm), nominal bending surface stress 12.36925608 MPa, band mean von Mises 8.177554624 MPa, torsion surface shear 5.745528551 MPa. Cut-plane reference bands: rear S=86.1 (bending 3.371299212 MPa at r 9.383997344), front S=98.1 (bending 11.54829096 MPa at r 4.902738351). Handbook bending Kt=2.585636113; Kt-based peak 31.98239522 MPa vs submodel sampled peak 45.16357966 MPa (measured Kt 3.651276952, submodel/band-vm ratio 5.522871045). Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: 33.4948787 MPa. The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.

Interpolation audit: exact hits 15552, boundary projections 1557, unsupported 0, maximum projection gap 0.0170210215 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
