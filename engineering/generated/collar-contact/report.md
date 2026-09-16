# UMCL collar / key contact study (resolved-pressure linear FEM)

Desk closure of the desk part of the **housing/collar contact** gate (docs/24 §3 mechanical gate; docs/12 "remaining local-stress hold"). The physical part (incoming bore/keyway measurement, clamp torque at assembly and the 5 Nm belt proof) remains open and is defined in docs/12.

## Assumptions (stated, justified)

- Geometry: Ø10 shaft section 60 mm long (axis Z, fixed end z=60), collar station z=5..25 mm, DIN 6885 t1=1.8 mm seat, nominal line-to-line 3 mm keyway width, key bonded to the seat (conforming mesh; tight-fit idealisation), seat floor perimeter R0.15, key long edges R0.1 (stand-in for the real 0.2×45° shaft-facing chamfer / deburr).
- Collar idealised as **rigid**: clamp as ±x̂ traction on two 90° arcs of the Ø10 surface across the 20 mm collar width; torque as uniform +x̂ traction on the protruding key side face, reacted by -x̂ traction on the rear 120° bore arc (rigid-bore bearing idealisation).
- Clamp force: M4 8.8 at 2.8 Nm, K=0.2 (dry zinc-plated) → T=K·F·d gives **3.5 kN per screw**; M4 stress area 8.78 mm² × 580 MPa proof ≈ 5.1 kN proof load, so 3.5 kN is 69% of proof. Two screws → **7 kN total clamp**. (The prompt's 2 kN would need K=0.35, i.e. lubricated/worn threads; the clamp case is solved separately and shown to contribute negligibly to the key stresses.) The actual UMCL screw size must be confirmed at receipt; the clamp load does not drive the verdict.
- Torque 5 Nm (fault/belt-proof) → key tangential force **1000 N** at the shaft radius r=5 mm (conservative: the face centroid sits at ≈5.6 mm).
- Material: E=205 GPa, ν=0.29 for both shaft (42CrMo4+QT) and key (C45) — single-material model.
- Allowables: C45 key bearing 120 MPa practice, shear 180 MPa (task); shaft 42CrMo4+QT yield 650 MPa; **collar wall is EN AW-5052 (65 MPa) — its bearing limit 65/1.5 = 43.3 MPa governs the contact face** (existing feed-analysis basis).

## Analytic (resolved-pressure) design values

| Key | Protrusion above r=5 | Contact face | Contact pressure | Key shear (b·L) | Seat bearing (t1·L) |
|---|---|---|---|---|---|
| 3×3×20 (DIN 6885) | 1.2 mm | 24 mm² | **41.67 MPa** | 16.67 MPa | 27.78 MPa |
| 3×4×18 (F109) | 2.2 mm | 39.6 mm² | **25.25 MPa** | 18.52 MPa | 30.86 MPa |

Against 120 MPa key bearing / 180 MPa shear: margins ≥2.9× (bearing, 3×3×20) and ≥9.7× (shear). Against the **aluminium** collar-wall bearing limit 43.3 MPa: the 3×3×20 variant leaves only **1.04×** (41.7 MPa), while the repository F109 3×4×18 variant passes at **1.71×** (25.3 MPa). The F109 identity must therefore not be substituted by a shorter DIN 3×3×20 key; both variants pass the C45 key allowables, the collar wall is the governing soft side.

## FEM results (linear, ≤150000 nodes)

| Variant | Level | bulk/min mm | Nodes | Tetras | Face pressure MPa (FEM) | Key peak vm MPa | Key peak shear MPa | Near-plane shear MPa | Face peak |sxx| MPa | Seat-wall peak vm MPa | Shaft peak vm MPa | Clamp-only key vm MPa | Disp um |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| DIN6885 3x3x20 (task statement) | 2 levels | 34968 | 180526 | 44.6 | 152 | 69.7 | 69.7 | 56.5 | 77.4 | 103 | 20 | 19.2 |
|  | level 1 | 0.6/0.2 | 21176 | 105722 | 44.3 | 144 | 63 | 63 | 49.6 | 73.4 | 95.3 | 19.8 | 19.4 |
|  | level 2 | 0.5/0.15 | 34968 | 180526 | 44.6 | 152 | 69.7 | 69.7 | 56.5 | 77.4 | 103 | 20 | 19.2 |
| F109 3x4x18 (repository design) | 1 levels | 35406 | 182565 | 26.2 | 164 | 70.8 | 70.8 | 33.3 | 105 | 108 | 22.4 | 25.2 |
|  | level 1 | 0.5/0.15 | 35406 | 182565 | 26.2 | 164 | 70.8 | 70.8 | 33.3 | 105 | 108 | 22.4 | 25.2 |
The FEM face pressure is the applied traction resolved on the actual meshed face (the R0.1 top-edge blend slightly reduces the face versus the ideal 1.2/2.2 mm height). The key peak von Mises hugs the loaded face's free top edge (an ideal 90° corner in the real key too); its last-change between the two mesh levels is reported without relaxing any acceptance criterion. The clamp-only column shows the 7 kN clamp adds ≈20 MPa peak in the key (≈13% of the combined peak, radial compression) and does not alter the verdict. No node count exceeds the 150000 cap.

## Verdict

**PASS — desk part closed** for the as-designed **F109 3×4×18** key: face-average contact pressure 26.2 MPa FEM (25.3 MPa ideal) against the C45 bearing practice 120 MPa and the aluminium collar-wall bearing limit 43.3 MPa (65/1.5), key shear 18.5 MPa against 180 MPa, seat bearing 30.9 MPa against 120 MPa, peak key vm 164 MPa against the F109 C45+N minimum yield 300 MPa. The task-stated DIN 6885 3×3×20 key also passes the C45 allowables (41.7 MPa bearing, 16.7 MPa shear) but erodes the aluminium collar-wall bearing margin to ≈1.0×, so it is **not** an endorsed substitution for F109. Remaining physical steps (unchanged, docs/12): incoming bore 10.000–10.030 mm and keyway 3.04±0.01/2.35±0.025 measurement on F110, clamp-screw identity/torque record at assembly, and the belt proof test (5 Nm, 10 s hold, 10 cycles, slip-marked) on the assembled cartridge. Nothing here releases fabrication.
