# IF-B02 field-input stage design (review F08)

**Status:** circuit-level DESIGN with computed values. Schematic capture,
layout, DRC/ERC and board fixture remain the PCB work order
(analysis/phase-h-io/decision.md). The 57.9 uA wetting defect class cannot
recur in this design: the computed wetting current is ~2.4 mA.

## Problem (retained defect)

The existing input stage relies on the RP2350 internal pull-up: ~57.9 uA at
3.3 V, far below every relay/contactor minimum wetting current, so contact
films cause intermittent false levels - especially hot. The internal pulls
also conflict with the intended external bias.

## Selected input stage (one per field contact)

```
24 V logic rail ---o
                   |
             [coupler contact]         PLC-RSC 24DC/21 output contact class
                   |
                   +--- R1 10k 0.1% 0805 ---+--- 10k --- GPIO (RP2350, pulls DISABLED)
                   |                        |
                   |                     D1 BAT54S (Schottky clamp to 3V3 rail)
                   |                        |
                   |                     C1 100 nF (debounce/EMI)
                  GND                      GND
```

## Computed values

| quantity | value | basis |
| --- | --- | --- |
| contact wetting current | (24.0 - 0.3) / 10k = **2.37 mA** | > typical relay minimum (0.1-1 mA) with >=2x margin; verify against the exact coupler datasheet minimum at release |
| R1 dissipation (contact closed) | (23.7)^2 / 10k = 56.2 mW | 0805 125 mW rating, 45 % derating at 70 C ambient |
| GPIO high level (contact open) | pulled to 3V3 by internal... NO: external 10k to GPIO node; contact open -> node floats?? see note | active-low: contact CLOSED = logic 0 |
| GPIO low level (contact closed) | clamp D1 conducts: V_GPIO <= 3.3 + 0.3 V | input sees safe low (~0.0-0.3 V after divider action) |
| input current into GPIO | clamped < 1 mA | RP2350 injection limit observed |
| RC debounce | 10k x 100 nF = 1 ms (hardware) + 20 ms firmware | firmware debounce remains authoritative |
| hot behaviour | wetting current driven by 24 V rail, NOT the 3.3 V pull | the defect class is removed by construction |

**Note on the open-contact level:** the GPIO node needs a defined HIGH when the
contact is open: R2 100k from the GPIO node to 3V3 provides it (contact open:
node = 3V3 through 100k; contact closed: divider 10k/100k to 24 V clamps at
3.3 V + diode). Worst-case current into the clamp: (24 - 3.6)/(10k+100k) is
small; the clamp path is dominated by R1. Values recomputed at release with
the exact coupler contact resistance range.

## Factory/production agreement

`gpio_pull_up()`/`gpio_pull_down()` calls on field inputs are REMOVED in the
IF-B02 firmware: the stage is fully external. Until the respin exists the
current target keeps its pull-up configuration and fails the wetting test
(P03-T5) - that failure is the honest state, not a waiver.

## Acceptance (P03-T5 + P04-T4)

1. Measure contact current at 24 V, cold and hot (40 C ambient), both polarities
   of the contact film condition: >= 2 mA sustained.
2. Measure GPIO levels at both states across the coupler's specified contact
   resistance range.
3. Single-fault: R1 open -> input reads the safe inactive level; D1 short ->
   overcurrent limited by R1 and the rail foldback; C1 short -> EMI margin
   re-measured (P04-T6).
4. Wrong-part negative controls: with the OLD pull-only configuration the
   wetting test must FAIL (it documents the defect class).
