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
| J8 | left ifm RU3100 encoder (REVIEWED 2026-09-16: TTL mode via IO-Link required; factory setting HTL) | `+5V` = M12-1 UB, `0V` = M12-3 GND, A → J8-3, B → J8-5; A−/B− (J8-4/J8-6) are the receiver mid-bias points; Z (M12-4) insulated. |
| J9 | right ifm RU3100 encoder (REVIEWED 2026-09-16: TTL mode via IO-Link required; factory setting HTL) | `+5V` = M12-1 UB, `0V` = M12-3 GND, A → J9-3, B → J9-5; A−/B− (J9-4/J9-6) are the receiver mid-bias points; Z (M12-4) insulated. |
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
3. Do not authorise guard unlocking. The E1/E2 release-permission circuit is
   deliberately unresolved pending the safety and energy evidence named in
   the cabinet design input.
4. Before power, record actual terminal product, wire/cable identity, crimp and
   torque method, continuity/separation/PE results, protection coordination and
   device settings. No blank, expected or simulated result is acceptance evidence.