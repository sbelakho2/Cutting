# IF-B01 draft assembly and inspection procedure

**Status: placed/routed KiCad source and source-generated draft assembly outputs
exist, but no released PCB or assembled board exists.** This record defines the
draft assembly and inspection sequence for a board fabricated only after the
human design, fabricator and release gates are closed. It is not evidence that a
board has been ordered, assembled, tested or approved for energisation.

The source of intent is [README.md](README.md), the editable
[native schematic](if-b01.kicad_sch), [component list](if-b01-bom.csv),
[netlist](if-b01-netlist.csv), generator-owned [placement schedule](if-b01-placement.csv)
and [draft output package](manufacturing/draft-r2-2026-09-13). The present
[PCB file](if-b01.kicad_pcb) is a $750\times400\ \mathrm{mm}$ placed/routed,
four-layer draft with clean current ERC/DRC/parity evidence. Its PCB geometry,
generated visual assembly exports and fabrication files still require human and
fabricator review before use as an assembly instruction.

## Draft sequence after release authorisation

1. Compare the received bare PCB identity, $750\times400\ \mathrm{mm}$ outline,
   mounting holes, terminal legends and fabrication data with the reviewed native
   source and generated draft package. Stop on any mismatch.
2. Fit/reflow the SMD parts using the manufacturer-recommended package process
   and the reviewed assembly drawing. Verify U1/U2/U3 orientation from the
   native silkscreen/assembly output, not from an assumed coordinate system.
3. Inspect IC leads, clamp diodes, fuses, terminations and passives at suitable
   magnification. Record bridges, insufficient wetting, lifted leads and
   tombstones as nonconformances.
4. Fit the Pico sockets and J3–J12 headers only after their exact footprint,
   mating orientation, mechanical clearance and the fabricator's capability for
   the board panel are verified. The draft names Phoenix MSTB 5.08 mm parts; do
   not substitute pitch or connector family without electrical/mechanical review.
5. Perform documented unpowered checks before inserting a Pico. Then run the
   dedicated bare-board fixture procedure in [if-b01-test.md](if-b01-test.md).
   Never use production motor drivers as test loads.
6. Fit the Pico and use nonconductive mounting hardware only after the board
   has a recorded fixture result. Cabinet mounting remains blocked by the
   release holds in [the cabinet control record](../../docs/22-controls-and-wiring.md).

## Provisional inspection checks

These are checks to transfer into the future controlled inspection record; no
pass result is implied today.

| Check | Intended observation |
|---|---|
| U1/U2/U3 identity | `AM26LV32EIDR`, `TPL7407LDR`, `TPL7407LDR` respectively |
| R1–R4 | 120 Ω, 1 % RS-422 termination parts installed across the named input pairs |
| R5 | 100 kΩ pull-down from the U3 shared-enable input to `0V_PELV` |
| F1–F3 | `MF-R050` parts in the intended three branch positions; inspect against schematic before treating cold continuity as meaningful |
| J8/J9 | 120 Ω termination across each named A/A-bar and B/B-bar pair, subject to schematic review |
| Dry-contact inputs | Intended 10 kΩ/47 kΩ/10 nF/3.6 V protection network present on all ten inputs |
| Supply nets | No unintended short between `+24V_PELV`, `+5V_LOGIC`, `3V3_MCU` and `0V_PELV` |
| Unused U3 channels | Inputs tied low and no field connector/net assigned |
| Connector labels | Match the reviewed schematic and generated assembly output, then the draft netlist |

## No substitution without redesign review

The previous logic-buffer, logic-transceiver and Darlington-array arrangement
is not the current IF-B01 architecture. Do not substitute any of those parts
for the intended `AM26LV32EIDR` / `TPL7407LDR` circuit. A different
differential receiver, low-side driver, protection network, connector family,
connector pitch or board material is an electrical/mechanical redesign
requiring source, review, ERC/DRC and fixture retest.
