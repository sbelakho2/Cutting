# M33 / M21 / M20 electrical-thermal desk closure

Deterministic output of `analysis/electrical_thermal/compute.py`.

## M33 branch absorption  `V_final = sqrt(V_i^2 + 2E/C)`

| case | V_i | E (J) | C (F) | V_final | within +10 % of nominal |
| --- | --- | --- | --- | --- | --- |
| stored_counterexample_36V_100uF_0.1J | 36.0 | 0.1 | 0.0001 | 57.411 | NO |
| candidate_36V_2200uF_0.1J | 36.0 | 0.1 | 0.0022 | 37.241 | yes |
| candidate_60V_2200uF_0.5J | 60.0 | 0.5 | 0.0022 | 63.675 | yes |
| candidate_60V_4700uF_0.5J | 60.0 | 0.5 | 0.0047 | 61.748 | yes |

The stored counterexample (36 V, 100 uF, +0.1 J -> 57.411 V) demonstrates why a terminal nominal voltage is never a branch safety argument. Absorption/bleed parts per branch with measured bus decay are the G33 gate.

## M21 bleed envelope

- Candidate: 18 kohm 25 W class bleed envelope (historical audit; identity unverified)
- 36 V/2200 uF: 33.7 V after 2.6 s, 7.9 V after 60 s
- 60 V/4700 uF: 58.2 V after 2.6 s, 29.5 V after 60 s
- Decision: power taper is NOT a discharge guarantee: measured deterministic bus decay with the exact retained resistor/relay per branch is required before guard release (G33/G36, P10/P11).

## M20 ventilation balance (candidate)

- Declared loss budget 650.0 W; required service flow 800.0 m3/h
- Two-path catalogue 890.0 m3/h at 50 Hz (445 per fan); 60 Hz alternative is a separate case
- Decision: candidate only: installed P-Q intersection with the real system resistance under final filters/doors/ducts must be measured (TS01-TS18).
