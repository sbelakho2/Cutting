# Cabinet electrical assembly and verification procedure — draft

**Revision:** `S1-B-CTRL-CAB-R2-2026-09-13`  
**Status:** design-controlled assembly plan — **not** a released work instruction
or permission to build, energise, commission or operate the machine.

This procedure organizes the current cabinet design inputs. It does not replace
the applicable electrical standards, a reviewed point-to-point schematic,
competent-person review, local regulatory requirements or the machine risk
assessment. The functional circuit source is [controls and wiring](22-controls-and-wiring.md).
The exact IF-B01 pin allocation is [the integration schedule](../hardware/if-b01/if-b01-cabinet-integration.md).
Outstanding physical items are listed in [the cabinet wiring BOM](../bom/selected-cabinet-wiring.csv).

## 1. Controlled inputs before work

1. Record the current revisions of the XPS instruction, E-stop, guard lock,
   contactors, supplies, IF-B01 source/output manifest and approved cabinet
   drawing set. Stop if any identity or revision differs.
2. Obtain written review of the selected terminal system, enclosure, incoming
   protection, PE arrangement, conductor ampacity, DC branch protection,
   contactor DC utilisation, regeneration/discharge path, cable entries,
   separation and thermal management. These items are presently open.
3. Do not begin any guard-release wiring: no authorised E1/E2 release-permission
   design exists.

## 2. De-energised installation sequence

1. Install the enclosure, PE bar, DIN rail, terminals and cable-entry system
   only after their selected products and layout drawing are approved. Mark each
   terminal with its controlled designation: TB24-01, TB24-02, TB24-03, TB-DL,
   TB-S, TB-M1, TB-M2, TB-M3 and PE-BAR.
2. Install the selected 24 V and 5 V supplies, XPS relay, K1/K2/K3, KA1–KA10
   and KSR1 according to approved mounting, spacing and thermal instructions.
   KSR1 is separate from KA3; do not combine their contact functions.
3. Install the IF-B01 only after the current draft board has passed its separate
   fabrication/assembly release gates. Re-run the source allocation check in
   [if-b01-gpio-allocation.csv](../hardware/if-b01/if-b01-gpio-allocation.csv)
   when a board, firmware or connector record changes. Keep J4–J7 disconnected
   from motor drivers during bare-board fixture testing.
4. Route encoder cables separately from AC and switched 36/60 V wiring. Bond
   each encoder screen at the PE entry and leave the two unused Z conductors
   individually insulated.
5. Terminate J3, J10, J11 and J12 strictly by the integration schedule. Apply
   no 24 V field signal to J10/J11; they carry isolated dry-contact pairs only.

## 3. Mandatory records before applying power

| Record | Actual result / reference | Reviewer / date |
|---|---|---|
| Equipment identities and revision traceability | | |
| Terminal, cable, ferrule and crimp-tool selection | | |
| Terminal torque method and recorded values | | |
| PE continuity and bonding test | | |
| Point-to-point continuity against approved schematic | | |
| Segregation/clearance and cable-screen inspection | | |
| Incoming/branch protection coordination review | | |
| 24 V and 5 V load/inrush/voltage-drop calculation | | |
| K1/K2/K3 DC switching and discharge design review | | |
| IF-B01 bare-board fixture record | | |

Any blank cell is an unclosed hold, not a pass.

## 4. Energisation and safety verification gates

Energisation requires a separately approved procedure and competent oversight.
It must verify actual—not assumed—power-domain voltages, PE integrity, E-stop
channels, guard interlock/locked-state channels, XPS reset/EDM behavior,
K1/K2/K3 dropout, DC-bus voltage decay, regenerated-energy behavior, fault
response and restart prevention. The $20\ \mathrm{ms}$ XPS component response
property does not establish a machine stop time.

No firmware behavior, IF-B01 output, USB command, `GP15` diagnostic, watchdog
or controller state can replace hardwired energy isolation. Follow the staged
physical qualification plan in [physical qualification](20-physical-qualification.md)
before machine operation.