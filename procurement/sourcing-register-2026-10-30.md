# Sourcing register — exhaustive marketplace sweep (2026-10-30)

**Method:** direct fetches and `r.jina.ai` text-extraction proxy against
made-in-china.com, omc-stepperonline.com, lcsc.com,1688.com, alibaba.com
(direct + mobile), misumi-ec.com, hiwin.com. Every observation below is a
**direct product link** captured on 2026-10-30. Price classes:
**EXACT** = single purchasable price shown; **RANGE** = bounded offer with MOQ
(low end of a range can be a teaser - the credible bound column records what
this project can rely on at our quantities); **REJECTED** = teaser/unusable.

Access record (search boundary, U05): made-in-china.com fully accessible;
omc-stepperonline.com accessible via proxy (direct 403); lcsc.com search
accessible (detail pages return datasheets; stock prices render in JS);
1688.com login wall; alibaba.com captcha/410; misumi-ec.com 403; hiwin.com
product pages 404. 1688/Alibaba exact offers remain behind their login/captcha
walls and are requested in the RFQ set instead.

## 1. Cut-axis linear motion (AX-R01: HGR20 400 mm rail + 2x HGH20CA per axis)

| offer | price / MOQ | class | credible bound | direct link |
| --- | --- | --- | --- | --- |
| Huhai Bearing (Shandong; Diamond/Audited/Leading-factory) "Hgr20 HGH20ca 20mm linear guides, factory price" | US$2.50-10.00 / 50 sets | RANGE | unusable at qty 4 without negotiation; per-set bound ~$10 upper | https://huhaibearing.en.made-in-china.com/product/JTvrhkfMOcRV/China-High-Quality-20mm-Linear-Guides-Factory-Price-Hgr20-HGH20ca.html |
| Liaocheng Guanzhe (Manufacturer; RoHS cert) "Custom length HGR20 + HGH20CA/HGW20CC" | US$3.00-5.00 / 1 pc | RANGE | cut-length 400 mm rail+block set bound ~$5-10 | https://1e76264b50cf3b31.en.made-in-china.com/product/MUZpYPuFnKWR/China-Custom-Length-Flange-Guide-Block-High-Precision-Linear-Guide-Rail-HGH20ca-Hgw20cc-Hgr20.html |
| Shandong Sair (Diamond/Audited) "HGH20CA block / HGR20" | US$9.00-16.00 / 1 pc | RANGE | block-side bound; pair set ~$16-32 | https://sairguide.en.made-in-china.com/product/LQrRDsvjhJYF/China-Hot-Sale-Square-HGH20ca-Hgr20-HGH20-Linear-Guide-Rail-Block.html |
| Dongguan Pinsi (Diamond/Audited) "ORIGINAL HIWIN HGR20/HGH20CA/HGW20CC" | US$10.00-100.00 / 1 pc | RANGE | genuine-brand claim - authenticity/COC must be verified at order | https://linearmotion.en.made-in-china.com/product/qarUHvZDhYcL/China-Original-HIWIN-HGR20-HGH20CA-HGW20CC-Steel-Rail-Linear-guide-Linear-Rail.html |
| Zaozhuang Yiding "HGH20CA blocks" | US$1.00-3.00 / 10 pcs | REJECTED | teaser range for block class; no rail/set identity |

**Selection note (F06):** none of these pages publishes certified C0/C
ratings. The candidates must be verified against analysis/axis-rating-requirements
(C0 >= 7.5 kN, C >= 15.1 kN class) with supplier datasheets at order -
cloned "factory price" sets at $2.50 cannot be assumed to meet them.

## 2. Ballscrew SFU1610 C7 10 mm lead x 500 mm + flange nut (AX-S01)

| offer | price / MOQ | class | credible bound | direct link |
| --- | --- | --- | --- | --- |
| Yunze Guide "SFU1610 rolled C7, high lead, WITH flange nut" | US$23.90-29.90 / 1 set | RANGE | best exact-configuration C7 lead; 500 mm set bound ~$24-30 | https://yunzeguide.en.made-in-china.com/product/RJQrkHacWmTW/China-Sfu1610-High-Lead-Ball-Screw-Rolled-C7-Ball-Screw-with-Flange-Nut.html |
| Auto Linear Motion "Rolled C7 SFU1610 300/500/600/800 mm custom lengths" | US$5.00-10.00 / 1 pc | RANGE | length-configured C7; screw-only bound ~$5-10 (nut extra) | https://autolinearmotion.en.made-in-china.com/product/MEcpXnNyArhs/China-Rolled-C7-Sfu1610-Sfu2504-300mm-500mm-600mm-800mm-Custom-CNC-Lathe-Ball-Screw.html |
| Nanjing Technical (njyigong; ISO9001/45001/14001) SFU1610 rolled set | US$45.00-50.00 / 1 set | RANGE | full set with machining option; upper bound $50 | https://njyigong.en.made-in-china.com/product/nwRAlcfKaQhB/China-Top-Grade-High-Accuracy-Sfu1610-Rolled-Ball-Screw-with-Integrated-Ball-Drive-Structure-for-CNC-Machine-Tools-1605-Specification-Low-Friction-Cost-Effective.html |
| Yunze Guide "SFU1610 high-lead kit (screw+nut+BK/BF)" | US$10.00-18.00 / 1 set | RANGE | kit-class bound | https://yunzeguide.en.made-in-china.com/product/fJgUaSpOVmGd/China-Sfu1610-High-Lead-Ball-Screw-for-High-Speed-CNC-Router-Hiwin-Standard.html |
| Sair C5 ground SFU1605/1610 with nut | US$7.00-12.00 / 1 pc | RANGE | C5 ground class - check actual lengths at $7-12 (likely short) | https://sairguide.en.made-in-china.com/product/vFyaiHVuwZYw/China-Sfu1204-Sfu1605-Sfu1610-Precision-Ground-Ball-Screw-C5-Ballscrew-with-Ball-Nut-for-3D-Printer.html |
| Tianjin Haorongshengye "C3/C5/C7 SFU1610 + 2505 nut sets" | US$6.99-15.00 / 10 pcs | RANGE | nut-set bound | https://tianjinhrsy.en.made-in-china.com/product/RmCYOIGHagro/China-High-Quality-C3-C5-C7-Precision-Sfu1610-2505-Ball-Screw-Nut-Set.html |
| wgjxzgzz SFU1610 domestic meter supply | US$120.00 / 1 m | EXACT | $60/500mm class - most expensive route; not selected | https://wgjxzgzz.en.made-in-china.com/product/NmpryQiPbKcv/China-China-Sfu1610-Ball-Screw-Manufacturer-Supply-Domestic-Ball-Screw.html |

## 3. Screw supports BK12/BF12 pair (AX-S01)

| offer | price / MOQ | class | credible bound | direct link |
| --- | --- | --- | --- | --- |
| Shandong Sair "Bk12/Bf12 matched seat for SFU1604/1605/1610, 1 set" | US$2.50-4.00 / 1 pc | RANGE | lowest credible pair lead; ~$2.5-4 per seat | https://sairguide.en.made-in-china.com/product/vxRrIGpJBEUS/China-High-Precision-Bk12-Bf12-Ballscrew-Support-Match-Use-Sfu1604-Sfu1610-Sfu1605-Ball-Screw-End-Support-CNC-Part-1set-Bkbf12-Ball-Screw-Support-Seat.html |
| Zhejiang Jianzhuang (SHAC brand) "Bk/Bf any size incl. Bk12/Bf12" | US$6.00 / 1 pc | EXACT | $6/seat flat listing - verify it applies to Bk12 (flat-any-size prices are teaser-prone) | https://txpbearing.en.made-in-china.com/product/xGURwHpbOrWq/China-Shac-Ball-Screw-End-Support-Bk10-Bf10-Bk12-Bf12-Bk15-Bf15-Bk20-Bf20-Bk25-Bf25-Bk30-Bf30-Bk40-Bf40.html |
| Lishui Jialida (ISO9001:2015) "Bk10/12/20 fixed bracket with bearing" | US$6.50-23.00 / 1 pc | RANGE | bearing-equipped class bound | https://jialidalinearmotion.en.made-in-china.com/product/FaGUQyeoXdVE/China-Bk10-Bk12-Bf12-Bk20-Fixed-Bracket-Seat-Unit-Lead-Ball-Screw-End-Support-with-Bearing.html |
| HangZhou Well "Taiwan SYK class Bk/Bf" | US$10.00-50.00 / 1 pc | RANGE | branded-class bound | https://cntnkbearing.en.made-in-china.com/product/VaxrmObdZhWv/China-Taiwan-Syk-Ball-Screw-End-Support-Bk-Bf-Any-Size-Bk10-Bf10-Bk12-Bf12-Bk15-Bf15-Bk17-Bf17-Bk20-Bf20-Bk25-Bf25-Bk40-Bf40-for-Tbi-Hiwin-Ballscrew-1605-2010-2505.html |

## 4. Motors and drivers (DR-M01/DR-02/TR-D02)

| offer | price / MOQ | class | notes | direct link |
| --- | --- | --- | --- | --- |
| STEPPERONLINE 34E1K-85 NEMA34 closed-loop motor + 1000PPR encoder | **US$76.21** (tier as-low-as **$65.02**) / in stock | EXACT | full datasheet on page; specs 8.5 Nm, 0.54 ohm, 5 mH, 1.8 deg | https://www.omc-stepperonline.com/p-series-nema-34-closed-loop-stepper-motor-8-5nm-1203-94oz-in-with-encoder-1000ppr-4000cpr-34e1k-85 |
| 34E1K-85 full datasheet (PDF) | - | SPEC SHEET | exact spec source for the axis calculations | https://files.omc-stepperonline.com/34E1K-85_Full_Datasheet.pdf |
| STEPPERONLINE CL86Y-V20 hot-rated head driver | US$44.88 | EXACT (2026-10-29 obs) | row on the CL57Y-V20 page table; hot-qualification open | https://www.omc-stepperonline.com/y-series-v2-0-closed-loop-stepper-driver-0-7-0a-24-50vdc-for-nema-17-23-24-stepper-motor-cl57y-v20 |
| STEPPERONLINE CL57Y-V20 hot-rated feed driver | US$33.52 | EXACT (2026-10-29 obs) | in stock | same page as above |
| TI TPL7407LDR (LCSC C139663) sink driver | price in JS cart | SPEC SHEET | full TI datasheet captured (600 mA/ch, 40 V, SOIC-16) | https://www.lcsc.com/product-detail/C139663.html |

## 5. Notes for the cost roll-up (honest treatment)

- RANGE offers are bounded leads, not verified quotes: the ledger uses the
  credible bound column, never the teaser low end, and flags every bound with
  its quantity/MOQ constraint (Huhai 50-set MOQ is unusable at our qty 4).
- "Original HIWIN" claims require authenticity verification (COC/traceability)
  at order; clones at $2.50/set cannot be assumed to meet the C0/C criteria.
- 1688/Alibaba exact offers could not be read (login/captcha walls); the RFQ
  set requests firm quotes there. Everything observable is registered above
  with direct links.
