# F101 R6 feed-shaft submodel — M15_relief (netgen)

Slab window S=72.7 +-6 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused netgen 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). Submodel levels: 0.02 mm sector, 0.01 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 0 +-60 degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction. Root refinement is confined to S72.3-74 mm; other guide features inside the window retain a 0.1 mm background root floor.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|working (37.5 deg, signs 1, torque 0) | 0.02 | sector | 60645 | 251363 | 60.46328318 | 63.64332149 | 72.71503812 | 37.5 |
|fault (142.5 deg, signs 2, torque 0) | 0.02 | sector | 60645 | 251363 | 110.5280442 | 113.4183934 | 72.8122645 | 142.5 |
|working (37.5 deg, signs 1, torque 0) | 0.01 | sector | 139132 | 552041 | 60.41493142 | 63.47922625 | 72.80945247 | 37.5 |
|fault (142.5 deg, signs 2, torque 0) | 0.01 | sector | 139132 | 552041 | 115.1295727 | 118.1667285 | 72.8179398 | 142.5 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.01 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |
|---|---:|---:|---:|---:|---:|---:|---:|---|
|working | 59.42475109 | 62.62133978 | 0.08003278859 | 0.2585022671 | 0.09441434779 | 0.01594766199 | 0.5247390123 | inside |
|fault | 105.9670093 | 108.9623131 | 3.996825894 | 4.018335093 | 0.005675301717 | 0.002094364878 | 0.08028425713 | inside |

Peterson/Pilkey shoulder-fillet cross-check (W.D. Pilkey, Peterson's Stress Concentration Factors, 3rd ed., Wiley 2008, Table 3.3, bending, 2<h/r<=20; Kt=C1+C2(2h/D)+C3(2h/D)^2+C4(2h/D)^3): D=15 mm, d=13.5 mm, r=0.2 mm, h/r=3.75

Case working (phase 37.5 deg): nominal band at S=74.5 (section radius 7.423558221 mm), nominal bending surface stress 11.62752786 MPa, band mean von Mises 5.231620714 MPa, torsion surface shear 1.833779408 MPa. Cut-plane reference bands: rear S=66.7 (bending 13.00580785 MPa at r 7.42515), front S=78.7 (bending 7.534316079 MPa at r 7.390830447). Handbook bending Kt=2.534218581; Kt-based peak 29.46669714 MPa vs submodel sampled peak 60.41493142 MPa (measured Kt 5.19585351, submodel/band-vm ratio 11.54803353). Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: 29.63738317 MPa. The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.
Case fault (phase 142.5 deg): nominal band at S=74.5 (section radius 7.423558221 mm), nominal bending surface stress 11.66186199 MPa, band mean von Mises 10.39060699 MPa, torsion surface shear 6.925282994 MPa. Cut-plane reference bands: rear S=66.7 (bending 13.01931521 MPa at r 7.42515), front S=78.7 (bending 7.538944915 MPa at r 7.390830447). Handbook bending Kt=2.534218581; Kt-based peak 29.55370734 MPa vs submodel sampled peak 115.1295727 MPa (measured Kt 9.872314803, submodel/band-vm ratio 11.08015854). Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: 31.89514463 MPa. The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.

Interpolation audit: exact hits 21300, boundary projections 2148, unsupported 0, maximum projection gap 0.02164739403 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
