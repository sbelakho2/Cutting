# M28/M29 reel and buffer physics (desk closure)

Deterministic output of `analysis/reel-buffer/compute.py` - replaces the mislabelled tooth-pass script note (review F12: tooth-pass is M28-M16 dynamics input, NOT a reel model).

- Envelope: 30.0 kg total, flange 400 mm, hub 60 mm, width 160 mm (dual-mode small cassette + large cradle; package freeze = owner D3)
- Inertia: J_full = 0.6135 kg.m2, J_empty = 0.1227 kg.m2
- Unwind torque demand:

  - wire 24.0 kg at r=102 mm: 0.82 N.m at 8 N tension, 0.20 N.m at 2 N; 46.7 rpm at 0.50 m/s
  - wire 12.0 kg at r=75 mm: 0.60 N.m at 8 N tension, 0.15 N.m at 2 N; 63.4 rpm at 0.50 m/s
  - wire 1.0 kg at r=36 mm: 0.29 N.m at 8 N tension, 0.07 N.m at 2 N; 132.5 rpm at 0.50 m/s

- Brake hold requirement: 0.82 N.m at full-pack radius
- Dancer storage floor: 172 mm (0 -> 750 mm/s demand vs supply slew)

Open gates:

- reel envelope freeze (D3): exact package OD/core/width/bore per family
- drive/brake selection with identified parts and hot ratings (G38/G39)
- dancer mechanism, control loop and fault behaviour (M29, P08/P14)
- full/mid/empty tension and storage validation on hardware (P06/P15)
- 30 kg loading method and retention (P15-T4)
