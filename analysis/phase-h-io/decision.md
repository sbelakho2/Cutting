# Phase-H I/O architecture — SELECTED DESIGN DECISION (review F03)

**Date:** 2026-10-30. **Status:** DECIDED (replaces the A/B menu in
`option-study.md`). PCB/implementation work remains open hardware work; the
architecture choice is no longer open.

## Decision

**Selected: Option B — board revision adding native channels** (a new I/O area
on the IF-B01 carrier PCB, respun as IF-B02). The four channels come out as
native RP2350 resources:

| channel | delivery | RP2350 resource | notes |
| --- | --- | --- | --- |
| station-D quadrature A/B | direct GPIO -> PIO SM2 | 2 new GPIOs (PIO capable) | reuses `quadrature.pio`; 20.5 k edges/s at 0.50 m/s needs PIO, never I2C polling |
| `part_clear` | coupler-isolated input (PLC-RSC 24DC/21 class, same pattern as existing field inputs) | 1 new GPIO | fail-closed semantics unchanged |
| `waste_clear` | coupler-isolated input | 1 new GPIO | fail-closed semantics unchanged |
| `waste_sweep` | TPL7407LDR-class sink output (same pattern as existing outputs) | 1 new GPIO | drives the sweep actuator relay/solenoid |

The respin also fixes the field-input stage (review F08): the bias/wetting
network is corrected so contact wetting current is above every relay minimum
across the hot range (the 57.9 uA defect class), the J11/P11 connector
identities are corrected to the real eight-position mates, and the Keystone
9903 fastener description is fixed to its actual M3 role.

## Rejected alternatives (with reasons)

- **Option A (I2C expander for slow channels only):** rejected because station D
  cannot ride I2C polling (20.5 k edges/s quadrature), so the expander would
  still require a board change for D while adding a new protocol path, a new
  component class and its own validation burden. Partial adoption leaves two
  architectures to validate.
- **SPI quadrature counter IC (LS7366R class):** rejected as a second protocol
  path with extra components; PIO on native pins is already proven by the
  existing W0/W1 implementation.
- **Sharing existing ALM/limit inputs (diode-OR / multiplexing):** rejected -
  the observability contract (requirements/signal-table.md) forbids vague or
  shared channels; evidence channels must be independent.

## Implementation spec (work order for the respin)

1. Carrier PCB respin IF-B02: 5 new GPIOs (2 PIO-capable for D, 2 coupler
   inputs, 1 sink output) + corrected field-input bias network + corrected
   connector/MPN identities (J11, P11, Keystone rows).
2. Update `shared/controls_spec.hpp` pin map and `requirements/signal-table.md`
   statuses in the SAME change (no signal left unassigned across a release).
3. Firmware: assign D to PIO SM2 (existing `quadrature.pio`), map the handling
   channels to the new GPIOs; keep fail-closed defaults until the pins exist.
4. Cross-build both images (production + factory test) on the target toolchain;
   run the factory fixture covering every new channel (P04-T4 extension).
5. BOM: add the new connector/coupler/fastener occurrences to the purchasing
   register (see `bom/phase-h-io-occurrences.csv`).
6. Evidence gates: P04-T4 (wiring audit), P03-T5 (wetting measurement),
   P11-T4 (channel timing), P08-T1/T3 (handling channels on real hardware).

## What this decision does NOT close

The PCB layout, schematic, DRC/ERC, board bring-up and physical fixture work
remain open hardware tasks (review F03 acceptance: "a real target or
electrically faithful HIL must acquire D and both handling channels..."). The
station-D mechanical fit (M02: 63.66 mm wheel/encoder/housing in the X38-140
window among the nips) remains its own gate.
