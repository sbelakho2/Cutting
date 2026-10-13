# Weld and assembly procedures — payoff and guard (S1-B)

**Revision:** `S1-B-WA-R1-2026-09-16`
**Status:** fabrication procedures for the internal build. Each step names
the fixture, the process, and the inspection criterion; record results in
the build record.

## 1. Payoff post-to-base weld (P004/P005 → P003)

1. **Fixture:** a weld fixture holding both posts with a Ø20 gauge pin
   through both finished Ø20.5 spindle bores and Ø47 gauge plugs in both
   bearing pockets, so the two pocket axes are coaxial during welding.
2. **Preparation:** degrease; root gaps ≤0.5 mm; tack both posts at the
   four corners with 1 mm fillets; re-check the gauge pin slides freely.
3. **Process:** MIG, ER70S-6, 8 mm fillet, alternating sides and posts to
   limit distortion; let each pass cool below hand-hot.
4. **Inspection:** coaxiality of the two Ø47 H7 pockets ≤0.1 mm on the
   fixture after welding; perpendicularity of the spindle axis to the
   base top ≤0.15/300; no cracks/undercut; paint the completed assembly.

## 1b. P008 drive boss (do this during §2 welding)

Weld the P008 boss (Ø54.6 × 12, 20 H7 bore, 6 JS9 keyseat) to the **left**
flange's inner face, inside the hub tube. The keyseat must line up with the
spindle keyseat when the boss bore is slid over the spindle: check it with
the key before welding. The spool must then turn with the spindle: if it can
spin freely on the shaft, the brake does nothing — refit. Right flange keeps
its plain Ø20.5 pilot bore.

## 2. Payoff spool weld (P001 flanges → hub tube)

1. **Preparation:** tube ends square; flanges deburred; the hub passes
   through both flange Ø60 bores per the drawing.
2. **Process:** MIG 4 mm fillet all around both flange/tube joints,
   alternating quadrants.
3. **Inspection:** flange runout ≤1.0 mm at Ø400; hub concentricity to
   the Ø20.5 flange bores ≤0.5; visual weld quality; balance by adding
   no extra material — mark the heavy point if runout is marginal.

## 2b. Blade backing (H007)

Before fitting the blades: slide the H007 backing plate (30 × 46 × 8) into
the slot behind each blade seat in H004. Set the blade against the backing
with shim stock to the 20 µm face-gap target from docs/36 P8, then lock the
blade screws. The backing carries the cutting force; the blade must not be
the part that deflects.

## 3. Guard jamb plate (G106) and door frame

1. **G106:** weld the 5 mm plate to the door jamb (2 × 40 mm fillets per
   side); drill the SLC mounting pattern only from the Bernstein
   installation drawing (docs/26 §2).
2. **Door frame:** mitre and weld the 20×20×2 tube frame; diagonal
   check ≤2 mm; glaze the 5 mm polycarbonate panel into the frame with
   mechanical fixing and sealant; door weight ≤15 kg before hinge fit.

## 4. Final assembly checks (repeat after each step of §3.1–3.9 of the guide)

- Wire path: 1.2 and 4.5 mm taut-wire pass, zero unintended contact.
- Guard: door swing full travel without panel contact; SLC/ACF-1
  insertion, alignment and locking travel per the Bernstein manual.
- Payoff: spool end float 0.1–0.3 mm; free rotation; brake adjustment
  trial (docs/26 §3).

All inspections above feed the blank records in
[`docs/inspection/`](inspection/) and the first-article procedure in
[docs/20](20-physical-qualification.md). No step here substitutes for the
risk assessment or the physical qualification of the guard function.
