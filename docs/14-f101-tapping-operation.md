# F101 R6 — selected blind-hole tapping tool and setup

This operation supplements the [F101 shaft drawing](../engineering/generated/drawings/F101-R6-shaft.pdf), [machining detail sheet](../engineering/generated/drawings/F101-R6-details.pdf) and [cartridge manufacturing record](12-feed-cartridge-fabrication.md). It does not change the shaft dimensions or authorize machine operation.

## 1. One selected tool

**Dormer Pramet E412M5, material ID 7441118**, Yellow Shark, M5×0.8-6H. Purchase quantity **2**: one working tap and one spare; these are fabrication tools, not installed machine parts. See [tooling BOM](../bom/fabrication-tooling.csv).

The [manufacturer's exact product record](https://dormerpramet.com/p/7441118), checked 2026-09-12, specifies:

| Property | Manufacturer value |
|---|---|
| Construction | HSS-E-PM, TiAlN-Top coating, 3 flutes, 48° spiral |
| Standard | DIN 371 |
| Cutting lead | Form C, 2–3 pitches; 1.6–2.4 mm for this pitch |
| Blind-hole application | 3×diameter = 15 mm |
| Pre-drill | Ø4.2 mm |
| Reinforced shank / square | Ø6 mm / 4.9 mm; square length 8 mm |
| Overall length | 70 mm |
| Usable length LU | 25 mm |
| Threaded cutting length THL | 8 mm; **not** the blind-thread depth limit |

The [family dimension image](../engineering/sources/dormer-e412-family-dim.png) distinguishes the reduced-neck configuration for sizes through M10. Its illustrated male centre is not dimensioned. Do not scale that schematic to manufacture or program a tip allowance. The [transcribed interface record](../engineering/sources/dormer-e412m5-interfaces.csv) separates catalogue values from that missing tip dimension.

The selected 3D tool covers the maximum 14.27 mm distance from the shaft end to the last required full thread. Dormer E003M5, material ID 5974394, is **rejected**, not an alternative: its manufacturer's 2.5D application is only 12.5 mm.

## 2. Hole and physical-tip setting

Keep the F101 R6 dimensions unchanged:

- M5×0.8-6H, **14.00–14.05 mm of full-form thread after the actual C0.20±0.02 entry**.
- Ø4.20+0.03/0 drill, **18.00±0.05 mm cylindrical depth** from the shaft end, followed by the 118°±2° point.
- Do not count the entry chamfer, incomplete tap lead or conical drill point as full thread.

The extreme full-thread end coordinate is 0.22+14.05 = **14.27 mm**. With the maximum 2.4 mm cutting lead, 17.95−14.27−2.4 = **1.28 mm** remains in the cylindrical drilled portion. This is the **cutting-lead allowance**, not automatically the clearance to the tap's foremost physical point: a projecting male centre may consume part of it.

Before a tool is used, measure **a**, the axial distance from its foremost physical point to the first complete full-form cutting section. Include the male centre, cutting lead and measurement uncertainty. For this operation, require the guarded upper bound **a≤3.18 mm**. Then the deepest permitted physical point is 14.27+3.18 = **17.45 mm**, leaving **at least 0.50 mm** before the minimum 17.95 mm cylindrical-drill bottom. No clearance credit is taken for the conical drill point. The 25 mm usable length leaves 7.55 mm to the reinforced-shank transition at that insertion. The qualified process must include tool-offset uncertainty and reversal overtravel in this limit.

This is a **STARZ incoming-tool acceptance requirement**, not a claim that Dormer publishes a 3.18 mm tip dimension. Record the actual tool measurement. Do not regrind the selected coated tap or silently program the 2.4 mm lead as the physical tip length. A tool that fails the measured criterion must not be used for this hole.

Set the tapping stroke from the **measured physical tip**, actual entry chamfer and actual full-form tool section. Prove it first in a same-lot material coupon, and gauge the result. Nominal catalogue length alone is not a CNC tool-length offset. No tapping cycle or rigid-tapping capability has been assumed for the available CNC.

## 3. Material and process checks

The shaft remains certified 42CrMo4+QT, yield≥650 MPa. That minimum yield value does **not** establish hardness or tensile strength. The material certificate and hardness check must place the lot inside the selected tool's documented steel range before use; no hardness result is prefilled.

For reference, the manufacturer's P3 alloy-steel table gives starting cutting speeds 24 m/min below 180 HB, 19 m/min for 180–260 HB and 16 m/min for 260–360 HB. These are source values, not an approved CNC cycle or a substitute for the actual material lot and tapping setup. Feed must follow the 0.8 mm pitch, with synchronized reversal; do not apply a drilling feed to a tap. Fixture the shaft without marking its matched bearing seats, and keep chips away from the completed key/washer pockets.

Inspect the minor-hole diameter and **cylindrical** depth before tapping. After tapping and cleaning, inspect full-form depth and 6H GO/NO-GO acceptance. The depth gauge's own incomplete lead must not be counted as full thread. Record the actual entry chamfer, tool identifier, tip measurement, offset, coupon result and final thread inspection in the [blank F101 inspection record](inspection/F101-R6.csv). A passing CAD section or a generic thread gauge inserted 14 mm is not evidence of 14 mm of complete thread after the chamfer.

## 4. Price and source integrity

[MRO Supply's exact listing](https://www.mrosupply.com/cutting-tools/6081909_e412m5_dormer-pramet/), SKU 6081909, displayed **USD 40.55 each** on 2026-09-12; two tools give **USD 81.10 merchandise**. Its “typically ships in 1 day” statement is a dispatch estimate, not confirmed stock or delivery to Tunisia. The free-shipping banner is not a Tunisia freight quote. No purchase, account or supplier contact was made. Duty/tax is budgeted zero only under the user's stated sous-douane premise.

The retailer's structured specification table contains displaced fields, including an incorrect 48 mm pre-drill and 4.9 mm shank. **Use the manufacturer values Ø4.2 and Ø6**, corroborated by the retailer's prose, not those erroneous fields. The linked [2020 unified catalogue](../engineering/sources/dormer-unified-catalogue.pdf) was checked and does **not** tabulate E412/7441118; it is an archived research document, not dimensional authority for this tool.

Physical tool-tip acceptance and the actual machining process remain unsigned. This operation record does not clear the shaft's separate stress, fit-temperature or joint-preload release holds.