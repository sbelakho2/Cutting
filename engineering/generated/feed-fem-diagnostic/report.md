# F101 R6 distributed-support 3D elastic study

Revision **S1-B-FEED-R6-2026-09-12**.

Actual nominal R6 STEP; linear tetrahedra; E205 GPa,nu0.29,density7850 kg/m³. 1 independently generated bulk/root mesh levels. Eight unit solutions are superposed only at the displacement and full stress-tensor level; von Mises magnitudes are never added as though linear.

## Loads and supports

Distributed work-conjugate force constraints place radial resultants at S26.5 and77.5. Only the front bearing locates axially. A coupling spin reference removes the sixth rigid mode; its reaction is checked below1e-6 Nm for every balanced transmission basis. The shaft's eccentric mesh mass needs an additional drive holding torque under gravity; that reaction must equal minus the actual body-load moment and is retained in the solution. No torsional bearing clamp or guessed bearing stiffness is introduced. These ideal distributed supports are not ball/race/housing contact.

Belt components±480 N and roller components±60 N inX/Z act at S47.5/S108.75, with actual shaft mesh gravity. All eight load-sign corners and the full rotating material-frame phase are covered. The imposed transmission-torque polygon |Tr|,|Tp|,|Tr+Tp|≤T has six vertices,including opposing interface torques. T=1 Nm working or5 Nm fault; the additional mesh-gravity holding moment is not omitted. Torque and radial loads are distributed over seat patches with preserved force/moment,not resolved key/hub contact pressure.

## Phase and refinement

16 equally spaced phases plus an analytic norm remainder bound per element/node enclose unsampled angles for each finite-element solution. The bound is2*sin(pi/(2N))*hypot(norm(cosine field),norm(sine field)). Torque and radial-load box extrema follow convexity of the stress/displacement norms. This closes angular sampling for the stated linear model,not geometric tolerances or contact states.

Finest working/fault full-shaft upper stress **59.5544465 / 124.1977913 MPa**. Working displacement upper envelope **0.0223046806 mm**. Failed/unmet numerical gates: **2**. Every named region retains its raw peak and5% refinement gate; no percentile replaces a peak. A single/coarse run is diagnostic only.

## Release holds

True helical threads,0.05–0.10 mm final edge breaks,dimensional tolerances,fit prestress,KM2/M5 preload,key/collar contact,elastic bearing/housing response and fatigue remain unqualified. Cosmetic major-thread transitions can contain artificial sharp edges; their peaks remain reported and gated rather than suppressed. This report does not release fabrication or establish physical accuracy/life.
