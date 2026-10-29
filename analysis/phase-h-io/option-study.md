# Phase-H I/O architecture option study (signal-table gates)

**Status:** engineering option study, NOT a selected design. Candidate part
identities are family-level leads; exact MPNs/ratings must be confirmed at
quotation (G38/G39 rule). Decision owner: completion/user (feeds D1/D8).

## Requirement (from requirements/signal-table.md)

| channel | class | timing need | electrical |
| --- | --- | --- | --- |
| `part_clear` (discharge present->clear) | digital input | edge capture ~10 ms resolution; 20 ms debounce in firmware | 24 V field -> logic level |
| `waste_clear` (chute transit) | digital input | edge capture ~10 ms resolution | 24 V field -> logic level |
| `waste_sweep` (sweep actuator) | digital output | pulse 200 ms | 24 V solenoid/relay drive |
| station-D quadrature (A/B) | fast counter | up to ~20.5 k edges/s at 0.50 m/s (2048 PPR x4, 200 mm wheel) | RS-422/TTL via AM26LV32E class receiver |

All 26 exposed RP2350 GPIOs are allocated (GP0-22, 26-28). Firmware is already
written against the channels (fail-closed defaults), so the decision is purely
how to physically deliver them.

## Option A — I2C expander for the 3 slow channels + board revision only for D

- Slow channels: TCA9555-class 16-bit I2C GPIO expander (100/400 kHz, interrupt
  output) or MCP23017-class equivalent, powered from the logic rail with 24 V
  field levels dropped through the existing coupler-relay pattern (PLC-RSC
  24DC/21 couplers already isolate field inputs; the expander reads the coupler
  contact side at logic level, consistent with the current input design).
- Timing: firmware polls at 1 kHz; I2C transaction to read 2 ports at 400 kHz
  is well under a millisecond, and the 20 ms debounce masks jitter. Edge
  capture fidelity is preserved because the controller samples continuously.
  The expander's interrupt output can raise a GPIO already allocated to the
  function (e.g. share an ALM input through a diode-OR only if the electrical
  release proves it - do not silently share lines).
- Station D: quadrature at 20.5 k edges/s cannot ride I2C GPIO polling. It
  needs either (1) a board revision exposing 2 pins for a PIO SM (cleanest;
  reuses the existing quadrature.pio path), or (2) a dedicated SPI quadrature
  counter IC (LS7366R-class) clocked from a repurposed/expander-assisted SPI
  port with the reading taken in firmware - more components, a new protocol
  path, and its own validation burden.
- Cost class: expander + couplers is a few USD; a board respin has NRE but
  delivers all four channels natively.

## Option B — full board revision (recommended default until costed)

Add one 2x4 header area: 2 pins (D A/B -> PIO), 2 pins (part_clear, waste_clear
via the coupler pattern), 1 pin (waste_sweep gate to the existing TPL7407LDR
sink pattern), plus spares. The RP2350 has PIO headroom (a third SM for D).
This keeps every channel on native firmware paths (quadrature.pio for D, GPIO
for handling), keeps the fail-closed behaviour, and needs no new protocol.

## What the decision must not do

- No signal may be represented only by a boolean constant or a vague spare
  input (M27 contract). Until a channel is physically implemented, the target
  keeps failing closed (reads "not clear") and production handling verification
  correctly faults.
- The piece-station mechanical fit (M02: 63.66 mm wheel/encoder/housing in the
  X38-140 window among the nips at 75.5/138) is a separate gate; solving the
  electrical side does not close it.

## Open actions

1. User/owner: choose Option A or B (feeds D1/D8 and the RFQ set).
2. If B: schematic/PCB revision task with its own review; update
   `shared/controls_spec.hpp` pin map and `requirements/signal-table.md`
   statuses in the same change.
3. If A: select exact expander/counter MPNs at quotation, prove the timing in
   a bench test (P04-T4 extension), and keep D's 20.5 k edges/s path native.
