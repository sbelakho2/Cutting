# IF-B01 cabinet-integration schedule — draft

**Revision:** `S1-B-IF-B01-NATIVE-DRAFT-R2-2026-09-13`  
**Status:** controlled connection schedule only — **not** a cabinet wiring
release, fabrication drawing, safety circuit, energisation procedure or
qualification record.

This schedule imports the current IF-B01 connector allocation from
[if-b01-netlist.csv](if-b01-netlist.csv) and the cabinet functional boundary
from [the controls/cabinet design input](../../docs/22-controls-and-wiring.md).
It is backed by the controller-visible allocation in
[cabinet_controls_spec.hpp](../../shared/cabinet_controls_spec.hpp). It does
not select terminal hardware, conductor size/colour, torque, branch protection,
cabinet geometry or safe-energy behaviour; those are explicit release holds in
[selected-cabinet-wiring.csv](../../bom/selected-cabinet-wiring.csv).

## Power and reference connections

| IF-B01 terminal | Cabinet designation | Net | Connection boundary |
|---|---|---|---|
| J3-1 | TB24-01 | `+24V_PELV` | Protected board supply; F1 feeds the U2/U3 COM rail only. |
| J3-2, J3-4 | TB24-02 | `0V_PELV` | Board reference/return; capacity and routing require review. |
| J3-3 | TB24-03 | `+5V_LOGIC` | Board logic/encoder source through F2/F3. |
| J4–J7 PUL+/DIR+/ENA+ | TB-DL | `+24V_DRVLOGIC` | External common-anode drive input supply; it does not pass through IF-B01. |
| J4–J7 PUL−/DIR−/ENA− | Named driver input | low-side outputs | One labelled CL86T/CL57T drive per connector; no motor-power path is on IF-B01. |

## Field and alarm relay-contact schedule

Every J10/J11 input is an isolated dry-contact closure to its adjacent
`0V_PELV` return. Do not apply field 24 V directly to either connector.

| Relay | Cabinet source function | IF-B01 contact pair | GPIO / asserted state |
|---|---|---|---|
| KA1 | START PB01 NO | J10-1 to J10-2 | `GP12`, low while pressed |
| KA2 | STOP PB02 NC monitor | J10-3 to J10-4 | `GP13`, low while loop healthy |
| KA3 | RESET/ACK PB03 NO monitor | J10-5 to J10-6 | `GP14`, low while pressed |
| KA4 | XPS diagnostic, downstream of XPS 13-14 | J10-7 to J10-8 | `GP15`, low while energised; diagnostic only |
| KA5 | rear home sensor monitor | J10-9 to J10-10 | `GP16`, low at rear home |
| KA6 | front home sensor monitor | J10-11 to J10-12 | `GP27`, low at front home |
| KA7 | blade-front drive alarm monitor | J11-1 to J11-2 | `GP18`, low on alarm |
| KA8 | blade-rear drive alarm monitor | J11-3 to J11-4 | `GP19`, low on alarm |
| KA9 | feed-left drive alarm monitor | J11-5 to J11-6 | `GP20`, low on alarm |
| KA10 | feed-right drive alarm monitor | J11-7 to J11-8 | `GP21`, low on alarm |

## Encoder and indication connections

| IF-B01 connection | Cabinet endpoint | Required boundary |
|---|---|---|
| J8 | left ifm RU3100 encoder (TTL mode via IO-Link required; 2048 PPR set via IO-Link; factory setting HTL 1024 PPR) | `+5V` = M12-1 UB, `0V` = M12-3 GND, A = M12-2 → J8-3, B = M12-5 → J8-5; J8-4/J8-6 are the on-board 2.5 V receiver mid-bias nodes (R38/R39/R40/R41, C14/C15) and must not receive any encoder wire; Z/IO-Link (M12-4) insulated. |
| J9 | right ifm RU3100 encoder (TTL mode via IO-Link required; 2048 PPR set via IO-Link; factory setting HTL 1024 PPR) | `+5V` = M12-1 UB, `0V` = M12-3 GND, A = M12-2 → J9-3, B = M12-5 → J9-5; J9-4/J9-6 are the on-board 2.5 V receiver mid-bias nodes (R42/R43/R44/R45, C16/C17) and must not receive any encoder wire; Z/IO-Link (M12-4) insulated. |
| J8/J9 cable screen | PE-BAR at entry | Bond the screen at the cabinet PE entry, never to IF-B01 `0V_PELV`. |
| J12-1 | PB01 green LED cathode | Anode uses separately protected `+24V_PELV`; `GP22` controls the low side. |
| J12-2 | PB02 red LED cathode | Anode uses separately protected `+24V_PELV`; `GP26` controls the low side. |
| J12-3 | BZ01 negative terminal | Positive terminal uses separately protected `+24V_PELV`; `GP28` controls the low side. |
| J12-4 | TB24-02 | Reference/test return only. |

## Mandatory boundaries before cabinet work

1. Use the source-generated [IF-B01 output package](manufacturing/draft-r2-2026-09-13)
   only after its human electrical/mechanical and fabricator reviews are recorded.
2. The XPS, K1/K2/K3, E-stop and guard contacts remain hardwired functions;
   KA4/`GP15`, IF-B01 and firmware are diagnostic only.
3. **Guard-release-permission chain (desk part 2026-10-13, docs/22 §11.8).**
   The E1/E2 release coil is driven only through the dedicated chain below; the
   K1–K3 built-in mirror NC contacts stay exclusively in the XPS `Y1`/`Y2`
   monitored-start (EDM) chain and are **not** reused. The release chain uses the
   second, distinct NC contacts added as one Schneider `LADN02` 2-NC auxiliary
   block on each LC1D12BD (DC-coil LC1D09…D38 accept exactly one front block;
   a 1-contact LADN01 is not offered for them). Terminal-level definition:

   | Element | Terminal definition | Notes |
   |---|---|---|
   | Protected release branch | `+24V_PELV → CB-R2-LINK (1 A T 5×20) → TB-S group` | separate 1 A branch, not the XPS output fuse |
   | PB04 release request | `TB-S → PB04 NO (11-14) → K1 LADN02 NC-A` | momentary hold-to-release, panel button |
   | Contact monitor chain | `K1 LADN02 NC-A → K2 LADN02 NC-A → K3 LADN02 NC-A → T2 A1` (each contact's second terminal in series; second NC of each block left unwired) | block terminal numbers per the fitted block marking; record at the wiring review |
   | T2 on-delay | `T2 A1/A2 = 24 V / 0 V`; output NO `15-18` in the R2 coil path | set to the **measured** bus-decay time + ≥1 s (desk estimate ≈3 s with the 2.2 kΩ 5 W bleeds) |
   | R2 | `T2 15-18 → R2 A1`; `R2 A2 → 0 V`; R2 NO `11-14` → E1; E2 → 0 V | Phoenix `PLC-RSC-24DC/21` (identity CT-RC01) |
   | E1/E2 | Bernstein SLC lock coil | existing guard device; spring-retained lock on power loss |

   A welded contactor main pole keeps that contactor's LADN02 NC block open and
   blocks release; a welded LADN02 contact is a documented single-channel fault
   that the risk assessment must accept or escalate (proof test required). All
   bus-decay, timer-setting and fault-injection measurements remain physical
   holds (docs/22 §10 hold 5).
4. Do not authorise guard unlocking from this schedule alone: the chain above is
   a desk design with the physical verification and risk assessment still open.
   Until they close, access after a trip remains a controlled lockout/tagout
   procedure.
5. Before power, record actual terminal product, wire/cable identity, crimp and
   torque method, continuity/separation/PE results, protection coordination and
   device settings. No blank, expected or simulated result is acceptance evidence.