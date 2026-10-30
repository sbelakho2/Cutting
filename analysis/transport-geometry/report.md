# Transport-geometry envelope proof (M27/M30/M28)

Interval arithmetic over 5184 sampled envelope cells (9 lengths x 24 lead x 24 tail settings) evaluated from the implemented phase states - not end-to-end runs, not a contact/control model (see manifest method_note). Deterministic output of `analysis/transport-geometry/compute.py`. The lead setting is swept for coverage but does not enter the ejection equations: it changes the feed start position, which cancels in the identity.

## Invariants

- PASS — I1_tail_moves_fully_observable
- PASS — I2_eject_always_partially_unobserved
- PASS — I3_grip_held_through_tail_moves
- PASS — I5_piece_spans_D_at_sever

Violations: 0


## Derived design requirements

- Eject observation window at D: 103.75-106.75 mm of the 230 mm move; unobserved remainder 123.25-126.25 mm in every cell -> discharge evidence mandatory: True
- Gripped eject distance: 137.0-140.0 mm; the takeaway must at minimum slide a further 130 mm to clear the discharge zone end
- Waste chute minimum body capacity: 140 mm (bodies below the grip boundary are never propel-lable: the waste path must accept severed bodies up to just under 140 mm (interrupted-stock tips), not only 10 mm stubs and 3-25 mm slugs)
- Minimum grip margin at the far nip during tail score: 35.0 mm

## M28 tooth-pass derivation

- Pulley 20 teeth x 5.0 mm pitch, 1:1 on the roller shaft; shaft 3.97887 rps at 0.50 m/s
- Tooth-pass frequency: 79.5775 Hz; belt speed 397.9 mm/s (below wire speed because the pulley pitch circumference 100 mm < roller circumference 125.7 mm)
- matches the handoff's corrected 79.5775 Hz nominal: the pulley runs 1:1 on the roller shaft, so tooth pass = teeth x wire speed / (pi x roller D), NOT teeth x wire speed / belt pitch circumference (belt speed is 397.9 mm/s, below wire speed)
