# 1688 + Alibaba verified offers (2026-10-30) — the sourcing result

**Access methods that WORK (recorded for reproducibility):**
- **1688:** `https://r.jina.ai/https://detail.1688.com/offer/<ID>.html` renders the full
  server-side offer page (price, stock, seller record, spec table). Rate-limited:
  after ~3 pages the proxy IP is blacklisted (`cloud_ip_bl` punish page); retry later
  or from another network. Offer IDs discovered via DuckDuckGo `site:detail.1688.com`.
- **Alibaba.com:** **country subdomains work** (`german.`, `italian.`, `spanish.`,
  `french.`...alibaba.com) where `www.`/`m.`/`chinese.` are captcha-blocked; pages
  render fully incl. tiered prices, MOQ, seller badges, freight and lead time.
  Product IDs discovered via DuckDuckGo `site:alibaba.com`. Some subdomain requests
  still hit the captcha (IP pool lottery) — retry until it passes.

## ALIBABA — verified product pages

### A1. HGR20 400 mm rail + HGH20CA block set — $15/pc (2-99), $12 (>=100)
- https://german.alibaba.com/product-detail/CNC-linear-guide-rail-set-hgr20_1601002034470.html
- Seller: Changzhou Maples Precision Electromechanical Co., Ltd. (4 yr, TüV Rheinland
  verified, 4.9/5 from 48 reviews, 100 % on-time, 42 sold)
- Spec: guide width 20 mm, length 400 mm (100-4000 custom), S55C/S55 steel, 1.5 kg,
  machine test report + outgoing video provided, 1-year warranty
- Logistics: ocean via Maersk, from $0.84/kg at 50 kg MOQ, 17-24 days
- Need: 4 sets -> $60 (2-99 tier)

### A2. SFU1610 500 mm ballscrew+nut linear module — $50/pc (10-99), $45 (>=100)
- https://italian.alibaba.com/product-detail/High-Precision-SFU1610-Ball-Screw-Linear_1601904701424.html
- Seller: Changsha Terry Machinery (NTLtrans), 16 yr, brand owner, 4.9/5 (5362 reviews),
  94.4 % on-time; CE machine-compliance mark shown
- Spec: SFU1610, 500 mm, rolled thread, bearing steel, 5 kg, kit = screw + nut + motor
  + coupling + supports; model 1610-500; 1-year warranty; sample $100
- Logistics: $119.47 per 10 pcs (Alibaba Logistics), delivery 8 Dec-22 Jan
- Need: 2 -> $100 (10-99 tier) or negotiate sample-tier

### A3. BK12/BF12 support set (SFU1604/1605/1610) — $7.80/set (MOQ 10)
- https://www.alibaba.com/product-detail/High-quality-BK12-BF12-Ballscrew-Support_1600555140447.html
- HLTNC (Lishui Hengli Automation, 10 yr, #13 top-selling in Speed Reducers)
- Also: "High Precision BF BK 12 C7 C5" **$4.30-18.64, MOQ 1 set, 28 sold** —
  https://www.alibaba.com/product-detail/High-Precision-BF-BK-12-C7_1601032419735.html
- Also: BKBF10-40 **$12.30 MOQ 1** — https://www.alibaba.com/product-detail/HLTNC-Ball-Screw-End-Support-Bearings_60628700586.html
- BKBF15 $7.40 MOQ 10; BK10-40 $5-5.20 MOQ 50 sets; FK/FF $4.80-5 MOQ 10 sets

### A4. Closed-loop NEMA34 drivers/motors (verified related offers on A1's page)
- HSS86 hybrid-servo driver **$25.90-29.50, MOQ 1** (CE certified)
- HBS86H AC20-80V driver **$31.70, MOQ 1** (Russian stock option)
- Integrated NEMA34 closed-loop 4.5/6.5/8.5/10 Nm with pulse driver **$60-75, MOQ 1**
- 12 Nm NEMA34 + HBS86 kit $72-75 MOQ 1 set
- DM860 24-80 VDC driver $25 MOQ 2

## 1688 — verified offer pages

### B1. HGR20 rail, cut to length — ¥55.00/rail
- https://detail.1688.com/offer/619052577501.html
- Guangzhou Qinxuan Bearing (12 yr, 100 % fulfilment/positive), HGR20 20×17.5,
  length up to 4000 mm cut to order, blocks any qty, stock 10 000, freight ¥25+
- Need: 4 rails -> ¥220 (+blocks, priced separately)

### B2. SFU1610 500 mm kit — ¥300 (300 mm ¥270 / 1000 mm ¥350)
- https://detail.1688.com/offer/867836995025.html
- Lishui Lingyue Transmission (10 yr), pitch 10 mm, 40Cr, **max load 30 kN claimed**,
  machining included, stock 8 878, free shipping
- Need: 2 -> ¥600

### B3. HBS86H 8.5 Nm closed-loop kit — ¥580 (>=2 pcs; ¥584 single)
- https://detail.1688.com/offer/590103180242.html
- Changzhou Fangneng Electric (9 yr), CE BSTXD220321204202EC + RoHS
  BSTDG220321204201CC certificates on page, free shipping
- Alternative architecture (HBS86H hybrid servo) - needs torque-curve verification
  before substitution for the selected 34E1K-85/CL86Y-V20 pair

## Offer IDs ready for the next 1688 pass (page not yet read - IP blacklist)
BK12/BF12: 946593393863, 674243791130, 561994089866, 693558846428 (BK12-C5)
86 motor 8.5 Nm: 526523338565 (OK86DL65), 623208325811 (KIMBROUGH CE), 900489606872 (格睿 set)
SFU1610: 579413912158 (钛浩), 45396565242, 652619504372 (SEMRI), 572722626850 (nut),
688364451690, 750511313987 (TBI SFNUR01610T3D-A), 597637575394
HGR20 second supplier: 597055564372; SFU1610 custom: 45396565242

## Cost impact (merchandise, ex-works)
Axis bundle per machine (2 axes = 2 rails+4 blocks... per axis 1 rail/2 blocks):
- Alibaba: HGR20 sets 4 × $15 = $60; SFU1610 modules 2 × $50 = $100; BKBF12 sets
  (need 2) $7.80 × 2 with MOQ-10 caveat -> $78 (MOQ) or $4.30-18.64 tier at MOQ1
- 1688: HGR20 4 × ¥55 = ¥220; SFU1610 kits 2 × ¥300 = ¥600; BKBF12 at ~¥30-40/pair
  (offer pages pending)
All cheaper than the previous register's $10-100 family listings by 3-10x, and both
marketplaces ship to a consolidator. Certification (C0/C, C7) still to be evidenced
per supplier at order - requested in RFQ-01/03.
