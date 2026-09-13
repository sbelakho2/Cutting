# IF-B01 bare-board fixture procedure

**Status: draft fixture procedure. It is not a production acceptance procedure
and does not release a board, cabinet or machine.** No assembled board has
passed this procedure. Record every actual reading, instrument, board identity,
fixture identity, firmware hash and anomaly; do not replace blank records with
expected values.

This procedure applies only to an isolated **bare IF-B01 board fixture**. Keep
the 36 V and 60 V supplies off and physically disconnected. J4–J7 must be
unplugged from every production motor driver for the entire test. The board is
not part of a safety function during this procedure.

## 1. Required setup

- Pico 2 programmed with the dedicated `if-b01-factory-test.uf2` image from
  the RP2350 build, not `starz-machine.uf2`.
- USB data connection to the fixture host and an agreed fixture-power method
  that prevents unreviewed USB/J3 back-feeding. The power arrangement itself
  must be recorded.
- Current-limited, isolated 24 VDC PELV bench supply for J3-1/J3-2 and a
  regulated 5 VDC fixture supply for J3-3/J3-4. Start at conservative current
  limits suitable for the fixture and stop on unexpected current or heating.
- DMM and, where available, oscilloscope or logic probe; a compliant
  differential RS-422 quadrature source or actual low-voltage encoder fixture;
  insulated shunts; and 24 V-rated high-value test loads. A 10 kΩ test load is
  suitable for observing the intended low-side output state without attaching
  a motor driver.

Before power is applied, inspect the actual assembly against the draft
[component list](if-b01-bom.csv), generator-owned [placement schedule](if-b01-placement.csv),
[netlist](if-b01-netlist.csv), [assembly procedure](if-b01-assembly.md) and
[draft output package](manufacturing/draft-r2-2026-09-13). Check
for solder bridges, wrong orientation, damaged terminals and unintended shorts
between `+24V_PELV`, `+5V_LOGIC`, `3V3_MCU` and `0V_PELV`. No numerical
resistance/current acceptance threshold is yet qualified; stop and investigate
any unexpected result rather than accepting it by comparison with an assumed
value.

## 2. Fixture state and console commands

The factory image boots with all 12 TPL inputs low. It exposes an ASCII USB CDC
console:

| Command | Fixture behavior |
|---|---|
| `IDENT` | Reports the factory-image identity |
| `STATE` | Reports armed state, active-low safety/reset state and active output |
| `INPUTS` | Reports all ten input raw and asserted states |
| `ENC` | Reports left/right count, A/B state and health |
| `ENC RESET` | Clears and restarts both fixture encoder counters |
| `ARM` | Arms outputs for at most 30 s, only while `SAFETY_OK` and `RESET` are held asserted |
| `OUT <0-11> 1` | Enables exactly one listed output for at most 3 s |
| `OUT <0-11> 0` | Forces every output off |
| `WALK <50-2000>` | Walks all 12 outputs with the requested dwell in milliseconds |
| `SAFE` | Disarms and forces every output off |

`ARM`, `OUT` and `WALK` require **both** `SAFETY_OK` and `RESET` to be active
low continuously. The required temporary fixture shunts are:

| Function | IF-B01 shunt |
|---|---|
| `RESET` asserted | J10-5 (`IN_RESET`) to J10-6 (`0V_PELV`) |
| `SAFETY_OK` asserted | J10-7 (`IN_SAFETY_OK`) to J10-8 (`0V_PELV`) |

With the shunts absent, `INPUTS` should report the respective inputs unasserted.
With each shunt installed, it should report `asserted=1` and `raw=0`. Do not
attach a real safety relay to create this test state; the shunts exist only in
the isolated bare-board fixture.

## 3. Test order

1. **Power and console.** Leave J4–J12 unplugged. Connect the recorded fixture
   supplies at J3 and connect USB without creating an unreviewed second power
   path. Confirm the startup text says that all outputs are low. Run `IDENT`
   and `STATE`; record their output. Verify the 3.3 V rail and that no 5 V is
   present at any Pico GPIO header pad.
2. **Default-safe output state.** With `RESET` and `SAFETY_OK` shunts absent,
   use `STATE`. Probe each intended output/test load and verify that no low-side
   channel is deliberately asserted. Reset or unplug the Pico while the fixture
   supplies remain present and recheck the shared enable test point/output.
   This is a board-fixture observation only, not safety validation.
3. **Protected dry-contact inputs.** With all input shunts removed, run
   `INPUTS` and record the raw state. One pair at a time, short each listed
   signal to its paired 0 V terminal, confirm `asserted=1 raw=0`, remove the
   shunt and confirm deassertion. Test J10 pairs 1/2 through 11/12 and J11
   pairs 1/2 through 7/8. Do not inject 24 V into J10/J11.
4. **Encoder inputs.** Connect one compliant differential A/B source at a
   time to J8 and J9 using the exact netlist order. Run `ENC RESET`, drive valid
   quadrature slowly in one direction, then reverse it. Run `ENC` and record
   count sign/change and `healthy=1` for both sides. Leave the Z/Z-bar
   conductors individually insulated; they are not IF-B01 inputs. Investigate
   invalid transitions or a false count with an open connector; do not declare
   a fail-safe encoder state without a reviewed circuit test.
5. **Prepare low-side test loads.** Fit a 10 kΩ load from fixture `+24 V` to
   the named negative output only. Do not connect real lamps, buzzers or motor
   drivers. For the shared enable output, fit the same test load to each of
   J4–J7 pin 3 if all four branches are being observed.
6. **Arm while held.** Install both fixture shunts from section 2 and run
   `INPUTS`, then `ARM`. Confirm the console reports a 30 s armed window. If
   either shunt is opened, arming/output control must fail and a fresh `ARM` is
   required after restoring the shunt.
7. **Individual output polarity.** Use `OUT <index> 1`, observe only the
   corresponding low-side output pull the test load toward 0 V, then observe
   automatic output removal within 3 s. Run `SAFE` between observations. The
   index map is below. A selected output test must never be repeated with a
   motor driver connected.
8. **Safety-shunt abort.** While one harmless 10 kΩ test load is active, remove
   the `SAFETY_OK` fixture shunt. Confirm the console reports `SAFE safety-loop
   opening` and the output returns to its inactive state. Restore the shunt;
   confirm no output reasserts and that `ARM` is required again. This verifies
   factory-image behavior only.
9. **Optional bounded walk.** With both shunts continuously installed, run
   `ARM` and `WALK 50` after all individual observations pass. Observe the
   declared one-at-a-time order and final `OK WALK complete; outputs safe`.
   Do not select a dwell that would exceed the 30 s arm window.
10. **Final safe state.** Run `SAFE`, remove both fixture supplies, remove all
    temporary shunts/test loads, then record test results. Do not connect the
    board to a cabinet or motor driver as the next step of this procedure.

## 4. Output index map and test connection

| Index | Factory console name | Connector terminal to receive a 10 kΩ test load from +24 V |
|---:|---|---|
| 0 | U2.CH1 BF_PUL | J4-1 |
| 1 | U2.CH2 BF_DIR | J4-2 |
| 2 | U2.CH3 BR_PUL | J5-1 |
| 3 | U2.CH4 BR_DIR | J5-2 |
| 4 | U2.CH5 FL_PUL | J6-1 |
| 5 | U2.CH6 FL_DIR | J6-2 |
| 6 | U2.CH7 FR_PUL | J7-1 |
| 7 | U3.CH1 FR_DIR | J7-2 |
| 8 | U3.CH2 ENA | J4-3, J5-3, J6-3 and J7-3 (shared net) |
| 9 | U3.CH3 GREEN | J12-1 |
| 10 | U3.CH4 RED | J12-2 |
| 11 | U3.CH5 BUZZER | J12-3 |

The factory image turns every output off before asserting a selected output.
`OUT <index> 0` and `SAFE` both turn all outputs off; `SAFE` also disarms.
Any loss of the safety shunt, reset shunt or arm window forces all outputs off.
The remaining two U3 channels must be tied low in the actual design and must
have no field connection; inspect this in native schematic/layout review rather
than trying to energise them.

## 5. Required fixture record

| Field | Actual value |
|---|---|
| Board serial/revision and operator | |
| Date, fixture identity and ambient conditions | |
| `if-b01-factory-test.uf2` SHA-256 and Pico identity | |
| USB/J3 power arrangement and 24 V/5 V supply identities | |
| Unpowered inspection observations | |
| J10/J11 input results | |
| J8/J9 encoder results | |
| Outputs 0–11: observed inactive/active/timeout state | |
| Safety-shunt abort observation | |
| Deviations, rework, reviewer and disposition | |

A complete fixture record does not qualify IF-B01 for machine use. The future
PCB release, cabinet design, hardwired safety verification, DC energy test and
physical machine qualification remain separate required gates.
