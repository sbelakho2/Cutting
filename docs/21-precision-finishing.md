# Precision finishing — S1-B custom parts

Revision `S1-B-FINISH-R1-2026-09-13`. Limits and pass schedules live in [finishing.hpp](../engineering/include/finishing.hpp); the card generator is `starz_finishing_cards` ([source](../engineering/src/finishing_cards.cpp)). Templates: [seat](../engineering/generated/finishing/templates/seat-records-template.csv), [bore](../engineering/generated/finishing/templates/bore-records-template.csv).

**Status: no part has been finished or measured. The templates are blank and the tool rejects them (exit 1, `run_complete:false`).** The `SYN-` fixtures under `engineering/tests/fixtures/` exercise the arithmetic only.

## Why per-part cards

The j5 bearing seats on F101 are matched to the actual bearing bore ([bearing-fit report](../engineering/generated/bearing-fit/report.md), minimum residual 1.28 µm within its thermal envelope). A fixed drawing size cannot hold that; the seat target is computed for each shaft/bearing pair from the measured bearing bore, corrected to 20 °C, and guard-banded by the gauge U(k=2) per ISO 14253-1.

## F101 bearing seats — cylindrical grinding

| Item | Value |
|---|---|
| Material / condition | 42CrMo4+QT, ≈300 HB, turned to Ø(nominal + 0.30) ± 0.05 before grinding, between centres |
| Wheel / coolant | ISO 525 A60K5V vitrified Al₂O₃, water-soluble coolant |
| Roughing | 0.020 mm diametral per pass until 0.040 mm remains |
| Finishing | 0.005 mm per pass until 0.010 mm remains |
| Final | 0.002 mm per pass to target, then 2 spark-out passes |
| Form / finish | cylindricity 1.0 µm, roundness 0.6 µm, Ra 0.4 µm, runout to centres 2.0 µm |
| Gauge | comparator or air gauge, U(k=2) ≤ 1.0 µm, identity recorded |
| Acceptance | measured size corrected to 20 °C inside `[lower + U, upper − U]`; window must remain ≥ 1.0 µm |

The card lists the pass counts for the worst pre-grind blank, the guarded window, target and resulting interference band against that bearing. A shaft that lands outside its window is regraded to another bearing or rejected (`acceptFinishedSeat`).

## F102 / F112 housing bores — fine boring

6082-T6 on the 4-axis CNC with a micrometric boring head: semi-finish to target − 0.15 mm diametral, gauge, then one 0.15 mm finishing pass. Cylindricity 4.0 µm, Ra 0.8 µm, assembled and dowelled F102/F112 coaxiality 10 µm. Three-point bore gauge U(k=2) ≤ 2.0 µm. Acceptance by `acceptFinishedBore` with the aluminium expansion coefficient 23.4 µm/(m·K).

## Faces — surface grinding

H003 adapter 15.00 ± 0.02 thickness and H001 flatness 0.03/300: 0.30 mm grinding stock, parallelism 10 µm, Ra 0.8 µm.

## Procedure

1. `starz_finishing_cards --template <dir>`; fill one seat row per shaft/bearing pair (bearing bore and its measurement temperature from the incoming inspection) and one bore row per housing.
2. `starz_finishing_cards --seats <csv> --bores <csv> --out <dir>` writes `grind-cards.csv`, `bore-cards.csv`, `report.md`, `status.json`. Exit 1 on blank/malformed rows, 2 on I/O error.
3. Grind/bore to the card; record the final measurement and temperature next to the card; accept only inside the guarded window.

Recorded measurements are the only proof that these limits were met.
