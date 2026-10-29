# M36 feed timing / M31 length error budget (desk closure)

Deterministic output of `analysis/m36_m31/compute.py`; values cite the single
active source `shared/controls_spec.hpp` (legacy 3200/40000 constants quarantined).

## M36 pulse/timing envelope

- Roller circumference (nominal): 125.6637 mm; feed scale 15.9155 steps/mm
- Demand at 0.50 m/s: 7957.75 Hz vs ceiling 8000 Hz -> margin 0.53 %
- Shaft speed 238.73 rpm vs limit 8.18 % margin (ceiling 260 rpm)
- Minimum loaded radius at the 8 kHz ceiling: 19.89437 mm (allowed tread/wear loss 0.10563 mm from nominal 20.0 mm)
- Decision: production ceiling stays 8000 Hz; loaded-radius loss must stay <= 0.10563 mm (min loaded radius 19.89437 mm). A higher qualified ceiling or recalibrated loaded radius is a recorded alternative (M36 gate).

## M31 length error budget (half-widths, um)

| term | half-width um | evidence | gate |
| --- | --- | --- | --- |
| command_quantisation | 48.8 | host: 2 independent count quantisations + one accumulated fraction (test_cycle) | closed_host |
| wheel_scale_calibration | 25.0 | calibration procedure + gauge R&R pending (P13) | open_P13 |
| loaded_radius_wear | 15.0 | wear/drift envelope pending (P06/P13) | open_P13 |
| slip_within_grip | 50.0 | planned movement verified against the calibrated wheel with tested error allocation (M26/M31) | open_P13 |
| tension_relaxation | 60.0 | wire relaxation model validated with held-out physical data (M23) | open_P13 |
| thermal_expansion_length | 25.0 | thermal drift (M23) + environment record (D1) | open_P10 |
| blade_datum_and_incision | 20.0 | blade datum stability (P13) | open_P13 |
| handling_deflection | 30.0 | output/handling deflection (M30/P08) | open_P08 |
| measurement_decision | 25.0 | predeclared conformity rule |e|+U<=T with U<=0.05 mm gauge (M31) | open_P13 |

- RSS of declared half-widths: 109.0 um vs external T_L(1000 mm) = 700 um -> margin 591.0 um
- Host-closed terms only: 48.8 um (quantisation class)
- Decision: external tolerance holds only if the RSS of the declared half-widths stays below T_L with the predeclared |e|+U<=T conformity rule; all open terms must close at their P-gates before capability claims (G05).

Open terms are physical gates (P06/P08/P10/P13), not fabrication: they are
scheduled with their test procedures and must not be closed by model output.
