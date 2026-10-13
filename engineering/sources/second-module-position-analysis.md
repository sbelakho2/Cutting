# Second measuring station — position and validity analysis (audit F06)

**Revision:** `S1-B-MS2-R1-2026-10-13`

## 1. Why not the piece side (the audit's core point, proven)

A measuring wheel is a disc whose rim line contacts the wire at the wheel
**centre** X. Contact therefore requires wire or part material **at** the
wheel-centre X at the moment of measurement.

- A finished part of length L occupies X = 0…L during backfeed and pull.
- The guard panel inner face is at X = ±345; the piece cassette ends at
  X = +168; a wheel needs ±31.83 mm around its centre and ~23 mm of
  housing each side.
- A piece-side wheel fully outside the cassette needs centre ≥ 168 +
  31.83 = 199.83; a wheel fully inside the guard needs centre ≤ 345 −
  31.83 = 313.17.
- For the shortest part, L = 200 mm: the stock tip reaches X = +200 once
  during the feed, so the wheel centre must be < 200 to ever touch it —
  leaving a 0.17 mm window at the cassette edge, and after the cut the
  part's tip only moves back (backfeed) so the window closes.

**Conclusion: a piece-side second station is geometrically infeasible for
200 mm parts with the current cassette/guard layout.** The audit's warning
was correct; the "mirror at +205" idea is withdrawn.

## 2. Selected architecture: two stock-side stations

The length measurand is the **stock travel between cut events** — the wire
is continuous from the payoff to the cut plane, so a wheel on the stock
side has a guaranteed contact window for every part length (200–3000 mm),
every strip combination and every phase (feed, backfeed, pull), because
the stock is always present at any X < 0 while any material is fed.

- **Station 1 (existing):** wheel centre X = −205, bracket outboard at
  X = −306…−232.
- **Station 2 (new):** wheel centre X = **−385**, same facing, bracket
  outboard at X = −486…−415, entirely outside the guard panel (X = −347),
  clear of the payoff after its 60 mm outboard shift (spool rim now at
  X = −510). Continuous stock contact for all phases; the wire envelope
  (X −440…+440) passes the wheel tangent (0 mm³ common volume, checked).
- The two wheels are 180 mm apart, mechanically independent (separate
  M201/M202/M203/M204/M206/M210 chains, separate 626 bearings, separate
  RU3100 encoders, separate supplied couplings). Two independent
  measurements of the same stock travel give slip and fault redundancy:
  a slipping or damaged wheel shows as a divergence between counters
  over a common interval.

## 3. Consequent changes

- **Payoff shift:** the payoff assembly moved 60 mm outboard (spool centre
  X = −710, base −960…−530, posts −745…−675, caliper −760…−660) to clear
  station 2's bracket foot. This is a pure X translation; the spindle,
  brake, guards and floor anchors are unchanged in form.
- **CAD:** mechanism R4 now models **547 occurrences** (both stations,
  43 parts each). All gates re-run: solids valid, zero interference in
  every group pair, wire envelope clear, nip/wheel-force windows intact.
- **Controller comparison contract (implemented in the controller
  workstream):** both counters are compared over the common observed
  stock-travel interval; the residuals must agree within the slip budget
  after the bounded correction, and the final residual must meet the
  recipe length budget (0.20 mm + 0.05 %·L) or the piece is faulted and
  not counted. Piece-side validity windows are no longer part of the
  architecture.
- **Validity rule (stated for the firmware/commissioning record):** a
  station's count is valid while its phase-gated contact flag is set
  (stock present and moving, no blade/strip motion between the station
  and the cut plane during the measured interval — both stations sit
  upstream of the cassette, so the strip motions at the head do not
  intervene during feed measurement).
