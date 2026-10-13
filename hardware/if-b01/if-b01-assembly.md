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
3. Inspect IC leads, clamp diodes, PTCs, mid-bias networks and passives at
   suitable magnification. Record bridges, insufficient wetting, lifted leads
   and tombstones as nonconformances.
4. Fit the Pico sockets and J3–J12 headers only after their exact footprint,
   mating orientation, mechanical clearance and the fabricator's capability for
   the board panel are verified. The draft names Phoenix MSTB 5.08 mm parts; do
   not substitute pitch or connector family without electrical/mechanical review.
5. Before any encoder is wired to J8/J9, parameterise each ifm RU3100 through
   its IO-Link interface to **TTL mode** and **2048 PPR** (factory setting is
   HTL 1024 PPR) and record the parameter set; an IO-Link v1.1 master with a
   Class A M12 port (e.g. ifm USB IO-Link master, E30390-class — identity
   confirmed at purchase), the RU3100 IODD and an M12 A-coded cable are
   required. The single-ended receiver interface and the 2048-count scaling
   are valid only in TTL mode.
6. Perform documented unpowered checks before inserting a Pico. Then run the
   dedicated bare-board fixture procedure in [if-b01-test.md](if-b01-test.md).
   Never use production motor drivers as test loads.
7. Fit the Pico and use nonconductive mounting hardware only after the board
   has a recorded fixture result. Cabinet mounting remains blocked by the
   release holds in [the cabinet control record](../../docs/22-controls-and-wiring.md).

## Provisional inspection checks

These are checks to transfer into the future controlled inspection record; no
pass result is implied today.

| Check | Intended observation |
|---|---|
| U1/U2/U3 identity | `AM26LV32EIDR`, `TPL7407LDR`, `TPL7407LDR` respectively |
| J1/J2 socket rows | J1 pad 1 (GP0) at the row top; J2 pad 1 (GP16) at the row bottom exactly where physical Pico pin 21 sits; both rows 17.78 mm apart. Confirm against the official Pico 2 pinout, not against pad numbering alone |
| R1–R4 | Must be **absent**: the RU3100 interface is single-ended TTL and no 120 Ω terminator is fitted |
| R38–R45 / C14–C17 | 10 kΩ 1 % / 100 nF mid-bias network present on all four receiver minus inputs (`L_ENC_A_N`, `L_ENC_B_N`, `R_ENC_A_N`, `R_ENC_B_N`); 2.5 V at each node with no encoder connected |
| R5 | 100 kΩ pull-down from the U3 shared-enable input to `0V_PELV` |
| F1–F3 | `MF-MSMF050/60X` at F1 (24 V branch) and `MF-MSMF050/16X` at F2/F3 (5 V encoder branches); inspect identity and polarity-free placement against the schematic before treating cold continuity as meaningful |
| Dry-contact inputs | Intended 10 kΩ/47 kΩ/10 nF/3.6 V protection network present on all ten inputs; external 47 kΩ pull-up only — RP2350 internal pulls must be disabled in both firmware images (pull-configuration contract) |
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
