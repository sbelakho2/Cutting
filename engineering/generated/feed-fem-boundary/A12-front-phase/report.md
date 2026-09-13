# F101 R6 distributed-support 3D elastic study

Revision **S1-B-FEED-R6-2026-09-12**.

Actual nominal R6 STEP; linear tetrahedra; E205 GPa,nu0.29,density7850 kg/m³. 1 independently generated bulk/root mesh levels. Eight unit solutions are superposed only at the displacement and full stress-tensor level; von Mises magnitudes are never added as though linear.

## Mesh sizing scope

Focused refinement of **A12_groove** in the complete unchanged shaft. All other root guides retain a **0.2 mm** background target. The size is min(bulk,background_root+0.45*all_root_distance,root+0.7*focused_root_distance). No submodel cut boundary,transferred displacement,removed stress region or weakened5% peak criterion is introduced. The bulk size also decreases between levels; convergence of this focused region alone does not establish all-root convergence or background independence. A mandatory scope hold prevents any focused study from claiming full-shaft spatial convergence,even if all reported last-change checks happen to pass.

Selected CAD feature **A12_S19_62**, 1 faces. The focused distance uses the maximum of padded face-bounds distance and untrimmed analytic-surface distance; BSpline junctions use only their padded bounds. These are lower-bound sizing guides to the selected nominal faces,not CAD geometry replacement or stress-error estimates. M15_slot_exit clips the selected upper-pocket face portions to S71.4–73.1. Individual A12-root studies do not independently qualify the other root. All original stress regions remain evaluated; see sizing-faces.csv for the complete selected set.

Volume optimizer **netgen**; netgen means Gmsh's Netgen volume optimization after the original linear Delaunay generation. No individual cell or stress is filtered. The finite elements remain four-node linear tetrahedra,not curved/high-order elements. The finest mesh has 0 tetrahedra whose four nodes share a CAD surface,including 0 exterior centroids. 0 of the40 reported finest raw/phase-bound peak centroids are outside the exact nominal solid. These two exterior-centroid counts have separate zero-count gates; no stress is suppressed to meet them. See same-surface-cells.csv and peak-cad-states.csv. Neither check establishes all-cell or continuous curved-volume containment.

Every tetrahedron has finite positive signed inverse condition quality and volume. All CAD-face nodes project to the underlying original surface within1e-6 mm. Per-face facet-centroid distances and the mesh/CAD volume difference are reported,not represented as a continuous Hausdorff error bound or a stress error estimate. Independent STEP and element-volume checks agree within1e-9 relative. See mesh-quality.csv and surface-audit.csv.

## Loads and supports

Distributed work-conjugate force constraints place radial resultants at S26.5 and77.5. Only the front bearing locates axially. A coupling spin reference removes the sixth rigid mode; its reaction is checked below1e-6 Nm for every balanced transmission basis. The shaft's eccentric mesh mass needs an additional drive holding torque under gravity; that reaction must equal minus the actual body-load moment and is retained in the solution. No torsional bearing clamp or guessed bearing stiffness is introduced. These ideal distributed supports are not ball/race/housing contact.

Belt components±480 N and roller components±60 N inX/Z act at S47.5/S108.75, with actual shaft mesh gravity. All eight load-sign corners and the full rotating material-frame phase are covered. The imposed transmission-torque polygon |Tr|,|Tp|,|Tr+Tp|≤T has six vertices,including opposing interface torques. T=1 Nm working or5 Nm fault; the additional mesh-gravity holding moment is not omitted. Torque and radial loads are distributed over seat patches with preserved force/moment,not resolved key/hub contact pressure.

## Phase and refinement

96 equally spaced phases plus an analytic norm remainder bound per element/node enclose unsampled angles for each finite-element solution. The bound is2*sin(pi/(2N))*hypot(norm(cosine field),norm(sine field)). Torque and radial-load box extrema follow convexity of the stress/displacement norms. This closes angular sampling for the stated linear model,not geometric tolerances or contact states.

Finest working/fault full-shaft upper stress **62.16553404 / 141.150528 MPa**. Working displacement upper envelope **0.01993248349 mm**. Failed/unmet numerical gates: **2**. Every named region retains separate5% refinement gates for its sampled raw peak and continuous-phase upper bound; no percentile replaces a peak. Peak coordinates/signs/torques in regions.csv describe the sampled raw maximum,not necessarily the element producing the upper bound. Each working-N.vtu/fault-N.vtu contains the single sampled load case governing **A12_groove** raw stress at mesh level N,not an incompatible combined envelope. A single/coarse run is diagnostic only.

## Release holds

True helical threads,0.05–0.10 mm final edge breaks,dimensional tolerances,fit prestress,KM2/M5 preload,key/collar contact,elastic bearing/housing response and fatigue remain unqualified. Cosmetic major-thread transitions can contain artificial sharp edges; their peaks remain reported and gated rather than suppressed. This report does not release fabrication or establish physical accuracy/life.
