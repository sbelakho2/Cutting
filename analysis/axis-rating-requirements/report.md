# M04/M06 axis rating requirements (review F06)

Deterministic output of `analysis/axis-rating-requirements/compute.py`.

## Unchanged acceptance loads

- working: axial 2500 N, transverse 750 N, vertical 375 N
- proof: 5000 N (double) - never relaxed

## Stall thrust

- ideal (eta=1): 5340.7 N (the stored 5340.708 N bound)
- declared selection basis (torque x1.2, eta=0.85): 7539.8 N

## Minimum catalogue ratings (selection criteria)

- **linear_guide_block_set**: static_load_C0_N_min=7500.0, dynamic_load_C_N_min=15079.644737231005
  per axis, 2 sets; HGR20-class candidates must show BOTH C0 and C against these numbers - the old generic '4900 N class' figure is below the 5000 N proof and may never be reused
- **ball_screw_nut**: dynamic_rating_N_min=11309.733552923253, static_rating_N_min=7500.0
  SFU1610-class C7 10 mm lead; end machining 8 h7x18; the published nut rating must be checked at the declared preload class
- **fixed_support_BK_class**: axial_rating_N_min=9424.777960769377
  the support carrying the fault-thrust direction; its axial rating must clear BOTH the 5000 N proof and the stall bound with margin - the 4900 N contradiction is resolved ONLY by a rated part
- **floating_support_BF_class**: axial_rating_N_min=5000.0
  floating side; radial capacity per guide reactions
- **coupling**: rated_torque_nm_min=10.2
  reversal duty (1 N.m x 2.5 service factor for feed; head axes see the full stall bound); 16 mm fit length on the exposed shaft
- **motor_adapter_joint**: 
  the R6 adapter FEM still FAILS its screen (285.987 MPa vs 216.667 MPa; refinement 18.46%/4.96% vs 5%/3% gates) - no rating number releases it; the joint redesign + converged re-solve is the gate (review F07)

## Register contradictions to eliminate

- AX-01 interface text says 'rail 440 total' while AX-R01 is a 400 mm rail: the register must state ONE rail geometry per axis (KK8610 lineage is 340 mm rail / 440 overall; the custom HGR20 lineage is 400 mm rail / 440 base) - mixing the two lineages in one row is a source-quality defect
- support rating class 4900 N vs 5000 N proof: never rounded, never reused
- 340/400 mm mismatch across engineering/README (KK8610 purchased-axis line) and bom/selected-cutting-head.csv (custom axis): the cost goal selects the custom axis; the purchased-axis line remains the comparison baseline, not the build

## Open gates

- select exact rated parts against these criteria and re-verify with catalogue documents (G38/G39); catalogue family text is not a quote
- complete native axis CAD with the selected parts and manufacturing drawings
- re-run all load signs/proof/fault cases and the loaded-gap/stiffness chain on the selected assembly (M04/M06/M08)
