# F101 R6 feed-shaft submodel — M15_relief (gmsh)

Slab window S=72.7 +-6 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused gmsh 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). Submodel levels: 0.02 mm sector, 0.01 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 342 +-60 degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction. Root refinement is confined to S72.3-74 mm; other guide features inside the window retain a 0.1 mm background root floor.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|working (142.5 deg, signs 2, torque 0) | 0.02 | sector | 61286 | 266285 | 60.51687988 | 63.71790544 | 72.7149452 | 142.5 |
|working (142.5 deg, signs 2, torque 0) | 0.01 | sector | 140345 | 582484 | 59.29133335 | 62.47015716 | 72.68579419 | 142.5 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.01 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |
|---|---:|---:|---:|---:|---:|---:|---:|---|
|working | 59.91025034 | 63.15142399 | 2.066991008 | 1.997350944 | 0.02915100767 | 0.008418987642 | 0.08446367431 | inside |

Peterson/Pilkey shoulder-fillet cross-check (W.D. Pilkey, Peterson's Stress Concentration Factors, 3rd ed., Wiley 2008, Table 3.3, bending, 2<h/r<=20; Kt=C1+C2(2h/D)+C3(2h/D)^2+C4(2h/D)^3): D=15 mm, d=13.5 mm, r=0.2 mm, h/r=3.75

Case working (phase 142.5 deg): nominal band at S=74.5 (section radius 7.41749007 mm), nominal bending surface stress 11.61602 MPa, band mean von Mises 5.246536985 MPa, torsion surface shear 1.89350983 MPa. Cut-plane reference bands: rear S=66.7 (bending 13.0176602 MPa at r 7.429248013), front S=78.7 (bending 7.498191835 MPa at r 7.375294821). Handbook bending Kt=2.534218581; Kt-based peak 29.43753372 MPa vs submodel sampled peak 59.29133335 MPa (measured Kt 5.104272665, submodel/band-vm ratio 11.30104172). Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: 29.61966458 MPa. The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.

Interpolation audit: exact hits 10635, boundary projections 1074, unsupported 0, maximum projection gap 0.02164739403 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
