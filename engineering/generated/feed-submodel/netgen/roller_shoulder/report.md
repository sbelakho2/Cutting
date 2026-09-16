# F101 R6 feed-shaft submodel — roller_shoulder (netgen)

Slab window S=92.1 +-6 mm cut from the exact nominal shaft B-rep. Global field transferred from the torus-focused netgen 1/0.09 mm finest solution (barycentric element-local interpolation, nearest-hull projection within 0.02 mm on the faceted boundary). Submodel levels: 0.02 mm full-ring, 0.02 mm sector, 0.01 mm sector on the region guide faces (growth 0.45, bulk 0.5 mm), documented node budget 150000 (unchanged cap, solver parity). Sector refinement is confined to the peak azimuth 162.6 +-45 degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction.

| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |
|---|---:|---|---:|---:|---:|---:|---:|---:|
|fault (45 deg, signs 0, torque 2) | 0.02 | full | 101337 | 402405 | 106.0340302 | 108.1399627 | 92.12292234 | 232.5 |
|fault (45 deg, signs 0, torque 2) | 0.02 | sector | 37275 | 151212 | 105.460918 | 107.5532173 | 92.11284927 | 240 |
|fault (45 deg, signs 0, torque 2) | 0.01 | sector | 91231 | 358487 | 104.7384578 | 106.9293003 | 92.1120626 | 45 |

Four-mesh (global) values and submodel last-change (final pair 0.02 mm sector -> 0.01 mm sector):

| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak shift mm | Peak CAD state |
|---|---:|---:|---:|---:|---:|---|
|fault | 98.92562681 | 100.7945756 | 0.6897755281 | 0.5834855901 | 1.863056783 | inside |

Peterson/Pilkey shoulder-fillet cross-check (W.D. Pilkey, Peterson's Stress Concentration Factors, 3rd ed., Wiley 2008, Table 3.3, bending, 2<h/r<=20; Kt=C1+C2(2h/D)+C3(2h/D)^2+C4(2h/D)^3): D=12 mm, d=10 mm, r=0.2 mm, h/r=5

Case fault (phase 45 deg): nominal band at S=93.5 (section radius 4.954443683 mm), nominal bending surface stress 12.53960427 MPa, band mean von Mises 29.93446539 MPa, torsion surface shear 26.66296341 MPa. Cut-plane reference bands: rear S=86.1 (bending 3.35601776 MPa at r 9.397456335), front S=98.1 (bending 11.75202037 MPa at r 4.941591381). Handbook bending Kt=2.585636113; Kt-based peak 32.42285364 MPa vs submodel sampled peak 104.7384578 MPa (measured Kt 8.352612694, submodel/band-vm ratio 3.498925284). Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: 56.42678701 MPa. The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.

Interpolation audit: exact hits 15552, boundary projections 1557, unsupported 0, maximum projection gap 0.0170210215 mm. This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify whole-shaft equilibrium, and no four-mesh gate value is altered or removed.
