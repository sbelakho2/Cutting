# IF-B01 control interface board — placed/routed draft source

**Status: a complete native KiCad 10 source and a source-generated draft output
package exist, but neither is released for fabrication, assembly, energisation
or machine operation.** The current four-layer, $750\times400\ \mathrm{mm}$
board source contains every electrical footprint and routing. KiCad 10.0.6
reports zero ERC violations, zero physical DRC violations, zero unconnected
pads and zero schematic-parity issues. The source-generated Gerber, drill,
placement, board-only STEP, IPC-D-356 and BOM files are available as an
explicitly marked draft package.

These source checks demonstrate only consistency of the controlled KiCad
files. They do not approve the deliberately spacious routing-cell layout,
component land patterns, connector fit, panel size, stack-up, electrical
design, fabrication, assembled board, cabinet integration or safety function.

IF-B01 is intended to be the PELV/SELV interface between a Raspberry Pi Pico 2
and 24 V common-anode drive inputs, indicators, dry-contact field interfaces,
and 5 V RS-422 measuring encoders. It contains neither mains circuitry nor a
functional-safety circuit. The XPS relay, E-stop, guard contacts and contactors
remain hardwired cabinet functions described as an unreleased design concept in
[the cabinet control record](../../docs/22-controls-and-wiring.md).

## Current design intent

| Function | Draft implementation | Controller-facing result |
|---|---|---|
| PUL/DIR | `TPL7407LDR` low-side NMOS channels to common-anode 24 V inputs | `GP0`–`GP7` high is intended to sink the named drive input |
| Shared enable | One `TPL7407LDR` channel with a `100 kΩ` input pull-down | `GP17` high is intended to sink every `ENA−`; reset/power loss should keep the channel off |
| Indicator/buzzer | `TPL7407LDR` low-side channels | `GP22`, `GP26`, `GP28` high are intended to sink the assigned 24 V load |
| Measuring encoders | `AM26LV32EIDR` 3.3 V receiver (input common mode −7…+7 V); R1–R4 120 Ω RS-422 terminators replaced by a 2.5 V mid-bias network for the RU3100 single-ended TTL inputs | Non-inverted A/B outputs intended for `GP8`–`GP11` |
| Field/alarm contacts | Ten protected dry-contact-to-0 V inputs, active low | `GP12`–`GP16`, `GP18`–`GP21` |
| Encoder power | Two individually resettable 5 V branches | One branch per ifm RU3100 encoder |

The firmware-facing signal names and polarities are defined in the
[controls specification](../../shared/controls_spec.hpp). The exact draft
connector-to-net mapping is in [if-b01-netlist.csv](if-b01-netlist.csv), and
the machine-readable cross-check is [if-b01-gpio-allocation.csv](if-b01-gpio-allocation.csv).
The allocation checker [check_if_b01_gpio_mapping.py](../../tools/check_if_b01_gpio_mapping.py)
reconciles the board connector CSV, native intent, controller pin constants,
cabinet relay schedule and RP2350 PIO setup. A label, connector position,
library symbol or historical board description must not be treated as a
substitute for a reviewed native schematic.

## Native-draft files presently available

| File | Meaning | What it is not |
|---|---|---|
| [if-b01.kicad_pro](if-b01.kicad_pro) | KiCad 10 project entry point | A board release |
| [if-b01.kicad_sch](if-b01.kicad_sch) | Editable logical schematic with embedded exact local symbols | A completed electrical design review |
| [if-b01.kicad_sym](if-b01.kicad_sym) / [if-b01.pretty](if-b01.pretty) | Project-local draft symbol and footprint libraries | Checked component land-pattern approval |
| [if-b01.kicad_pcb](if-b01.kicad_pcb) | Editable four-layer $750\times400\ \mathrm{mm}$ placed/routed draft PCB, including all board footprints, copper and four M3 holes | A released or fabrication-approved layout |
| [if-b01-native-intent.json](if-b01-native-intent.json) | Generated BOM, pin/net and release-hold manifest | A fabrication bill of materials |
| [review/if-b01-native-source-review.md](review/if-b01-native-source-review.md) | Interpretation of recorded source-level KiCad checks | A design-review approval or release record |
| [review/if-b01-erc.rpt](review/if-b01-erc.rpt) | KiCad 10.0.6 ERC result: zero reported violations | Human electrical review or qualification evidence |
| [review/if-b01-netlist.net](review/if-b01-netlist.net) / [review/if-b01-native-bom.csv](review/if-b01-native-bom.csv) | Native KiCad export evidence compared with the manifest | A released manufacturing export |
| [review/if-b01-pcb-drc.rpt](review/if-b01-pcb-drc.rpt) | KiCad 10.0.6 physical plus schematic-parity result: zero DRC violations, zero unconnected pads and zero parity issues | A human design review, fabrication approval or qualification result |
| [review/if-b01-pcb-prelayout-drc.rpt](review/if-b01-pcb-prelayout-drc.rpt) | Historical pre-layout record retained for traceability | The result for the current board |
| [if-b01-netlist.csv](if-b01-netlist.csv) | Intended connector, net and firmware-signal mapping | A netlist generated and checked by an EDA system |
| [if-b01-bom.csv](if-b01-bom.csv) | Intended electrical parts, packages and connector identities | A placed, approved or fabrication BOM |
| [if-b01-placement.csv](if-b01-placement.csv) | Generator-owned exact footprint coordinate schedule for the current board | An approved assembly instruction |
| [if-b01-assembly.md](if-b01-assembly.md) | Draft assembly/inspection procedure tied to current source outputs | A released work instruction |
| [if-b01-test.md](if-b01-test.md) | Fixture-only board test procedure | A production-machine commissioning procedure |
| [if-b01-cabinet-integration.md](if-b01-cabinet-integration.md) | Cabinet terminal, relay-contact, encoder and indication integration schedule | A cabinet wiring release or safety circuit |

| [manufacturing/draft-r2-2026-09-13](manufacturing/draft-r2-2026-09-13) | Verification-gated draft Gerber, drill, placement, visual assembly, board-only STEP, IPC-D-356, BOM and SHA-256 manifest package | A release package or authorisation to order/build |

The deterministic generator [generate_if_b01_kicad.py](../../tools/generate_if_b01_kicad.py)
reconciles the input BOM, connector allocation and fixed IC channels before it
writes the native source. The parity check
[check_if_b01_kicad.py](../../tools/check_if_b01_kicad.py) checks the actual
KiCad netlist and BOM exports: 100 BOM references, 90 electrically represented
netlist references, 278 connected symbol pins and six deliberate no-connects.
The [output generator](../../tools/export_if_b01_manufacturing.py) refuses stale
source or any ERC/DRC/parity violation before it writes the draft package. These
checks constrain transcription and layout errors; they do not replace
electrical, mechanical, PCB, fabrication or safety review.

## Fixed electrical allocation to preserve in schematic capture

| Array | Intended channel assignment |
|---|---|
| U2 `TPL7407LDR` | BF `PUL−`, BF `DIR−`, BR `PUL−`, BR `DIR−`, FL `PUL−`, FL `DIR−`, FR `PUL−` |
| U3 `TPL7407LDR` | FR `DIR−`, shared `ENA−`, green LED negative, red LED negative, buzzer negative |
| U3 unused channels | Inputs tied low; no field connection permitted |
| U1 `AM26LV32EIDR` | Left A, left B, right A, right B; only non-inverted receiver outputs go to the Pico |

The U2/U3 low-side outputs connect only to the named negative drive/load
terminals. The corresponding PUL/DIR/ENA positives are separately protected
`+24V_DRVLOGIC` cabinet wiring and must not be routed through IF-B01. The
IF-B01 `0V_PELV` pins are references/test returns, not a substitute for the
selected driver input wiring.

## Connector and protection intent

The currently named Phoenix Contact MSTB 5.08 mm headers and matching plugs in
[if-b01-bom.csv](if-b01-bom.csv) are draft selections. Exact fit, creepage,
clearance, conductor range, mating orientation, current capacity, availability
and assembly compatibility require schematic/PCB review.

| Ref | Positions | Draft function |
|---|---:|---|
| J1/J2 | 20 + 20 | Pico 2 socket rows |
| J3 | 4 | `+24V_PELV`, `0V_PELV`, `+5V_LOGIC`, `0V_PELV` |
| J4–J7 | 4 each | BF, BR, FL, FR: `PUL−`, `DIR−`, `ENA−`, `0V_PELV` |
| J8/J9 | 6 each | Left/right ifm RU3100 encoder: `+5V`, `0V`, `A+`, `A−`, `B+`, `B−` (single-ended A/B to the + terminals; A−/B− are the receiver mid-bias points, not encoder wires) |
| J10 | 12 | Six isolated field-input contact pairs |
| J11 | 8 | Four isolated driver-alarm contact pairs |
| J12 | 4 | `GREEN−`, `RED−`, `BUZZER−`, `0V_PELV` |

Each planned dry-contact input has a 10 kΩ series resistor, 47 kΩ pull-up,
10 nF filter and 3.6 V clamp on the board side. An isolated NO contact is
intended to close `SIG` to `0V_PELV`, producing a low asserted level. Each
encoder supply and the TPL `COM` feed are intended to pass through a Bourns
`MF-R050` resettable fuse. These values require schematic/ERC review and
physical testing; neither a CSV nor a firmware pull-up proves their behaviour.

For encoder cables, the intended order is the RU3100 single-ended allocation:
A and B to the J8/J9 A+/B+ terminals; the RU3100 has no A-bar/B-bar outputs,
so the A−/B− terminals are the receiver **mid-bias** points, not encoder
wires. Parameterise each RU3100 to **TTL mode** via IO-Link before wiring
(factory setting is HTL 50 mA). The Z conductor stays unconnected and
individually insulated (it doubles as the IO-Link line for bench
parameterisation). Bond the cable screen at the cabinet PE entry, never to
IF-B01 `0V_PELV`.

## Safety boundary

The board's `GP17` pull-down, controller watchdog, firmware safety interrupt,
USB transport and output arrays are supplemental control measures only. They
do not provide safe torque off, contactor isolation, safe discharge, guard
unlock permission or a personnel-safety function. See
[the cabinet control record](../../docs/22-controls-and-wiring.md) for the
proposed hardwired XPS/E-stop/guard/contactors topology and its unresolved
release holds.

## Fixture-only firmware and test boundary

Use the dedicated `if-b01-factory-test.uf2` image only on a bare IF-B01 fixture
with J4–J7 unplugged from production motor drivers. It deliberately has no
production `Controller` or step-motion PIO. It begins with every TPL input low,
requires active-low `SAFETY_OK` plus held active-low `RESET` before output
testing, limits arming to 30 seconds and limits a selected output to three
seconds. The exact procedure and command map are in
[if-b01-test.md](if-b01-test.md).

The production `starz-machine.uf2` image is not an individual-output test
tool. Do not extend it with fixture commands or use the factory image on an
assembled cabinet/machine.

## Local EDA toolchain status

Checked on 2026-09-13: `kicad-cli --version` reports **10.0.6**. The current
logical schematic returns zero KiCad ERC violations; native-netlist/BOM parity
passes against the generated intent manifest; and the routed board returns zero
physical DRC violations, zero unconnected pads and zero schematic-parity
issues. The output-pipeline manifest records SHA-256 values for every source and
draft output. KiCad parsing, ERC, DRC and parity cleanliness are not release
verdicts and must not be presented as a substitute for review or physical
evidence.

## Preconditions for a future board release

1. Complete human electrical and mechanical review of the logical capture and
	current layout, including the $750\times400\ \mathrm{mm}$ panel, actual
	component footprints, connector orientation, Pico socket geometry, protection
	values, TPL COM slew/bulk capacitance, cable use, return paths and stack-up.
2. Obtain fabricator confirmation of panel capability, finished copper/stack-up,
	drill, clearance, mask, silkscreen and assembly process constraints; revise
	the source if any requirement differs from the draft assumptions.
3. Inspect a fabricated bare board and build it only under a reviewed assembly
	process. Run the isolated fixture procedure and record all actual results.
4. Complete cabinet, DC-power, PE, guarding, EMC, safety-function and machine
	qualification evidence before permitting machine energisation.
