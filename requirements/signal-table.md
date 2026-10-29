# Signal and I/O table (M27/M30 contract, Phase C deliverable)

Source of truth: `shared/controls_spec.hpp` (`pin`, `sensing`, `timing`) and
`software/machine/core/board.hpp`. Coordinates are machine millimetres from the
cut plane X0, +X toward the piece/eject side. Polarity is explicit: every input
states which physical level means what.

Status vocabulary: **ASSIGNED** = GPIO/terminal defined in `ctrl::pin` and read
in `board_rp2350.cpp`; **PHASE-H GATE** = channel defined in the contract but
without a physical pin (all 26 exposed RP2350 GPIOs are allocated; options are
an I2C GPIO expander or a board revision - to be selected and validated in
Phase H / M26-M27). Unassigned channels FAIL CLOSED in the target adapter
(`read_inputs` returns "not clear" for both handling channels): no handling
acceptance can be faked by absent hardware.

## Inputs

| Signal | Polarity / physical meaning | Source device (contract) | Pin | Status |
| --- | --- | --- | --- | --- |
| `start` | LOW while pressed | 3SU1152-0AB40-1BA0 NO, via PLC-RSC-24DC/21 coupler | GP12 | ASSIGNED |
| `stop_ok` | LOW while loop closed (button not pressed) | 3SU1152-0AB20-1CA0 NC | GP13 | ASSIGNED |
| `reset` | LOW while pressed | 3SU1152-0AB50-1BA0 NO | GP14 | ASSIGNED |
| `safety_ok` | LOW while E-stop + guard loop closed | XPSUAF13AP enabling contact 13-14 | GP15 | ASSIGNED |
| `home_front` | LOW at blade-front open end | IE5338 inductive, PNP 24 V via coupler | GP27 | ASSIGNED |
| `home_rear` | LOW at blade-rear open end | IE5338 | GP16 | ASSIGNED |
| `alm[0..3]` | LOW = driver alarm | Per-driver ALM via dedicated PLC-RSC coils | GP18-21 | ASSIGNED |
| `enc W0 A/B` | Quadrature of stock wheel (X=-385) | ifm RU3100 + AM26LV32E, TTL 2048 PPR | GP8/9 | ASSIGNED |
| `enc W1 A/B` | Quadrature of stock wheel (X=-205) | ifm RU3100 + AM26LV32E | GP10/11 | ASSIGNED |
| `enc D A/B` | Quadrature of downstream piece station (X=106.75 candidate) | ifm RU3100 + AM26LV32E (repackaged module; M02 fit gate) | **PHASE-H GATE** | pin + PIO SM needed |
| `part_clear` | **true = takeaway path clear**, false while a body is present. Present->clear pair inside the ejection window is the acceptance evidence for the unobserved ejection remainder. | Through-beam/proximity across the discharge zone [230,270] | **PHASE-H GATE** | fails closed (reads false) |
| `waste_clear` | **true = waste chute mouth clear**, false while a released stub/slug body transits. Present->clear pair after each sweep is required before the next feed/blade close. | Optical sensor on the blade-region chute fall path (drop 30 mm) | **PHASE-H GATE** | fails closed (reads false) |

## Outputs

| Signal | Meaning | Destination | Pin | Status |
| --- | --- | --- | --- | --- |
| `blade_front/rear pul/dir` | Step/dir to blade drivers | CL86T-V41 via TPL7407LDR sinks | GP0-3 | ASSIGNED |
| `feed_left/right pul/dir` | Step/dir to feed drivers | CL57T-V41 via TPL7407LDR sinks | GP4-7 | ASSIGNED |
| `out_drv_enable` | HIGH enables all drivers (100 k pull-down safe-off) | TPL7407LDR -> ENA- common | GP17 | ASSIGNED |
| `led_green/red`, `buzzer` | Panel indicators / 3SU1200 buzzer | 24 V via TPL7407LDR | GP22/26/28 | ASSIGNED |
| `waste_sweep` | Constrained mechanical sweep actuator, pulsed `waste_sweep_ms` | 24 V solenoid/relay for the blade-pocket sweep paddle | **PHASE-H GATE** | not driven until assigned |

## Station/measurand observability (every phase)

| Phase group | Stock body | Piece body | Waste body |
| --- | --- | --- | --- |
| Feed / FeedVerify / FeedCorrect / LeadAdvance / LeadRetract / RestoreStock | observed by W0+W1 (redundancy; disagreement guard) | n/a (not severed) | n/a |
| Cut (sever) | split at X0 | body [0, len] created; < 140 mm is a loose stub (never propelled by the right cassette) | stub enters blade pocket |
| TailWithdraw | W0+W1 (stock retract) | D, fully occupied (backfeed) | n/a |
| TailPull | n/a | D, fully occupied | n/a |
| Eject (final) | n/a | D for the occupied portion only; the remainder is covered by the discharge present->clear evidence before acceptance | n/a |
| LeadOpen / TailOpen | n/a | n/a | slug released to pocket |
| WasteSweep / WasteClear | n/a | n/a | chute transit (present->clear) required in production |
| Prime (first pass) | W0+W1 for the trim feed | stub classification by real length: short -> sweep; long (interrupted stock) -> scrap discharge | as at left |

## Explicitly not signals

- Commanded motor steps are **not** material motion (the counterexample defect):
  no acceptance, verification or clearance may be derived from them alone.
- `part_clear=true` and `waste_clear=true` are **never** default-bypasses: the
  target adapter returns false while the channels are unassigned, and the
  controller requires the transitions in production (`part_clear_check=1`).
- Service fixture mode (RAM-only, `Msg::SetFixtureMode`) is engineering-only:
  no accepted production counts (`pieces_fixture`), no persistent approval.
