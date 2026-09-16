# Preliminary guard and payoff CAD accommodation — S1-B

**Revision:** `S1-B-GP-CAD-R1-2026-09-13`  
**Status:** static CAD accommodation, superseded for fabrication by the
[R2 selection record](26-guard-and-payoff-selection.md) and the
[fabrication guide](31-fabrication-guide.md). Not a guard design or a risk
assessment.

**R2 supersedes the accommodation state:** the
[guard/payoff selection record](26-guard-and-payoff-selection.md) adds explicit
material, spindle, bearing, spool and brake selections on top of this
geometry. This R1 record remains the authority for the current published
full-machine CAD until the R2 source changes are built and checked.

This record makes the previously absent guard/payoff volume explicit in the
full-machine assembly while retaining every physical and safety release hold.
The CAD source is [mechanism geometry](../engineering/src/mechanism_geometry.cpp),
the controlled dimensions are in [mechanism specification](../shared/mechanism_spec.hpp),
and [the full-machine builder](../engineering/src/mechanism_cad.cpp) validates
their static solids, intersections and straight wire route. `status.json`
continues to report `full_machine_integrated:false`, `physical_results:false`
and `fabrication_released:false`.

## 1. What the source models

The enclosure is a closed-door, $5\ \mathrm{mm}$-thick **geometric** shell
outside the current machine envelope. It consists of two side panels, rear
panel, roof and front-door envelope. The shell has nominal $30\ \mathrm{mm}$
diameter wire pass-throughs at $Y=0$, $Z=112\ \mathrm{mm}$. Those apertures
are CAD clearance only: they are not a finger-safe opening, labyrinth,
containment feature or safety-distance calculation.

| CAD IDs | Modeled item | Controlled source status |
|---|---|---|
| `G101`, `G102` | Left/right side panel envelopes with wire pass-through | Custom geometry concept; material, edge treatment, mounting and safety-gap design unselected |
| `G103`, `G104`, `G105` | Rear panel, roof and closed front door envelopes | Custom geometry concept; hinge, latch, visibility and retained-part behavior unselected |
| `G106` | Fixed-jamb lock mounting-plate envelope and pocket | Custom geometry concept; drilling, retention and load path unselected |
| `G107` | Bernstein SLC-F-024-20/20-R4 body envelope | Exact product identity selected; only the listed external dimensions are modeled |
| `G108` | Bernstein ACF-1 actuator envelope | Exact product identity selected; only the listed external dimensions are modeled |
| `P001`, `P002` | 400 mm OD × 160 mm provisional spool and 39 mm spindle envelopes | Explicitly `UNSELECTED`; not a commercial spool, spindle, bearing or brake selection |
| `P003`–`P005` | Base and two support-post envelopes | Explicitly `UNSELECTED`; no material, anchor, stiffness or load-rating claim |

The SLC body occupies a $30\times192\times44\ \mathrm{mm}$ bounding box,
and the ACF-1 actuator occupies a $60\times40\times52.6\ \mathrm{mm}$
bounding box in the current coordinate orientation. Their body/door pockets
include $0.5\ \mathrm{mm}$ geometric clearance. The two envelopes are
deliberately statically separated: this does **not** demonstrate actuator
engagement, locking travel, tolerances, mounting-hole compatibility or door
kinematics.

## 2. Direct-source evidence used for external envelopes

| Item | Direct product page observed 2026-09-13 | CAD use and limitation |
|---|---|---|
| Bernstein SLC-F-024-20/20-R4, article 6018200007 | [Automation24 product page](https://www.automation24.com/mechanical-switch-with-guard-locking-bernstein-slc-f-024-20-20-r4-6018200007): $30\times192\times44\ \mathrm{mm}$; EUR 143.50 plus VAT/dispatch; dispatch next working day | Uses an external bounding box only—not vendor mounting holes, cable bend, insertion path or application approval |
| Bernstein ACF-1, article 3911742391 | [Automation24 product page](https://www.automation24.com/actuator-bernstein-acf-1-3911742391): $60\times40\times52.6\ \mathrm{mm}$, stated 400 mm minimum actuation radius; EUR 17.90 plus VAT/dispatch; dispatch next working day | Uses an external bounding box only—not mounting-hole geometry, angular misalignment or engagement travel |

These are one-store public observations, not landed quotations or a lower-price
comparison. The [sourcing evidence register](../bom/sourcing-evidence.csv)
states the exact commercial scope and outstanding evidence.

## 3. Source-level static checks

For both specified wire diameters (1.2 mm and 4.5 mm),
`starz_mechanism_cad --check` verifies:

1. every newly modeled custom solid is valid and has positive volume;
2. exact Boolean intersections for every preliminary guard/payoff pair and
   every other group pair implemented by `starz_mechanism_cad`; the three
   head/cassette placement pairs are checked separately by `starz_layout_cad`;
3. a straight $880\ \mathrm{mm}$ wire envelope from $X=-440\ \mathrm{mm}$ to
   $X=+440\ \mathrm{mm}$ at the specified wire axis, including the two side
   pass-throughs; and
4. a negative regression: `--reject-guard-wire-route` removes the side-panel
   pass-throughs and must fail because the wire envelope intersects a panel.

The root source check `tools/check_mechanism_outputs.py` separately rejects a
published mechanism artifact if its revision, 507 occurrence count, static
guard/payoff IDs, generated-BOM classifications or unreleased status fields
drift from this R2 source definition.

Those checks establish only the modeled static geometry. They do not establish
cutting containment, human access distance, structural deflection, vibration,
door travel, spool rotation, tension, overspeed, wire payout path, braking,
emergency access or any legal safety requirement.

## 4. Required work before any fabrication or use

The following order is mandatory and must be recorded using [the blank guard/
payoff release template](inspection/guard-payoff-release-template.csv):

1. Produce a task-specific risk assessment covering cutting, wire release,
   entanglement, ejection, access, maintenance, loading, loss of power and
   emergency egress.
2. Obtain controlled manufacturer mounting/install documentation for the exact
   SLC body and ACF-1, select hinge/latch/fastener/panel/frame materials and
   design the actual mount/load paths. Confirm tolerances through a physical
   fit check; do not drill from a bounding box.
3. Select a real spool, spindle, bearings, retaining features, brake/tension
   method, guarding and anchors. Calculate and physically validate static,
   rotating, runaway and wire-break loads before fabricating `P001`–`P005`.
4. Complete the independent hardwired guard-contact, stop-time, energy-removal,
   DC-bus decay and E1/E2 release-permission work specified by
   [controls and wiring](22-controls-and-wiring.md). This CAD document does not
   authorize any unlocking circuitry.
5. Rebuild and inspect the actual assembly, test full door/payout movement and
   retain raw inspection and qualification records. A passing CAD check cannot
   close any of those physical gates.

The associated custom items are deliberately listed as non-fabricated concepts
in [the mechanism custom-parts BOM](../bom/mechanism-custom-parts.csv). The
master sequencing and first-article gate remain in [the production/release
plan](24-production-and-release-plan.md).