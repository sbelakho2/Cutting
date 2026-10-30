# Purchase-order requirement clauses (closing the conditional items)

Every conditional compliance item becomes an enforceable PO clause with a
receipt acceptance test. Attach the relevant clause to each RFQ/order.

## PO-1 Linear motion (AX-R01)
- Genuine HIWIN HGH20CA blocks + HGR20 rail, 400 mm, 2 blocks per set, with
  certificate of conformity and lot traceability.
- Rejection criteria: no COC, clone markings, or C/C0 not matching the HIWIN
  catalogue (C 17.75 kN / C0 27.76 kN). Receipt: P00-T2 dimensional + COC check.

## PO-2 Ballscrew (AX-S01)
- SFU1610, 10 mm lead, 500 mm, C7, with inspection certificate; journals
  machined per drawing: bearing seats 12h6 (fixed, BK12) and 10h6 (floating,
  BF12), coupling extension Ø8h7 x 18.
- Rejection: no C7 certificate, journal tolerance outside drawing, nut rating
  below 11.3 kN class. Receipt: P00-T2 + P01-T3.

## PO-3 Supports (BK12/BF12)
- BK12 (6001 bearing, 12 mm bore) + BF12 (6000 bearing, 10 mm bore) matched set;
  fixed side axial rating >= 9.4 kN (= max(5000 N proof, 7539.8 N stall) x 1.25).
- Receipt: bearing identity check + P01-T1 fit record.

## PO-4 Drivers and supplies
- CL86Y-V20 (head) and CL57Y-V20 (feed) at 24-50 VDC; supplies LE-350-48 (48 V
  7.3 A) and LE-350-36 (36 V). No 60 V supply on any V20 branch.
- Receipt: P11-T4 voltage/id setting records; bus stays <= 50 V (M34 bound).

## PO-5 Coupling (CP-01)
- R+W MK2/100/50/8/14 (or the current equivalent with bores 8H7/14H7), clamp
  screws to 4.5 Nm per the R+W operating instructions; peak torque figure
  supplied on the order confirmation.
- Receipt: P01-T5 alignment + 4.5 Nm torque record.

## PO-6 Encoders and wheels (W0/W1/D)
- ifm RU3100, 6 mm shaft, resolution set to 2048 PPR (via IO-Link) and recorded;
  push-pull 24 V output. ifm E60137 wheel 6 mm bore / 63.66 mm OD.
- Receipt: P04-T4 point-to-point + P06-T2 force/calibration.

## PO-7 Ventilation (TH-F01/TH-F02)
- Pfannenberg 11666102055 + 11760002055, 230 V 50 Hz, P-Q curve at 50 Hz and
  60 Hz supplied; spare filter pack included.
- Receipt: TS01-TS18 campaign.

## PO-8 Contactor (LC1D12BD)
- Schneider must supply the DC-13 rating at 48 VDC (>= 7 A) on the order
  datasheet page; otherwise a DC-13 rated alternative is selected.
- Receipt: P03-T1/T2 DC make/break.

## PO-9 Mechanism parts (waste sweep, chute sensor, output takeaway)
- 24 V actuator/relay class, 24 V optical sensor, powered takeaway; interfaces
  and acceptance in bom/phase-h-io-occurrences.csv; P08 acceptance tests.
