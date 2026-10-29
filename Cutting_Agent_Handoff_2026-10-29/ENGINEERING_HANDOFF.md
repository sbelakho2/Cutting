# CUTTING: COMPLETE ENGINEERING TASK FOR THE COMPLETION AGENT

**Handoff date: 29 October 2026. Owner: Sadok Belakhoua. Repository: https://github.com/sbelakho2/Cutting.**

**Read this as an instruction to finish the machine, not as a request for another audit.** The handoff agent is supplying Markdown and supporting records only. **The completion agent, not the handoff agent, must produce the one final visual assembly PDF.** No PDF in this handoff is required or intended.

**Navigation:**

- [0. The actual assignment and the definition of success](#0-the-actual-assignment-and-the-definition-of-success)
- [1. Source, recovery and evidence: know what actually exists](#1-source-recovery-and-evidence-know-what-actually-exists)
- [2. Requirements register: preserve every original goal](#2-requirements-register-preserve-every-original-goal)
- [3. Fresh reanalysis: fix these interactions first](#3-fresh-reanalysis-fix-these-interactions-first)
- [4. Earlier redesign candidates to reconstruct, challenge and verify](#4-earlier-redesign-candidates-to-reconstruct-challenge-and-verify)
- [5. Payoff and reel handling: use the photos thoughtfully](#5-payoff-and-reel-handling-use-the-photos-thoughtfully)
- [6. Mandatory standard for every single part and interface](#6-mandatory-standard-for-every-single-part-and-interface)
- [7. Cost, sourcing and the 70% target](#7-cost-sourcing-and-the-70-target)
- [8. Manufacturing, assembly and service design standards](#8-manufacturing-assembly-and-service-design-standards)
- [9. Universal simulation and evidence standard](#9-universal-simulation-and-evidence-standard)
- [10. The mandatory simulation and analysis programme: 48 work packages](#10-the-mandatory-simulation-and-analysis-programme-48-work-packages)
- [11. Physical verification and qualification: executable, instrumented and honest](#11-physical-verification-and-qualification-executable-instrumented-and-honest)
- [12. Accuracy, statistical confidence and reliability claims](#12-accuracy-statistical-confidence-and-reliability-claims)
- [13. Embedded, I/O and screenless-operation implementation contract](#13-embedded-io-and-screenless-operation-implementation-contract)
- [14. Standards, safety and acceptance governance](#14-standards-safety-and-acceptance-governance)
- [15. Ordered execution plan: turn this handoff into a finished machine](#15-ordered-execution-plan-turn-this-handoff-into-a-finished-machine)
- [16. Reproduction, tools, evidence layout and automation](#16-reproduction-tools-evidence-layout-and-automation)
- [17. The completion agent's single visual PDF: exact deliverable contract](#17-the-completion-agents-single-visual-pdf-exact-deliverable-contract)
- [18. Definition of done, remaining decisions and permissible final claims](#18-definition-of-done-remaining-decisions-and-permissible-final-claims)
- [19. Source index and evidence priorities](#19-source-index-and-evidence-priorities)
- [20. Copy-ready task for the completion agent](#20-copy-ready-task-for-the-completion-agent)

## 0. The actual assignment and the definition of success

Take over the design of a single-lane automatic machine for cutting and independently stripping both ends of stranded copper/PVC wire. Finish its mechanical, electrical, embedded, metrology, safety, handling, payoff and assembly design as one coherent system. Find and price **every** purchased part and specify and cost **every** made part and operation. Use Chinese as well as western sources. Minimise the cost of an adequate, robust, complete machine, with a target of about 70% reduction against a properly reconstructed equivalent baseline. Improve quality and robustness; do not obtain savings by reducing the loads, accuracy, wire range, lifetime, sensing, protection or manufacturing quality required.

Replace the EUR1,669 cabinet cooler with suitable hotter-cabinet-rated drivers and adequately filtered ventilation. Calculate it, integrate the actual cabinet and interlocks, execute the available numerical tests, and provide executable physical thermal/fault tests. Resolve actual design failures, rather than relabeling them as things for a builder to decide.

Design a practical, economical payoff for **both small and large reels**. Use the three supplied Schleuniger photographs as layout inspiration, with the evidential limits below. Include the payoff, dancer/buffer, guards, controls, adapters and output takeaway in the cost, native CAD, wiring, simulations and assembly instructions. They are part of the machine.

The completion agent's final outputs must include:

1. One frozen, internally consistent build configuration, with native editable CAD/PCB/software sources and exportable full-machine and part STEP files. Include all purchased component interface geometry and all manufactured features, including the real custom-axis internals and every moving interface. A box labelled BK12 or HGR20 is not a finished axis.
2. One complete, reconciled bill of materials and fabrication/operation register. Every physical occurrence must resolve to an exact buyable identity or a complete make-to-drawing definition, machine quantity, purchase quantity, cost and acceptance basis. No unresolved catalogue family, unspecified 'equivalent', missing mating plug, unpriced custom part or zero-cost labour fiction in the final selected build.
3. A reproducible engineering evidence package covering the unchanged requirements and the full declared operating envelope. It must contain actual model inputs, raw outputs, solver versions, hashes, convergence/uncertainty evidence, independent checks and the qualification tests that tie models to reality. Passing a validator that checks a document's own assertion is insufficient.
4. **Exactly one canonical visual PDF: `Cutting_Assembly_Manual.pdf`.** It must be a complete, simple assembly manual for the actual final selected parts, not a concept brochure, agent report, collection of conflicting PDFs or a list of external documents for the builder to reconcile. Section 17 sets its detailed contract. Native STEP, SVG, DXF, Gerber, firmware and structured data can be separate supporting files; all builder-facing PDF instructions and dimension sheets belong in the one PDF.
5. A transparent, like-for-like cost reconciliation and a qualification/release record. Distinguish design verification, prototype acceptance and production qualification. Execute physical tests only when real hardware/instruments are available; never invent measurements or call a simulation a physical test.

Treat 'works perfectly' as an obligation to satisfy every explicit requirement over a frozen envelope, including realistic variation, faults and assembly tolerances. A finite simulation or test campaign cannot establish universal 100% success for all future units and environments. Do not claim that it does. Establish bounded design margins, measured acceptance and quantified confidence, and explain the exact scope of the resulting claim. This is an evidence requirement, not permission to leave desk-solvable work unfinished.

The user is dissatisfied with repeated reports dominated by 'BLOCKED'. Therefore identify a failed mechanism, implement a concrete correction, regenerate its dependent artifacts, and verify it. An unresolved item needs a specific task, required input, artifact and acceptance test. A new status table by itself is not completion.

### 0.1 Authority and change control

- Preserve all 47 existing goals. Section 2 restates them; the original rows are in `data/original-47-goals.csv`. The later user instruction authorises replacing the 40 °C-limited drivers with suitable hotter-rated drivers while retaining the 40 °C room case. It does not authorise lowering any mechanical or process requirement.
- Preserve real upstream fixes. The older 13 October audit predates the latest fix-wave commit. Recheck each finding against current source; do not blindly restore earlier defects.
- A justified change to architecture or component choice is allowed within the unchanged goals. A change to acceptance numbers or advertised scope needs an explicit requirement decision. Do not quietly exchange Cpk, proof loads, maximum strips, smallest wire, maximum length or endurance for an easier target.
- Throughput in pieces/hour and the detailed reel size envelope are not fully frozen. Develop a concrete proposal using actual material/reel data, keep these as explicit requirements to resolve early, and continue independent work. Do not invent a measured rate or a reel rating from a photograph.
- Do not import the '90% same components' rule from the user's unrelated power-supply project. It is not a Cutting requirement.
- No supplier message, RFQ sending, purchase, merge or deployment has been authorised in this chat. Prepare exact RFQs and reviewable changes; do not send or purchase without explicit instruction. Keep development reversible and preserve the user's changes.

## 1. Source, recovery and evidence: know what actually exists

### 1.1 Fresh upstream check

On 29 October 2026, the connected GitHub repository listed only `main`, at:

`0f28aa3289cf052409ad48f54dc5d23b130fd441`

Its commit message is 'Audit fix wave: 35 findings closed across firmware, hardware, CAD, analysis, build definition'. The author date is13 October 2026. There are1,374 tracked paths in the fresh tree inventory. A new agent must inspect fresh HEAD before editing and preserve subsequent changes if the branch has moved.

Key source areas:

| Area | Authoritative starting paths |
| --- | --- |
| Original requirements and process | `docs/01-design-basis.md`, `docs/41-goal-traceability-matrix.md`, `docs/42-process-dynamics-r2.md` |
| Shared definitions | `shared/cutting_head_spec.hpp`, `feed_drive_spec.hpp`, `measurement_spec.hpp`, `mechanism_spec.hpp`, `controls_spec.hpp`, `machine_spec.hpp`, `cabinet_controls_spec.hpp` |
| CAD and analyses | `engineering/src/`, `engineering/include/`, `engineering/tests/`, `engineering/CMakeLists.txt` |
| Existing generated evidence | `engineering/generated/`; many files are historical or stale relative to current source |
| Supplier source evidence | `engineering/sources/`; distinguish exact manufacturer facts from family or reseller claims |
| Purchased/custom parts | all CSVs in `bom/`, PCB BOM in `hardware/if-b01/` |
| Native electronics | `hardware/if-b01/`, `tools/generate_if_b01_kicad.py`, `check_if_b01_kicad.py`, `check_if_b01_gpio_mapping.py`, `export_if_b01_manufacturing.py` |
| Actual target and core | `software/machine/core/`, `software/machine/rp2350/`, `software/machine/tests/`, `software/machine/host/` |
| Earlier reference software | `software/firmware/`; this is not a hardware-operating substitute for the production target |
| Manufacturing/assembly | `docs/12`, `13`, `14`, `21`, `22`, `23`, `30`, `31`, `34`, `36` |
| Model history | `docs/15` through `19`, `26`, `28`, `37` through `40`, and `docs/verification/` |
| Release/records | `docs/20`, `24`, `33`, `docs/inspection/`, source validators in `tools/` |

The numbered `docs/` references above are prefixes; use the exact filenames in `evidence/repository-tree.csv`. Documents 45/46 mentioned in recovered historical notes belonged to later local work and are **not present at pinned upstream HEAD**. Treat their ventilation/input proposals as reconstruction context in section 4, not files a fresh clone already contains.

Read relevant `AGENTS.md` instructions if present in the checkout. None was found in the recovered earlier work, but do not assume that remains true.

### 1.2 Lost local work: do not confuse it with upstream

Workspace maintenance removed the earlier local development checkout, unpushed source and numerical fields. The local branch name was `fix/buildable-redesign-2026-10-17`; a local commit was reported as `c83bca9`. **That commit is not an available remote branch or a patch delivered here.** The R4 board manufacturing files, routed source, both target UF2s, R7 STEP/drawings and raw R7 FEM fields from that work were not recovered. Cloning main does not restore them.

This handoff preserves the detailed R4/R7/ventilation proposals and reported numerical results as reconstruction leads in section 4. Reimplement them or improve them, then rerun the actual checks. Do not claim that the earlier local successes are current source, independently revalidated evidence, or physically qualified hardware.

### 1.3 Recovered historical files and inventories

The saved13 October artifacts were recovered:

- `Cutting_Design_Audit_2026-10-13.md`: audit of commit40b8b2d..., before the upstream fix wave.
- `Cutting_Engineering_Audit_and_Thermal_Test_2026-10-13.pdf`: historical engineering/ventilation report. It is source history, not the completion agent's final manual. No PDF copy is included as a deliverable in this kit; the extracted historical report text is retained for context.
- `Cutting_Parts_Prices_and_Goals_2026-10-13.xlsx`: historical inventory, fabricated-part quote fields, 47 goals and price observations.

Extracted tables in this kit contain217historical parts rows, 54fabrication-operation rows, 47 goals and52historical price observations. These counts do **not** prove a complete or currently qualified machine. Some rows are assembly summaries, shared parts, tooling or alternatives; normalise them before summation. Unknown amounts are unknown, not zero.

### 1.4 Checks actually executed for this handoff

- Compiled the unchanged upstream production core/tests with GCC C++20, `-Wall -Wextra -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer. All21 named host groups pass. LeakSanitizer was disabled for the hosted environment.
- Built and ran the new stock-side encoder counterexample using the same upstream controller with only a copied host fixture's sensor kinematics corrected. It faults at priming/ejection with `length_slip=0x100`, zero accepted pieces. See section 3 and `tools/reproduce_stock_side_mismatch.py`.
- Recomputed the stall-thrust, nominal feed-frequency/radius margin, tooth-pass, ventilation energy balance, old GPIO/contact-current and regenerative-headroom examples independently.
- Checked current BOM CSV field counts: `bom/selected-cutting-head.csv` line 2/AX-01 has 12 fields for 9 headers. The source also lists AX-01 quantity 1 although the machine requires two complete axes. This must be repaired before an automated procurement total is trusted.
- Re-read pinned GitHub source and selected published evidence. **No fresh native CAD/FEM solve, target Pico cross-build or physical hardware test was performed for this handoff.** Current raw native results must be regenerated by the completion agent.

Relevant records: `evidence/review-provenance.json`, `source-and-calculation-review.json`, `repository-tree.csv`, `host-tests-2026-10-29.log`, and `stock-side-counterexample/`. Every record states its scope. A stock-side counterexample is a deliberate demonstration of a defect, not a passing machine-function test.

## 2. Requirements register: preserve every original goal

The base material is **single-conductor stranded copper/PVC**, 0.5-2.5 mm2, actual OD1.2-4.5 mm. Solid copper, other insulation, aluminium, multicore, coax, shielded cable and live wire are outside the present declared scope unless separately designed/qualified. Do not quietly expand the claim to them.

All lengths are millimetres unless stated; force N; torque N m; motion time seconds. Active coordinates: X wire travel; Y opposing cutting strokes; Z up; cut plane X0; wire Y0/Z112. Legacy shared constants still contain other dimensions/scales and must not be used interchangeably with active definitions. Derive a single active coordinate/control contract and test its consumers.

| Goal | Required result | Completion agent's acceptance work |
| ---: | --- | --- |
| 1 | Finished length within +/-(0.20 mm+0.05%L), L200-3000 | Close the entire relaxed-length error budget and qualify full range; examples: 0.30 mm at 200, 0.70 at 1000, 1.70 at 3000. Internal correction residual alone is not external part accuracy. |
| 2 | Independent strip lengths within+/-0.20 mm | Verify both ends, all 3-25 mm settings, disabled combinations, actual incision and slip; internal guard bands must leave room for metrology and physical errors. |
| 3 | Qualified cut/strip quality, no conductor damage | Define measurable strand/jacket/end criteria, inspection method, blade material/edge geometry and wear limit; count/vision presence alone does not establish quality. |
| 4 | A stated, achievable throughput/cycle budget | Freeze a rate matrix by material, length and strips; include every physical state, buffer recovery, settle, quality check, slug removal and discharge. No unsupported100 mm example. |
| 5 | Cpk>=1.33;>=30unique consecutive pieces/cell | Stable actual process, independent guarded metrology, appropriate distribution model and uncertainty/confidence treatment; 30is a minimum record size, not proof of rare-event reliability. |
| 6 | Accurate Komax benchmark without unproved parity | Update dated feature/component/rate comparisons; use identical measurands/materials; no OEM-level performance claim from a paper target. |
| 7 | 0.5-2.5 mm2 Cu/PVC, OD1.2-4.5 | Verify all guide/nip/blade/sensor interactions and representative constructions/lots at both extremes and intermediate risks. |
| 8 | 200-3000 mm; 0or3-25 mm strips independently; 10 mm insulated grip margin | Model actual wire and insulation intervals through every phase; qualify shortest/maximum-asymmetric strips and supported3m output. |
| 9 | 0.50m/s feed; 0.05m/s initial commissioning | Prove hot loaded torque, traction, payoff/buffer and control timing; do not silently reduce full speed to evade a radius/firmware limit. |
| 10 | 2000feed pulses/rev; 260rpm ceiling; loaded-radius calibration | Freeze delivered settings; verify minimum loaded radius and maximum pulse demand, not just nominal 40 mm roller OD. |
| 11 | Blade12.5 mm/s, 300 mm/s2; 10 mm lead; 1000pulses/rev=100pulses/mm | One real screw/driver configuration, actual end machining, direction and homing; no SFU1605 label or800steps/mm instruction mixed into it. |
| 12 | Head2500axial/750transverse/375vertical; all signs; double proof | Actual complete load path, guide blocks, nut, supports, receivers, bolts, base and tooling must sustain unchanged working and proof envelopes. |
| 13 | Feed1 N m x2.5reversal; 5 N m fault; belt+/-480 N; roller+/-60 N both planes | Include actual rotor roles, dynamic sharing and all accepted nip-force cases; the upper75 N nip setting cannot disappear from an analysis capped at 60 N. |
| 14 | Machined-section stiffness>=reduced model; 191-section audit retained | Check final geometry and tolerance bounds, local contacts and actual sections; 191nominal samples are not a continuous proof. |
| 15 | Shaft fatigue/10,000h/1.8x10^7cuts | Derive a consistent usage spectrum, shaft revolutions, reversals, start/stops and rare jams; include mean stress, surface/size/notch/material scatter and tests. |
| 16 | Matched j5, 3-6 um actual interference, remaining bearing clearance | Resolve thermal fields first; measure each matched shaft/bore and cold/hot clearance, with capable metrology and assembly fits. |
| 17 | Bearing/housing contact stresses acceptable | Final fits, preload, actual loads and housing flexibility; no rigid support hiding contact or contact loss. |
| 18 | Shaft rise<=20 K above housing | Resolve the known failing motor/coupling/shaft/bearing heat path; verify hot operation with instrumented shafts and housings. |
| 19 | F109/collar withstands5 N m plus 7 kN clamp | Exact collar, key, thread and finish; torque/preload scatter, contact, slip, settlement and fretting proof. |
| 20 | Adapter peak converged and<=216.67 MPa screen | Actual receiver/support with certified650 MPa material/factor3; R7is only a candidate until regenerated and tested. |
| 21 | Actual cutting force inside actual axis capability | Validated cut-force envelope plus hot motor/driver/screw/guide/support limits; no old HIWIN cap assigned to a cheap generic support. |
| 22 | 0.020 mm nominal loaded blade-face gap | Derive and freeze the permissible interval from the unchanged cut-quality requirement; verify the complete loaded chain, edge/tool flexure and thermal drift. Nominal shimming is insufficient. |
| 23 | Every nip50-75 N, target60 N | Measure each nip over OD range, travel, hysteresis and wear; prove traction without conductor/jacket damage. |
| 24 | Each measuring wheel6-19 N, target10 N | Actual two modules, all bearing/encoder interfaces, force and loaded-circumference calibration; support every required sensor-validity interval. |
| 25 | Complete motor/coupling/head fit and independent jaw sweep | Native CAD with actual internals, tolerance-extreme continuous motion and tool/cable access; envelope-only tests do not qualify the custom axis. |
| 26 | Full static/moving machine integration | Include actual axis, feed, metrology, payoff/buffer, guards, cabinet, wiring, output and service/removal paths. |
| 27 | Payoff up to 30 kg; adequate support | Preserve30 kg maximum unless a new requirement increases it; define reel sizes/cores and safe loading/retention/braking for small and large reels. |
| 28 | Belt180-200 N, target190; dynamic margin | Correct79.58 Hz nominal tooth-pass, actual tension/load sharing, temperature, wear and resonance. |
| 29 | 40 °C room; hotter-rated replacement drivers | Suitable driver operating inlet rating, not a storage/marketing number; normal design air<=45 °C and selected candidate drivers<=50 °C, with actual physical margins. |
| 30 | All cabinet losses within ventilation/protection envelope | All motors' in-cabinet losses, supplies, drivers, fans, coils, PCB and added payoff/control loads; installed filtered flow, hotspots, faults and dirty filters. |
| 31 | K1-K3 adequate DC make/break on 36/60 V branches | Exact catalogue DC topology plus capacitive inrush, contact life, real opening and bus behavior. AC rating is insufficient. |
| 32 | Correct branch/XPS protection and coordination | Exact fuse/breaker voltage, breaking capacity, curves, I2t, wire/terminal limits and PSU-limited faults; input/output protections are different functions. |
| 33 | Regeneration absorption and deterministic discharge | Per-branch energy/headroom, actual minimum capacitance, exact absorption/bleed components, tolerances, thermal and measured bus decay. |
| 34 | Adequate guard openings/distances | All hazards/openings/entry paths, including added reel/buffer/conveyor; actual guards and assessed foreseeable use. |
| 35 | Independent dual-channel E-stop/guard safety | Risk-derived functions/PLr or SIL basis, complete schematic, certified data, diagnostics, common-cause and physical validation. Ordinary MCU/relays are not automatically safety-rated. |
| 36 | Guard release only after safe energy/standstill; safe power loss | Exact independent release request/monitor/timer/contact topology; validate stored energy, coast, power loss, welded contacts and manual recovery. |
| 37 | Cheap custom axis meeting unchanged goals | Complete geometry, verified exact components, all original loads/accuracy/life and actual equivalent cost. No cost-only C7/BK12 class substitution. |
| 38 | Every sourcing line selectable and priced | Exact MPN/configuration, quantity/MOQ, stock/date, tax/freight/currency, valid source and qualification; all custom operations also costed. |
| 39 | Resolve remaining sourcing and site-delivery enquiries | Compare exact couplings, axis, monitor, protection, mats, custom work and Tunisia delivery; draft RFQs without sending unless authorised. |
| 40 | Rust/C/C++ machine stack; independent safety; uncommissioned start prevented | Target cross-build and real startup/timing/fault tests; approval tied to all relevant identities and incapable of bypassing missing hardware in production. |
| 41 | Easy, reproducible assembly | Independent competent builder follows the one PDF without choosing unspecified geometry, wiring, torques, variants or missing parts. |
| 42 | Screenless routine operation | Approved retained recipe, clear panel states, start/stop/reset/repeat/recovery/material-change behavior; no PC dependency for ordinary production. |
| 43 | Complete automatic handling | Real prime/stub/slug removal, part presence-to-clear, positive output support and quarantine; no accepted part based solely on roller turns. |
| 44 | Factory robustness | Cold/hot starts, mains/USB/logic faults, EMI, sensor faults, friction/spool changes, wear, maintenance and recovery; improve actual failure margins. |
| 45 | Resonance/structural precision | Qualified relative tool-to-wire motion across speeds/duty, with measured FRFs, adequate bandwidth and final assembled modes. |
| 46 | Whole-machine endurance/serviceability | Duty-specific test, real wear limits, spares, lubrication/filter intervals, replacement access and requalification. Bearing L10 alone is not machine life. |
| 47 | Minimum adequate total cost and rational insourcing | Equivalent delivered build, all machining/grinding/heat-treatment/tooling/metrology/labour/qualification/consumables; no blank costs or duplicate kit contents. |

Additional explicit user requirements: complete small/large-reel solution; about 70% cost reduction while quality/robustness improve; all parts found; one visual assembly PDF generated by the completion agent; extremely broad and deep simulation/validation. Keep these linked to goals27, 29-30, 37-39, 41, 43-47 rather than letting them disappear between workstreams.

## 3. Fresh reanalysis: fix these interactions first

### 3.1 Sensor topology and firmware contradict each other

Pinned CAD (`engineering/src/mechanism_cad.cpp`) places the two measuring wheels at X=-205 and X=-385. `engineering/sources/second-module-position-analysis.md` explicitly selects both on the stock side because the old piece-side wheel cannot fit outside the existing cassette and still observe a200 mm part during backfeed.

Pinned controller `start_tracked_feed()` nevertheless maps feed-left to encoder0 and feed-right to encoder1. It verifies severed-piece tail withdrawal, pull and230 mm ejection with encoder1. After severing, no physical connection exists between the piece and either stock-side wheel. Those wheels cannot measure piece travel.

The existing `SimBoard::apply_step()` advances encoder[side] from each corresponding motor's commanded steps. This quietly gives the right motor an ideal piece-side wire encoder that the CAD no longer has. Consequently21host tests can pass while the real sensor configuration cannot pass a normal cycle.

The new fixture keeps the controller unchanged and makes both stock wheels follow stock motion. Result: `state=Fault`, `faults=0x100`, `pieces_done=0`, `pieces_accepted=0`, stock travel9990.265 um versus right roller travel240017.679 um. It fails at prime/ejection. This is a kinematic contradiction, not an uncertain stress estimate. Preserve the counterexample as a regression until a real revised sensor/material model makes the architecture coherent.

Do not 'fix' it by disabling all feedback checks, assigning imaginary encoder movement, setting part-clear true, or counting failed parts as good. Implement real observability and suitable validity windows. Candidate approaches to compare:

- Repackage a downstream measuring module within the short-piece insulated interval, relocating actual interfering cassette/encoder parts. At the cut plane for L200 and strips25/25 with 10 mm edge margin, the nominal insulated interval is X35-165; during25 mm backfeed it becomes X10-140; after a3 mm net pull it becomes X38-168. The intersection X38-140 is only a **first kinematic screen**, not proof that a63.66 mm wheel/encoder/housing fits. Check the full sequence, all contact/clearance tolerances and nips at X75.5/138.
- Retain stock redundancy and add real piece-motion sensing, if its total hardware/I/O/cabinet/accuracy cost is lower than repackaging. A driven motor encoder is not independent actual-wire sensing and cannot automatically establish no slip.
- A well-justified alternative architecture may use process observations and independent post-process inspection, but must satisfy accuracy, yield, handling, fault detection and throughput without merely rejecting an excessive fraction of parts. Quantify those consequences.

With a downstream wheel, its encoder is invalid before the wire arrives and after the piece leaves it. Do not require both counters to equal the whole commanded feed from time zero. Use material occupancy, shared observation intervals and explicit origin transfer. State which measurand each sensor observes in every phase. Two stock wheels also share material/tread/temperature systematic errors; agreement is not independent proof of absolute length.

### 3.2 Prime, slug and part-clear state logic needs a physical plant

The nominal prime stub is10 mm; the nearest right nip is75.5 mm from the blade. A10 mm loose stub cannot be propelled by a nip it never reaches. Specify actual stub/slug removal: a constrained mechanical sweep, chute or justified air system, with exact components and fault detection. Do not assume compressed air is free or available because an OEM photo has a rear connector. Include compressor/site air, filter/regulator/valve/hoses/energy/noise if the design relies on air.

Leading and trailing insulation slugs of 3-25 mm have similar reach/retention problems. Prove their release and clearance before a new wire/feed or blade-close can trap them. Simulate adhesion, static charge, wire curl and debris realistically; verify with actual materials. Gravity alone needs a demonstrated reliable geometry, not a sentence.

Current `PieceDone` clears `part_present_seen_` and only starts watching after ejection. A correctly cleared part may have passed the sensor already. Capture and validate the prior-present/clear transition during the correct physical observation window, with timestamps and plausibility checks. Test early/late events, bounce, double parts, stuck high/low, open/short, prime waste and manual removal.

The target currently hardcodes `in.part_clear=true`; Settings defaults `part_clear_check=0`. No real channel is allocated. Production must not bypass required handling using either default. Fixture/engineering modes must be explicit and must not emit accepted production counts or persistent approval equivalent to a qualified build.

### 3.3 The custom axis is still an envelope with incompatible parts

Current spec uses rail340 mm while procurement says400 mm; the stage is440 mm long/86 mm wide and the carriage60x116 mm. The guide to four HGH20 blocks/DSG16/BK12 is not a resolved fit. H003 is incorrectly described in docs/39 as attached to moving H006 while its motor/screw support must be stationary. Attach the adapter to a real stationary receiver, and design that receiver together with the bearing/end-machining stack.

The spec's BK12-class maximum 4900 N is below both the 5000 N proof requirement and the independently calculated stall thrust 2*pi*8.5/0.01=5340.708 N. Old generated 'current-analytical' CSV still uses HIWIN6320 N and old HIWIN guide ratings to pass this check. That output is stale relative to selected custom source. It cannot qualify a generic support. Current-source guide moment allocation also needs actual block-group reactions, not a favourable total capacity divided by four.

Reconcile the actual10 mm-lead screw designation, grade, preload, nut dimensions, root diameter, end machining, shoulders, bearing pair, locknut, seal/lubrication drag and accuracy. Do not treat an SFU1605 family page as an exact SFU1610 C5/preloaded complete kit. Inspect real component drawings and tolerance classes; generic Chinese blocks cannot borrow genuine HIWIN ratings by name resemblance.

### 3.4 Feed pulse margin and metrology budget are not closed

At nominal 40 mm roller OD, 500 mm/s and2000pulses/rev require7957.747 Hz. Current production ceiling is8000 Hz: only 0.53% margin. To stay within 8000 Hz at 500 mm/s, the actual loaded radius must be at least19.89437 mm. A0.1056 mm radius reduction consumes the entire margin. Urethane indentation, wear and actual drive calibration can matter. Raise and qualify the timing envelope if necessary; do not silently reduce the required feed speed or leave nominal-only assertions.

A current legacy/shared header also contains3200feed pulses/rev and a40000 Hz ceiling while active controls use2000/8000. Establish and test one active source of truth; historical constants need clear isolation and must not drive production consumers.

The documented full error allocations already fail: approximately0.524 mm at 200 mm against 0.30, and0.788 mm at 1m against 0.70, with conditional tension and synthetic phase compensation. The published uncalibrated old report1.65 mm versus 0.70uses old measuring selections. These are not actual measured part errors, but they demonstrate that the existing allocations are not a proven bound. Do not confuse encoder resolution with accuracy, or use the full part tolerance for controller residual while leaving no budget for every other error.

The target stores scalar counts/metre; the phase-map model requires repeatable wheel phase/index referencing not currently integrated. Either implement calibration and reliable phase capture through the actual hardware/firmware, or prove the scalar-only architecture meets all original targets. No phase-map benefit can be credited to a controller that does not use it.

### 3.5 Shaft heat, fit and complete structural chain remain coupled failures

Pinned `bearing-contact-thermal/status.json` reports 30 W imposed source, seat conductance 2800 W/(m2 K), shaft maximum 54.6603 °C with 25 °C reference: rise 29.6603 K against 20 K. Conductance sensitivity 1000/8000 gives 51.7258/15.5336 K. This is a conditional model failure, not a measured thermal result. Model the complete motor, coupling, shaft, bearings, housing and convection paths and actual heat split. Never reduce 30 W merely to get a pass or assume a thermally insulating coupling without measured/catalogued conductance and adequate torsional stiffness.

Pinning a housing to room temperature can conceal its own heating and differential expansion. Recalculate the j5/interference, C3/CN internal clearance, preload, friction and accuracy under the coupled field and worst cold/hot starts. Use actual ball/race thermal gradients and manufacturing tolerances. Matched bearing seats require actual grinding and competent measurement; available CNC is not evidence of 1 um capability.

Pinned R6 adapter criteria show285.987 MPa against 216.667 MPa; peak refinement18.46% against 5%, displacement refinement4.955% against 3%. These stored outputs are historical and unconverged; 285.987 MPa is the value in the pinned criteria CSV, not a freshly re-solved result. The earlier audit also quoted about 313 MPa from another study; do not select whichever number looks best. The complete holder/backing/blade/axis/joint/datum path must maintain the loaded face gap, not merely pass an isolated holder calculation.

### 3.6 Electrical, thermal and documentation inconsistencies still matter

Native BOM says internal pulls must be disabled, but both target and fixture source still call `gpio_pull_up()` on field inputs. Old 10k/47k inputs with an internal 32k pull-up give 1.1364 V at nominal 3.3 V, above 0.8 V guaranteed LOW. Selected mechanical relay contacts see only 57.895 uA/3.3 V, below their intended minimum wetting duty. RP2350 silicon errata and startup states need an actual fix.

The native BOM also has wrong Phoenix mating-plug order codes, an eight-position footprint paired with a ten-position header identity, and Keystone 9903 described as M3 mounting hardware although it is a6-32 screw. Header/socket body, pin length, underside clearance and mating-plug access need manufacturer-coordinate checks. Native ERC/DRC cannot detect every wrong purchased MPN.

The old regeneration claim compares energy to energy already stored rather than voltage headroom.0.1J added to 100uF initially at 36 V raises it to 57.411 V, over a50 V feed-driver ceiling. Normal PSU overload protection is not a specified regenerative sink. Actual branch capacitance, energies and discharge components remain required.

Old guides/README still say fabrication-ready and carry cooler, T-driver and other superseded selections. Do not build a new manual by copying these claims. Make every active instruction agree with the final native design and invalidate stale generated bundles by hash checks.

### 3.7 Fresh catalogue corrections

Both V20 manuals state0-50 °C operating inlet and40-90% noncondensing RH. The CL86Y manual's marketing text says up to 110VDC but its electrical table says80VDC; use the conservative80 V ceiling until the exact delivered variant is clarified. CL57Y ALM output maximum is50 mA. Its5.6A maximum output must not be copied blindly into a5A/phase motor setting; distinguish RMS, peak, current-percentage base and load-adaptive behavior.

The unusual published40%RH lower bound also needs explicit reconciliation with the real site envelope. Do not assume it means0%RH or that a heater/ventilation arrangement automatically keeps the local humidity in range. Obtain manufacturer clarification or select a better-evidenced driver if the operating humidity cannot be satisfied. The current data therefore support a candidate, not a universally qualified drop-in replacement.

The exact PF66000 manufacturer page now says **sleeve bearing**, G4 mat, 120 W at 50 Hz/160 W at 60 Hz, 445 m3/h with a matched outlet and a6A gG prefuse. The earlier local claim 'ball bearing' is withdrawn. Resolve any catalogue conflict against the exact supplied variant before claiming increased lifetime. A brochure describing another voltage/fan is not sufficient.

Do not silently narrow the factory environmental envelope to 40-70% RH just because a candidate fan-presence sensor is limited to 70%. Resolve real site conditions, choose suitable parts or obtain clear manufacturer limits. Likewise a photograph of a ventilated OEM does not establish that this machine's650 W cabinet or contamination environment is acceptable.

### 3.8 Count rollers, nips and force records correctly

Pinned CAD/source and the recovered audit describe **four rollers per cassette: two driven lower and two passive upper, forming two nips per cassette**. The complete two-cassette machine therefore has eight rollers and **four nips**, plus two measuring-wheel stations. Do not confuse four rollers with four nips or count both faces of a passive nip as independently driven traction. There are two separate belt-tension stations.

The existing qualification template's twelve force rows mean six physical force stations (four nips plus two wheels) at two wire ODs. They are not twelve different nips/stations. Record each real nip/wheel independently at both ODs and the full travel/temperature/hysteresis cases; record both belt tensions separately. If a revised architecture genuinely changes the counts, derive the new occurrence/force map from its actual CAD and update every consumer.

## 4. Earlier redesign candidates to reconstruct, challenge and verify

These are unusually detailed starting points, not recovered source or release evidence. Preserve their useful reasoning; do not blindly accept their claims. A change to sensor/I/O architecture can require a new PCB revision before any manufacturing order.

### 4.1 R4 field-input and board proposal

The lost local proposal reduced IF-B01 from 750x400 mm to 250x200 mm, with four copper layers and two 35 um inner0Vplanes. Ten on-board Toshiba `TLP176AM(TPL,E` photorelays replaced ten external mechanical input relays. This gives functional level separation; do not imply a complete safety-isolation barrier when field returns are bonded externally.

Per input:

- Field source19.2-28.8 V, ON drop<=2.5 V, OFF leakage<=0.10 mA, source/sink capable of at least39 mA.
- R46-R55 LED series: 1.5k, 1%, 1 W, 2512, Yageo `RC2512FK-071K5L`.
- R56-R65 parallel field bleeders: the same1.5k1%1W2512SKU; 20resistors total. The older 1k bleeder was rejected because input current approached the feed driver's50 mA ALM maximum and dissipated more heat.
- LED reverse diode: Diodes `1N4148WS-7-F`, SOD323, physical striped cathode to LED anode node, anode to field24 N.
- Input TVS: MCC `SMBJ33CA-TP`, bidirectional33 V SMB, across24P/24 N. Recheck dynamic clamp and pulse-energy coordination against every input device/resistor; 600 W headline alone is not a system surge qualification.
- TLP actual physical pins: 1anode, 3cathode, 4/6output drains; no pins2/5. Pin4to0 V; pin6to1k series into GPIO; 4.7k GPIO pull-up to 3V3; 100nF16VX7R to 0 V. Remove the old GPIO Zener arrangement if using this exact circuit. Check full transient/injection behavior before accepting any replacement.
- MCU rail3.135-3.465 V, return offset<=50mV. Both images must execute internal-pull disable calls for these externally biased inputs.
- Candidate hot limits VF<=1.8 V and off-state leakage<=100 uA were fixture acceptance assumptions, not full-temperature guarantees supplied by the manufacturer. Replace assumptions with guaranteed component bounds where possible and verify the actual board across temperature, contamination, aging and source types.

Reported earlier numerical reconstruction targets: 32 corners; LED9.7865-19.4924 mA; maximum source38.9848 mA; maximum LED/bleeder resistor dissipation0.561381 W each; ten-channel field heat11.2276 W; GPIO LOW<=0.674921 V, HIGH>=2.65318 V. Six deliberately invalid cases were reportedly rejected. The raw data/script are lost; recompute all of these from the final circuit, including every tolerance and hot limit. Recheck TLP trigger/release/switch resistance, derating, input capacitance and response. In particular, do not freeze the 1.5k LED series value merely to consolidate the resistor SKU: optimise LED current separately from contact-wetting/bleeder current using guaranteed hot trigger/aging limits and the actual local PCB temperature. A lower-current LED branch could reduce heat and improve margin if its worst-case trigger and timing remain adequate. A nominal gate calculation does not qualify field immunity or all sensors.

The proposed MCU was Pico2 with factory headers `SC1632`, requiring RP2350 A3-or-later silicon because of E9. SDK-reported chip-version interpretation must be checked against its pinned code and current manufacturer datasheet; do not rely on the printed A4 label alone. Unsupported silicon must fail boot safely and cannot be cleared into production. Do the same in the fixture image. Target pulse-high and DIR-setup proposals were10us each; the production8000 Hz ceiling was not raised in that lost work.

Corrected complete connector/mounting proposal:

| Item | Quantity for one board/kit | Exact identity / condition |
| --- | ---: | --- |
| 4-position PCB headers J3-J7, J12 | 6 | Phoenix 1759033 |
| 6-position PCB headers J8/J9 | 2 | Phoenix 1759059 |
| 8-position PCB header J11 | 1 | Phoenix 1759075; not1759091 |
| 12-position PCB header J10 | 1 | Phoenix 1759114 |
| 4-position mating plugs | 6 | Phoenix 1757035; not1757233 |
| 6-position mating plugs | 2 | Phoenix 1757051; not1757259 |
| 8-position mating plug | 1 | Phoenix 1757077; not1757291 |
| 12-position mating plug | 1 | Phoenix 1757116; not1757314 |
| Pico with male headers | 1 | SC1632; receiving silicon test required |
| Pico sockets | 2 | Samtec SSQ-120-23-G-S; actual height, pin tail, contact engagement and SWD underside clearance verified |
| M3 female/female nylon spacers | 4 | Wurth 970120365, 12 mm, SW6 |
| M3x6 nylon pan-head Phillips screws | 8 | Wurth 97790603111; two per spacer |
| Board holes | 4 | Fabricator NPTH3.2 mm; not a purchased Keystone 9903hardware set |

The proposed mounting plate is2.0+/-0.1 mm and PCB1.6 mm. Check actual opposed-screw engagement and tip gap, with at least3 mm engagement at each end and at least1 mm internal tip gap. Inspect nylon screw length and use a qualified sample torque; never apply a steel M3 torque table. Strain-relieve the harness independently. The two rows of Pico sockets must be mapped by actual coordinates to physical Pico pin numbers, not simply by symbol numbering.

Existing other PCB components retained in the proposal included TI `AM26LV32EIDR`, two `TPL7407LDR`, Bourns real1812PTCs `MF-MSMF050/60X-2` and `MF-MSMF050/16X-2`, named Yageo0603resistors, Murata decouplers and Keystone 5000test points. Complete suffixes, hot PTC resistance and actual source coordination still need review. Native pin, pad, drill, courtyard, mating-body and solder-process verification is mandatory for all parts.

A previous32-observation basket was aboutUSD111.7556for all proposed board components/Pico/mating plugs/mounts, excluding bare PCB, assembly, fixture, shipping, tax. Most of those observations are dated17 October and are not fresh29 October quotes; their raw priced source capture is not recovered. Do not use the number as the final board price. The roughlyUSD12.547input-addition subtotal was only the extra input components, not the complete assembly.

A reported earlier native run used KiCad10.0.7 and Freerouting2.4.1 with analytics off, zero ERC/DRC/unconnected/parity issues and1055tracks/vias. Its files are lost and no such passing native board is in main. Recreate routing natively without altering approved component/pad geometry, run all-track-errors and schematic parity, and rebuild a source/output hash manifest. A generator must never silently overwrite the final routed board with an unrouted draft.

### 4.2 Complete the I/O architecture before freezing PCB manufacture

All26exposed Pico GPIOs are allocated in the old design. Real part/slug sensing, index capture, buffer/feeder status and any additional quality sensing need actual I/O. Do not multiplex safety or motion signals with an undocumented harness change.

One concrete option to evaluate is a16-bit industrial-temperature I2 °C expander with defined power-on states, timeout/fault behavior and proper24 V conditioning. Move only suitable slow panel/diagnostic/indicator functions to it; retain step/direction, drive-disable and any timing-critical encoder/index/home capture directly as justified. Example allocation to investigate: move RESET and safety **diagnostic** inputs plus three indicator/buzzer outputs to the expander, free GP14/15for a checked I2 °C pin pair, GP22/26for two index captures andGP28for an expander interrupt. This is an engineering candidate, not a verified pinmux or board; check official pin functions, all PIO consumers and actual source. The hardwired safety path is never moved onto the expander.

Budget all expanded inputs, outputs, photorelays/buffers, connectors, pull-ups, power and firmware. Detect stuck bus, NACK, missing module, stale readings, interrupt stuck, power-order/backfeed and readback mismatches. A failed expander may extinguish an indicator; it must not leave motion authorised. Sensor observability may instead justify another COTS MCU module or a revised board; compare complete cost and verification effort, not just IC price.

### 4.3 R7 adapter proposal

H003 remains100x100x15.00+/-0.02 mm, 42CrMo4+QT with certified yield>=650 MPa. Four F0 counterbores become13.00+0.10/0diameter and9.50+/-0.05deep. Add eight F118 load-spreading washers per machine: 42CrMo4+QT>=650 MPa, OD12.00+0/-0.05, ID5.50+/-0.05, thickness2.00+/-0.05, flatness/parallelism0.03, Ra1.6, edge break0.10 mm. There are two H003adapters, four F118washers per adapter.

The candidate uses 8F0M5x18class10.9screws; item0.0.655.05was an exact manufacturer identity lead, but no price was verified. Retain8motorM5x25class10.9and8separateOD9x1motor washers. Do not keep16OD9washers plus 8F118by accident. Consider a cheaper standard bolt length such asM5x20if the **actual redesigned stationary receiver** can provide the extra full-thread/drill depth, tip clearance and stiffness; recompute all stacks and joint mechanics first.

The receiving interface is certified steel>=650 MPa, actual locating pocket/PCD70, M5full thread>=12 mm, drill depth>=14 mm, entry chamfer<=0.30 mm for the 18 mm-bolt proposal. A genericBK12 or aluminium moving carriage is not this receiver. Specify the real bearing housing/receiver and its elastic edge/support effects before calling the adapter assembly complete.

Candidate tightening: 4.2 N m+/-4%, but **only** with qualified thread and underhead friction0.10-0.18, interface friction>=0.10 and settlement<=5%. A generic zinc finish or threadlocker does not guarantee this. Use exact thread helix/friction and annular bearing-radius equations, not an arbitrary torque coefficient.

Reported earlier reconstruction targets: 64corner cases; preload3247.61-5931.72 N; retained minimum 3085.23 N; rear150 N load adds225.96 N/bolt; loaded6000 N-preload bound6225.96 N; M5class10.9proof14.2mm2x830 MPa=11786 N; slip screen3.1413; conservative receiving-thread shear screen3.1822; minimum full engagement8.13 mm; maximum penetration10.62 mm; remaining full-depth margin1.38 mm; minimum floor5.43 mm; motor-pilot/head clearance0.60 mm. These depend on the exact geometry and assumptions; rerun and inspect all stacks, including incomplete screw tips. The central2 mm register boss is outside the bolt grip path and must not be subtracted from thread engagement.

The earlier scoped FEM reportedly used6 kN/screw, both 8.5 N mtorque signs, 150Nrear motor load and self-weight; all four loaded/support seats refined at 0.40/0.28/0.20 mm, bulk2 mm, nodes34973/58331/109664. Reported peak111.2225 MPa, maximum displacement0.0043484 mm, last peak change1.9237%, last displacement change0.5231%, foundation0.5x/1x/2x peak111.2329 MPa, seven scoped criteria passed. It was a **linear resolved-pressure elastic-foundation** model with an assumed smooth quartic annular profile, not nonlinear real washer/threads/friction or the entire axis. Raw files are lost. Reproduce from source, then challenge with actual contact, receiver flexibility, preload scatter, slip and physical torque-tension/proof tests. Do not promote this number to a full-machine guarantee.

An earlier new F118section drawing was briefly wrong and then corrected. This illustrates why every final page must be visually reviewed against true section geometry. Recreate accurate plan/section with the central bore; do not use an uninspected generated rectangle.

### 4.4 Ventilation and V20-driver proposal

Candidate core: two `CL86Y-V20` head drivers, two `CL57Y-V20` feed drivers, two PF66000 `11666102055`230V50Hzfans, two PFA60000 `11760002055`outlets, one Rittal `3114200`digital thermostat. Keep existing motors only after hot torque/current/temperature validation. Head motor extensions were `CE2-M2-18`, two sets. Do not also order the retired complete T-driver kit after disaggregating motors/drivers/cables.

Current29 October seller-page observations: CL86Y-V20USD44.88each; CL57Y-V20USD33.52each; PFfanEUR171.89exVATeach; PFAoutletEUR34.65exVATeach; thermostatEUR86.13exVAT. The two fans/two outlets/controller core sumsEUR499.21exVAT. Relative toEUR1669, this is70.09% lower **only on an equal tax basis and only for that core**. Relays, monitor, fuses, mats, enclosure, wiring, installation, shipping/import and extra control/driver differences are additional. The old cooler tax basis was inconsistent. Whole-machine70% saving remains unestablished.

One matched fan/outlet pair is445 m3/h at 50 Hz, not640 m3/h free air. Two catalogue paths give 890; require measured installed service flow>=800 m3/h under the final filters, doors, ducts, device layout and cabling. Fan curves and a measured system resistance curve determine actual flow. Choose exact mats and spare pack `18611600033`only after reconciling grade/identity. Current maker page says sleeve bearing; verify lifetime/orientation rather than asserting ball bearings.

Earlier650 W allocation: three350 W motor-supply losses123.1844Wassuming efficiency>=89.5%; drivers160; fans280(two 140 W design allowances); control-supply losses16; coils30; board/safety20; thermostat2; other10; reserve8.8156. These are **allocations**, not measured upper guarantees. Verify voltage tolerances and additional payoff/IO/bleed losses and update the total.50 Hz-only qualification cannot be extended to 60 Hz, where the maker lists 160 W per fan.

With density>=1.05 kg/m3 andcp1000J/(kg K), 650 W/800 m3/h gives bulk rise 2.785714 K. At 40 °C room plus assumed1.5 K hotspot, the hottest normal air is44.285714 °C. Earlier alarm45 °C+2Ksensor error+1.5Kmapping+0.5Kpost-trip overshoot gives 49 °C. The hotspot/mapping/overshoot are **physical acceptance bounds**, not manufacturer facts or outputs proven by the simple energy balance. Compute moist-air density from actual pressure/temperature/humidity and qualify the real site envelope.

Proposed24Vsupply `HDR-60-24`; 5Vsupply `MDR-40-5`with DC-OK contact, set5.2 V. The 5Vmodel is30 W/6A, not40 W, and at 5.2 V current is limited to 5.77A before thermal derating. Recalculate full hot cable/PTC/connector drop and Pico maximum voltage with source regulation, ripple, temperature coefficient and transients. Old Siemens+/-3% and fault-current limits do not transfer. Also model USB backfeed into VSYS/the 5Vrail and all power-order cases.

V20MF is active motor-free/disable and clears alarms; it is not an ENA-compatible drop-in and is not STO. Earlier proposed adapter: two Phoenix 2966265gold-contact relays; GP17HIGH energises coils and opens NC contacts to permit motion; LOW/reset drops coils and closes NC toassertMF while24 V exists. One relay serves both headMFinputs(max40mAtogether); one serves both feedMFinputs(max15mAtogether). A single50mAgold contact cannot serve55 mA. Check actual delivered currents, minimum loads, temperature, coil suppression and startup behavior.

The earlier protective permission proposal chainedMDRDC-OK, RittalK2healthy, three driver-logic-presence relays andtwo fan-presence switches into two normally energised thermal permission relays; one contact in each independent XPS input loop. Keep XPSA1properly supplied, manual reset, EDM and guard-release chain. Ordinary presence relays are **not** calibrated undervoltage monitors; prove valid logic thresholds and dropout sequencing over voltage ramps. Loss of MFlogic power must remove motor power through an independent adequate path. A software alarm cleared by MFmust still be latched/logged and quarantined correctly in the controller.

Saipwell `LC013 NO01300.1-00`was a fan-**presence** candidate, not an800 m3/h flow interlock. Exact price was not obtained. Its maker lists a50 °C/70%RHlimit and broad vane threshold/hysteresis; verify its complete current catalogue. Do not use a STEGO price or NCpart as its quote. Partial filter clogging can leave a presence contact closed. Either use proven quantitative protection or demonstrate that the thermal interlock detects all unsafe flow patterns before temperature limits. Include all new hardware and its wiring/access in the cabinet model.

No1000x800x400 mm cabinet size is approved merely by repeating the earlier minimum suggestion. Each 320 mm fan/outlet face and292 mm cutout, 150 mm fan intrusion and outlet depth must be placed in real3Dwith device clearances, baffles, ducts, terminal bends, service paths and no flow short circuit. Material thickness can change required cutout geometry. All devices need actual inlet rather than room-air temperatures.

## 5. Payoff and reel handling: use the photos thoughtfully

### 5.1 What the supplied photos establish

`images/machine-front.jpg`shows a guarded Schleuniger cut/strip machine with two feed sections around the tooling and a screen. `images/machine-rear.jpg`shows ventilation/service/power connections. `images/payoff-and-buffer.jpg`shows a floor-level reel supported in a roller cradle and a separate vertical, multi-groove pulley/slide arrangement between reel and cutter, with a local control cabinet. The general arrangement is useful.

The supplied originals are retained in the kit at those three paths. Inspect all three before designing; the payoff photograph in particular should inform loading, storage, guarding and service concepts rather than be treated as a scaled drawing.

The photos do not establish exact model, reel mass/diameter, bearings, motor/brake ratings, dancer reeving, control law, wire tension, airflow, certification, pricing or hidden guarding. Do not scale dimensions from perspective or copy a screen dependency into a screenless requirement. Do not infer the depicted unit is a specific PreFeeder model.

Primary Schleuniger information confirms distinct architectures: PreFeeder1000motorises a reel with a dancer arm and is rated20 kg; PreFeeder1100uses an adjustable-pressure belt puller with a dancer slide, 0.90mstorage, also20 kg. These are useful references, not30kgqualifications. Other published PreFeeder2200ratings vary by version/region; never mix them. The project must have its own exact30 kg-rated design and declared reel envelope.

### 5.2 Preferred design hypothesis and alternatives

Investigate a **common low-tension buffer/controller with a floor-level powered roller cradle for larger rigid flanged reels and a removable supported spindle cassette for small/unsuitable reels**. It follows the useful physical separation in the photograph: the cutter need not accelerate the reel through the wire. Share as much controller, drive, brake, frame, guards and buffer hardware as the two modes allow. This is a preferred hypothesis, not a purchased/final design.

Compare against a single two-supported powered arbor with interchangeable cones. It may be cheaper and more positive, but30kgloading, arbor handling and torque transfer need an equally good solution. A belt-puller prefeeder with passive reels can be economical only if wire tension **upstream of the buffer** never causes permanent conductor stretch, jacket damage or uncontrolled reel overrun. Moving high tension upstream does not make it disappear.

| Method | Strength | Risks to resolve before selection |
| --- | --- | --- |
| Driven floor cradle+small-reel cassette | Low lifting effort; large flanged reels easy to roll in; common buffer | Rim slip, warped/soft flanges, axial walk, flange damage, roller nip guarding, cassette drive/brake connection |
| Powered two-support arbor+cones | Positive controlled reel; many bore sizes; potentially fewest mechanisms | Loading/retention, bore/clamp damage, cone torque path, shaft/stand deflection, removable-end alignment |
| Belt-puller+passive reel+buffer | Universal wire transport; possible COTS low-cost module | Acceleration tension before buffer, slip/jacket marking, reel coast, small-core/large-flange inertia |
| Passive spindle+brake+buffer | Cheap for small, low-inertia reels | No universal large-reel replenishment/low-tension guarantee; brake force changes with radius |
| Over-end payoff/coil basket | Few rotating parts for some packages | Twist per turn, loops, pigtailing, entanglement, bend memory; not a universal copper/PVC solution |

Rigid wood/plastic, cardboard, damaged flanges, loose coils and flangeless packages are different interfaces. State which are included. For a declared variant, provide real adapters and instructions; otherwise state it is outside that variant rather than pretending one cradle accepts everything.

### 5.3 Freeze a real reel and wire envelope

Known existing limit is30 kg. Resolve minimum/maximum flange OD, core OD, reel width, arbor bore, reel empty mass/flange construction, winding width/layer pattern, wire package and centre-of-mass offset. Obtain actual catalogue or measured examples for both small and large reels. Include full, part-empty, near-empty and abnormal winding states. A generous conservative proposal may be developed while dimensions are being confirmed, but it cannot become a hidden assumption in a perfect-function claim.

Specify lowest wire-manufacturer minimum bend radius at the relevant temperature and construction. Choose pulley diameter/groove/finish, wire alignment and reeving for OD1.2-4.5without pinching, crossing, falling out or rubbing flanges. Do not simply set pulley radius from wire OD without manufacturer/coupon evidence. Include reeving access and an unambiguous diagram in the final manual; wrong reeving must be hard to assemble and detectable.

Use a motorised payoff that normally unrolls only; the buffer should absorb the stock's reverse strip motions without forcing an unqualified loose rewind onto the reel. Qualify every start, stop, prime, pause, emergency trip, wire break, spool end, stuck guide and power-loss state. Include the small-reel cassette's own inertia/drag and any changed gear ratio.

### 5.4 Required payoff/buffer equations and state coupling

Use physically consistent radius, mass and inertia rather than combining a full reel's wire mass with an empty core's radius as if they coexist. For a uniformly wound annulus as a first model:

`J_wire(r)=0.5*m_wire(r)*(r^2+r_core^2)`; add actual empty spool/flange inertia. Identify winding packing and mass-radius relation from known wire length/mass or measurement. Preserve conservative uncertainty bounds for nonuniform winding and eccentricity.

`omega=v/r`;`tau=J*alpha+tau_bearing+tau_brake+tau_winding`; direct wire-driven acceleration would require approximately`F=J*a/r^2+tau_resistance/r`. Include changing r and angular acceleration terms during unwind. For a rim drive, derive its geometry separately, including flange-radius torque conversion and slip/normal loads. Do not apply the wire-radius equation blindly to a roller drive.

Buffer storage is a state, not just an arm length:

`B(t)=B0+integral(v_payoff-v_cutter)dt`.

Use the actual reeving Jacobian `dB/dx`; for Nfully active moving sheaves with nearly parallel strands it approaches2 N, but angles and limits matter. Compute required stroke from the **largest accumulated speed mismatch**, sensor/filter/control/brake delays and reverse motion, with reserves at both ends. Long3mfeeds do not necessarily require3mstatic buffer if the payoff continuously replenishes; prove the actual trajectories instead of assuming either extreme.

For a simple vertical dancer, derive tension from the complete moving-pulley mass, spring/constant-force element, acceleration, guide/bearing friction and strand angles. The approximate`T=(m*g+F_spring+m*a+F_friction)/(2N)`is only a limiting geometry check. Use the exact force balance and quantify stick-slip, deadband and hysteresis. Zero-tension marketing language means low controlled tension, not physically zero tension.

Wire extension must enter the metrology model:`deltaL=T*L/(E_effective*A)`. For0.5 mm2 at 110 GPa this is0.01818 mm per N per metre; at 3m it is0.05455 mm/N. Stranding, PVCviscoelasticity, construction, temperature and relaxation can make the actual relation different. Derive a permitted tension/variation allocation from the final full error budget; do not arbitrarily book5 N or44 N as harmless. Include measuring-wheel and guide drag **after** the buffer and direction changes.

Compute motor torque-speed/thermal duty, gear/belt efficiency, reflected inertia, brake stopping energy `0.5*J*omega^2`, repeated-stop heating and power-loss coast. Check peak speeds at minimum winding radius and maximum credible overspeed; never let a rapidly emptying small reel exceed mechanical limits. Include radius-estimator failure if used; prefer observable, stable control that does not depend on an unmeasured radius when practical.

### 5.5 Control, faults, mechanical loading and acceptance

Provide a hybrid state model: load/thread, buffer-charge, ready, run, controlled stop, buffer-low/high, jam, wire-end, wire-break, guard-open, E-stop, power-loss and recovery. Derive speed limits and rate limits; include saturation, integrator windup, sensor delay, mechanical end stops and feedback failure. A two-switch bang-bang scheme is acceptable only if its ripple, overshoot, wear and fault margins pass the same requirements; do not equate simplicity with adequacy.

The cutter starts only with enough verified buffer reserve and a healthy payoff. A payoff fault must prevent new feed and produce the assessed controlled/safe stop through the proper paths. A broken serial cable or dead local controller must not leave an uncontrolled reel running. Personnel protection must remain independent of either ordinary MCU. Provide exact handshake/isolated I/O and all connectors/cables.

For a30 kg reel, model spindle/cradle/axle bending, bearing reactions, rim contact, frame/weld/floor-anchor strength, axial walk, roll-off and tipping. Include asymmetric loading, eccentricity, side pull, blocked reel and brake reaction. Loading must not require an unexplained unsupported30kgmanual lift. Specify a ramp, retained cradle, removable arbor aid or justified handling device and include it in parts and cost. Guard rotating rims, chains/belts, roller nips and dancer travel; make wire threading/service safe and accessible.

Validate with the smallest/largest qualified reels, full/mid/empty states, smallest/largest wire, hot/cold, higher/lower friction, maximum feed acceleration1.5m/s2, all strip reversals and abrupt stops. Record actual tension at the cutter, buffer reserve, slip, wire condition, coast, brake heat and ready/fault response. The final selected method is the lowest **complete verified** cost that passes this matrix, not automatically the photographed method.

## 6. Mandatory standard for every single part and interface

This section applies to **every** buy/make/modify occurrence, including apparently trivial fasteners, cable accessories and consumables. Apply the universal record plus the relevant family standards. Do not exempt hidden kit contents or peripherals.

### 6.1 One controlled part record, one occurrence graph

Every record must contain:

1. Stable part ID, revision, part family and function; parent assembly; all occurrence IDs; machine quantity; installed quantity; purchase quantity; spares quantity; kit inclusion and duplication rule.
2. Buy/make/modify/reuse classification. For buy: manufacturer, complete order code including suffixes/configuration/grade, length, bore, preload, voltage and accessories. For make: material grade/condition/certificate, drawing revision, all manufacturing features and final inspection. For modify: a purchased blank ID plus the exact qualified secondary operation and inspection. An assembly summary is not another purchase.
3. Exact interfaces: coordinate/datum frame, pin/terminal numbers, gender, handedness, mating order code, drill/pad footprint, thread class/depth, clearances, shoulders, bearing contact faces, keys/slots, end gaps, service/removal paths and tools. Record the mating ID and expected fit interval. Preserve a real closed force/torque/current/heat path.
4. All applicable ratings and sources: working/proof/ultimate/fatigue loads; torque-speed-temperature; voltage/current/transients; ambient/inlet/case limits; duty/lifetime; friction/preload; ingress/contamination; material properties and tolerances. Distinguish manufacturer-guaranteed limits, typical values, supplier claims, engineering assumptions and measured acceptance.
5. Cost/source: exact URL or quote ID, line item, date/validity, stock/lead time, MOQ/pack/tier, unit and extended price, currency, tax basis, freight/destination/incoterm, import/handling, inspection/qualification and expected rejects. Preserve quantity actually purchasable, not a reel/1000-tier price for a one-unit build.
6. Verification links: requirement IDs, model/workpackage IDs, load/fault cases, input/output hashes, inspection/test record and pass criteria. A substitute changes this record and all affected models/tests/manual pages.
7. Assembly definition: orientation, sequence, tools, torque/finish/lubricant, press/heat method, adhesive cure, alignment, measurement and go/no-go. Specify what makes a wrong part/orientation detectable.
8. Maintenance/spares/replacement and requalification consequences; expected availability and an explicitly qualified alternative only if useful. The final build has one selected identity per occurrence, not a menu for the builder.

Audit both directions: every CAD/electrical/wire-harness/manual occurrence resolves to the register; every purchased/manufactured row is used, spared, tooling or explicitly excluded. Check quantities against geometry and harness endpoints, not only duplicated spreadsheets. Conduct mass, power, fastener, connector and wire-end balances as independent completeness checks. Include needed shims, gaskets, sleeves, keys, dowels, nuts, washers, locking elements, plugs, terminals, end plates, jumpers, labels, ferrules, glands, braid clamps, PEstuds, fan mats, guards, bins, feet, anchors, lubricant and fixtures.

### 6.2 Frame, datum and supports

H001, H002, H005/H006 or their redesigned replacements, bed, frame, welds, feet and anchors need real material/cross sections, load paths, datum transfer and tolerance chains. Analyse combined loads, local mount compliance, bench/floor stiffness, twist, thermal expansion and transport handling. Specify weld symbols, sequence, distortion allowances, stress relief if needed, post-weld machining, flatness and inspection. Do not bolt an accurately ground head onto an unqualified flexible or warped table.

Use self-locating shoulders/dowels/repeatable interfaces where they reduce assembly effort and misalignment without unnecessary precision costs. Slot adjusters need controlled travel, locking, access and an objective alignment procedure. Captive/standard hardware can improve serviceability; do not invent exotic precision features when a cheaper qualified arrangement works.

### 6.3 Custom cut axes, rails, blocks, screws and supports

Finish actual rail/block spacing, travel/end reserves, mounting pitches, nut carrier, bearing housing, end-machining and stationary motor receiver. Include lubrication ports, wipers, seals, covers, grease access and carriage removal. Derive individual block forces/moments from the complete six-degree-of-freedom load distribution; include base/block stiffness and preload variation. Do not divide a total force by four without moment reactions.

Verify ball-screw lead/grade, lead error and repeatability over the **used** travel, backlash/preload, axial/torsional stiffness, buckling, critical speed, DN, static capacity, life, drag torque and thermal growth. Supports require exact bearing identities, arrangement/preload, shoulder/fillet/abutment diameters, locknut/clamp and floating-end behavior. A shaft shoulder smaller than the bearing's minimum abutment needs an engineered spacer/contact solution, not a handwave. Where clamp load and operating thrust combine, include both; do not size a spacer for operating force alone.

Recompute hot motor margin with the actual drag budget and inertia. The old conditional running margin1.2688against1.25is tight; adding arbitrary nut/bearing friction can consume it. Use real torque curves at the selected supply/current and temperature, including curve reading and unit variation. Keep5000 N proof and all transverse/vertical cases unchanged. Rails, receiver and couplings must be compatible in real CAD.

### 6.4 H003, H004, H007, F118, blades and cutting joints

Use certified metal for load-bearing parts. Define tool material, heat treatment, hardness, edge radius, grind/finish, flatness, parallelism, 90degree geometry and replacement limits. Include every blade/backing/shim fastener and access path. Develop a grind/inspection quote, not 'tooling supplier' with a blank price. Validate cutting and stripping force, edge wear, coating/adhesion and conductor damage on real wire lots.

The 0.020 mm nominal face gap is an assembled **loaded** functional dimension. Include two holders, backing plates, blade flexure, joints, rails, receivers, datum, bolt settlement and thermal gradients. Derive allowable variation from cut-quality data. No-contact in CAD or a positive isolated-holder gap is not sufficient.

Bolted joints need separate tension, proof, torsion/shear, underhead and washer bearing, thread stripping, slip, prying, separation, settlement and cyclic loosening analyses. Use certified receiving material and actual full thread after tip/chamfer allowances. Do not substitute a proof stress with a higher yield stress, an OD9washer with OD10, or a generic torque with a qualified preload range. Specify the friction/finish process and verify on lot-representative samples.

### 6.5 Feed shafts, housings, bearings, keys, retention and rollers

F101-F112and all rotor variants require actual manufacturing edge breaks, groove/keyway/slot/thread roots, fit diameters, axial stacks and accessible assembly paths. Check actual pulley and roller stations, rotor role, upper/lower hand and180degree rotations. Include8rollers, all bearings, KM2/MB2/rings/keys/collars and secondary operations once. Independently verify blank containment of every machined part.

Bearing fits need measured matched pairs, clearance loss, ball/race temperature, axial freedom, preload, contact compliance, alignment and lubrication. Use manufacturer internal-clearance data for the exact C3/CNpart. Housing restraint must not be an unrealistic rigid bore. Derive a manufacturable fit process with gauges and records; state the grinding supplier if required. Verify wear/fretting and service replacement maintain the same fits without arbitrary mixing.

For urethane rollers, define actual compound/hardness, geometry, bore/key fit, access holes, contact band, loaded effective radius, viscoelastic hysteresis, creep, wear and friction versus temperature/material/contamination. The wire is deliberately offset 5.25 mm from the 20 mm roller midplane to avoid access pockets; preserve or revalidate the lane/tolerance and all OD clearances. Measure both nips per side (all four machine nips), not one average side force. Passive upper rollers do not add driven traction.

### 6.6 Belts, pulleys, tensioners and springs

Verify exact pitch, tooth count, width, bore/key configuration, manufacturer power ratings and engaged teeth. Include tensioning travel, inward/outward reserve, tension measurement method, locking torque and belt replacement access. Use180-200 N, target190, with real temperature/aging/dynamic response. Analyse slack-side positivity, tooth shear, jump, transient torsion, tracking, misalignment and bearing reactions under working, reversal and5 N m fault.

Spring records need rate/preload, tolerance, working stroke, solid height, stress, fatigue, relaxation, attachment/retention and safe stored-energy handling. Verify force windows across the entire wire OD range and manufacturing variation. A catalogued nominal spring does not make all nip/wheel forces nominal.

### 6.7 Measuring modules, encoders, couplings and wire guides

For E60137/RU3100or lower-cost substitutes, verify complete geometry, mounting threads/depth, shaft/bore fit, coupling clamp/torque, shaft load/drag, signal levels, programmable resolution, IO-Link tools/cables and current/temperature range. The commissioning master is a real tooling/cost item. Do not assume a supplied coupling or cable unless exact kit contents prove it.

The measuring wheel needs independent shaft support, actual wheel force, runout/tread profile, loaded scale, thermal/force/wire-lot dependence and usable validity intervals. Model wheel drag and slip, including reversals and strain differences between sensor stations and blade. A high-resolution encoder on a slipping wheel is still wrong. Two similar wheels can have correlated errors. Validate with an independent reference that is not the wheel model or motor encoder being calibrated.

Guides/bushings/funnels need manufacturer bend-radius/material compatibility, finish, wear, retention, clearance for maxODand smallest flexible wire, debris escape and threading access. Specify replaceable inserts and reject limits. Do not put high-friction hidden guides downstream of a carefully designed low-tension buffer without including their drag.

### 6.8 Payoff, dancer, output and waste

Apply section 5to every bearing, shaft, roller, motor, drive, brake, spring, position sensor, limit, end stop, retainer, guard, stand, adapter, anchor and cable. Include safe changeover and torque transfer for both reel modes. Define exact overload/anti-tip proof cases and component cycles; dancer cycles are not necessarily equal to cut count.

The output must physically support and clear200-3000 mm parts. Compare a guided passive tray with a synchronised/torque-limited takeaway or mechanically shared drive, including long flexible-wire buckling, coiling, friction and introduced tension. A loose3munsupported overhang is not a completed handling design. Use an actual flexible-wire contact model and tests; positive rollers/conveyor may cost less than repeated jams. Include folding/modular supports, alignment, guards and full footprint if selected.

Waste needs real stub/slug paths, bins, retention, access and detection. No debris may enter guides, rollers, electrical compartments or next-cut space. Define bin-full and jam recovery; do not require fingers near an energised blade. A pneumatic option includes all air equipment and operating costs.

### 6.9 Motors, drivers, couplings and power supplies

Require exact motor/encoder/wiring variant, torque-speed curves at actual supply/current, hot winding temperature, current RMS/peak interpretation, inertia, holding/fault torque and connection/extension kit. Driver current adaptation and firmware version need parameter readback. Couplings require exact bores, lengths, hub engagement, keyless/keyed configuration, clamp torque, stiffness, misalignment, temperature/thermal conductance, fatigue and service access. A class-price sibling is not a price for the selected bores.

Supply records need continuous rating at actual inlet, mains voltage/frequency/variation, efficiency bounds, ripple, regulation, adjustment, overvoltage/overcurrent/short-circuit behavior, inrush, hold-up, DC-OKthresholds, reverse-current tolerance, PE/classification and clearances. Model actual load profiles and regenerative energy; PSUs are not presumed energy sinks. Include USB and multiple-supply backfeed/startup/shutdown sequences. Do not add a cheaper supply and retain the old source's fuse/drop/thermal analysis.

### 6.10 PCB, field wiring, sensors and connectors

Verify every pin against manufacturer coordinates and electrical function, and every footprint against exact package dimensions, pad/lead/drill/solder process and assembly orientation. Verify MCU socket physical numbering, polarity marks, all omitted pins, component body clearance and mating access. Native netlist agreement is necessary but not sufficient.

For every input/output calculate full voltage/current/temperature/tolerance/aging/noise/injection corners, ON/OFF/leakage, min/max load, logic threshold, hysteresis, timing, power-loss and cable/transient behavior. Qualify sensor type PNP/NPN/contact/optocoupler against the actual interface and protective branches. Add missing pull-downs/defaults as required. A gold-contact relay's maximum 50 mA and a power-contact relay's minimum load are both functional constraints.

Harnesses need endpoint-to-endpoint schedules, terminal numbers, colour, cross section, shield/PE/drain treatment, connector genders, contact/crimp tools, ferrules, strip lengths, twist/route, bend radii, cut lengths and strain relief. Every connector needs its mating half and locking/labeling hardware. Cable-chain/torsion ratings depend on actual motion; fixed cable is not automatically a moving cable. Separate noise sources from encoder/control routes and preserve return paths.

PCB production must specify layer stack, copper, minimum track/via/current, creepage/clearance, temperature rating, drill tolerances, finish, soldering, cleanliness, inspection, electrical test and supplier capability. Do not call a functional photorelay stage a safety isolation system without a complete barrier analysis. Place test points and an executable fixture for every I/O, power domain and fault. Inspect actual solder joints and hot resistor temperatures.

### 6.11 Mains, protection, contactors, PE and functional safety

Define actual site supply, prospective fault current, installation category, disconnect, RCD/MCB/fuse functions, PE bonds and conductor ratings. Select branch devices by DC/ACvoltage, breaking capacity, time-current/I2tand let-through behavior against actual supplies, loads and wiring. Prove clearing of current-limited faults; fuses can fail to clear if the source limits below the relevant trip region.

Contactors need exact poles-in-series topology and manufacturer-supported DCmake/break/endurance under the actual resistive/capacitive/inductive loads. Include welded/stuck contacts, auxiliary/mirror/EDM semantics, coil suppression/dropout delay, control-supply faults and critical-current endurance. Do not mix ordinary NOcontacts with positively guided safety contacts by naming alone.

Conduct an ISO12100-style risk assessment and derive the necessary safety functions and required performance. Assess cutting, crushing, nips, spool rotation, dancer stored energy, conveyor, live conductors, hot surfaces, sharp scrap and service/loading. Use appropriate current standards/editions and local applicability, including relevant machinery electrical, safety-control, guard/interlock, E-stop, safety-distance and unexpected-startup provisions. Section 14names verified starting references; do not infer a PLr merely from having two channels. Evaluate diagnostics, common cause, MTTFd/B10d, proof tests and software/systematic faults using actual manufacturer data. Validate independently on real hardware.

### 6.12 Cabinet, ventilation, guards and human interfaces

Use actual enclosure/mounting-plate dimensions and manufacturer device clearances. Include all fans, outlets, filters, ducts, baffles, gaskets, thermostat/probes, guards, wire ducts, rails, terminal supports, door equipment, PEbraids, labels, glands, blanks, screws and service access. Prevent intake/exhaust recirculation and air bypass. Separate abrasive/conductive cut debris and oil mist from electronics. IP54filter presence does not prove a safe conductive-dust environment.

Guard material/thickness/fastening/hinges/lock/actuator/clearance/openings need real mechanical and reach/impact evidence. No printer-made structural safety guard latch without a valid engineered justification. Ensure service removal does not defeat PEor require live reach. Panel labels/status must be unambiguous for homed, approved, ready, running, paused, fault, scrap and material-change states. The required screenless workflow must work without an always-connected PC.

### 6.13 Consumables, tools, fixtures and support equipment

Every lubricant, adhesive, threadlocker, cleaner, filter, shim, blade and wear item needs grade, quantity, application, cure/compatibility, shelf/service life and procurement. If it changes friction, preload, wire quality or plastic behavior, it is a model input. No arbitrary threadlocker substitution after torque-tension qualification.

List required torque tools, press/heater, pullers, bore/shaft gauges, indicator/straightedge, force gauges, independent length metrology, electrical instruments, flow hood, temperature logger and fixtures. State range, resolution, uncertainty, calibration and whether owned, outsourced or purchased. Treat tooling and one-time qualification separately from per-machine recurring cost, with both shown. Never use 'available shop' as proof of missing precision capability.

## 7. Cost, sourcing and the 70% target

### 7.1 Optimisation objective and comparability

Minimise complete verified delivered build cost subject to **all unchanged requirements**, rather than minimising each headline price in isolation. A cheaper part may need more machining, inspection, filters, drivers, adapters or energy; include those consequences. Produce at least a small Pareto comparison for major alternatives: complete custom versus COTSaxis, payoff modes, cabinet fan layouts, metrology choices and electronics architecture. Select one final baseline.

Reconstruct the old complete baseline on the same quantity, configuration, tax/freight/currency and qualification basis. Define recurring parts/manufacturing cost, one-time tools/NRE/qualification cost and annual operating/service cost separately, then show first-machine and repeat-build totals. The target is approximately`C_new<=0.30*C_baseline`for a declared comparable basket, not automatically a mathematically possible global minimum. If the target is not achieved, continue real alternatives and show the exact gap and binding constraints; do not weaken quality or hide costs to claim70%.

'Cheapest possible' cannot be established across an unobservable global market. Establish the lowest verified **adequate** offer among a broad recorded search, including Chinese factory/distributor routes and local work. Remove dominated choices when the cheaper one is demonstrably equivalent or superior. Maintain alternative/rejection reasons so the search is auditable.

### 7.2 Search standard for every purchased line

Search exact manufacturer/direct and authorised distribution; Alibaba, Made-in-China, LCSC/Chinese industrial suppliers; reputable regional sellers; and local Tunisia manufacturing/sourcing where relevant. Prioritise high-cost/high-risk lines and broad-source the entire inventory. Use actual configured part pages and manufacturer drawings. An Alibaba family listing with a price range is a lead, not an exact configured quote.

Confirm whether the displayed price is for a rail, block, nut, support, coupling or the entire pictured kit. Verify screw lead, grade/preload, total/threaded length, end machining, bearing contents, cable length, voltage and accessories. Preserve screenshots/extracted price facts or formal quotes with date, without inventing a current stock/price from an old search snippet. Refresh final public observations within seven days of publication or use a firm quotation valid on publication; state quote expiry and lead time. For components without a public offer, prepare an exact RFQwith drawing/configuration and receive authorised price evidence before calling the final basket completely verified.

Compare **basket price at actual purchase quantity**. MOQ100passives can be cheaper than 15single pieces; ten expensive screws can cost more than eight at the single tier. Include small-order fees, cut tape/reel fees, connector mates, extension cables, shipping/handling and rejected units. Do not apply a USDunit price to an EURtotal or silently mix tax-inclusive/exclusive sources. Record an exchange-rate date only when conversion is used; keep native-currency subtotals as well.

The repo assumes sous-douane treatment in some old estimates. This is an explicit costing premise, not a demonstrated customs entitlement. Confirm actual delivery/handling/tax treatment or show sensitivity scenarios. Shipping to a European postcode or a free-USshipping listing is not a Tunisia delivered price.

### 7.3 Made parts and production process cost

For every made/modified part, cost stock including machining allowance and nesting/scrap, setup per lot, run time per unit, machine/labour rate, tooling/consumables, heat treatment, grinding/coating/outside work, inspection, assembly, rework/expected scrap, packing and delivery. Record explicit zero only when the operation is genuinely unnecessary or an owned/reused resource has a documented cash-cost treatment. Show economic resource cost separately where useful; blank does not equal zero.

Use actual drawings and achievable tolerances for quotes. A low raw-plate estimate does not price a ground/heat-treated adapter or matched bearing shaft. Compare local insourcing to qualified Chinese custom machining including delivery, rejects and metrology. Batch common materials/fasteners and standardise noncritical dimensions where that reduces cost without affecting precision or access.

### 7.4 Quality and robustness must improve in measurable ways

Before/after comparison must include eliminated deterministic defects, actual force/thermal/electrical margins, protected fault coverage, assembly error opportunities, precision/uncertainty, qualified service interval and availability. Do not claim higher robustness merely because a part is branded, metal, larger, more expensive or cooler in a model with changed assumptions. Cheaper Chinese parts can be selected when exact identity, data, quality controls and qualification support them; they cannot inherit another manufacturer's ratings.

For each major substitution, show purchase/manufacturing saving and all new costs, plus performance evidence. The fan/core comparison, extra input-only subtotal and single cheap ballscrew listing are not a full-machine saving.

## 8. Manufacturing, assembly and service design standards

### 8.1 Drawings and tolerances

Every made part needs a complete native solid and controlled drawing definition: material/condition, datums, dimensions, tolerances, GD&T where functional, surface finishes, threads/full depth/drill point/chamfers, fillets/edge breaks, hardness/coating, quantity, inspection and revision. All holes/features must be accessible to real tools. A schematic plate drawing is not a machining definition. Check raw blank containment, tool reach, clamp/workholding, minimum wall/thread edge distances and post-treatment distortion.

Make tolerance allocation functional. Use worst-case stack bounds for hard interference/strength/energy/safety limits; use justified statistical stacks only for validated manufacturing distributions and the relevant acceptance probability. Preserve correlations from one machining setup. Do not use root-sum-square to erase unknown systematic offsets. Evaluate assembly, adjustment and thermal limits together.

Make assembly deterministic: positive datums, identifiable handed parts, poka-yoke/mating restrictions, standard tools, short independent alignment chains and objective checks. An adjustment must have a finite range, locks, measurement method and recorded target; no 'adjust until it works'. Prove access for tightening, gauging, lubricating, filter replacement, blade changes and bearing/module removal. Simulate installation/removal paths and tool envelopes.

### 8.2 Process qualification and traceability

Use material certificates and actual process inspection for strength/thermal/fit assumptions. Qualify heat treatment/grinding/welds and record the relevant lots. Verify real shaft/bearing matches, not only nominal j5labels. Protect ground surfaces during assembly; use race-specific press support and an appropriate temperature/force method. No force through rolling elements when fitting a race.

Record actual torque/preload and friction lot where required. Give shim selection and reinspection instructions for the loaded blade gap. Freeze cable routing and sensor targets before calibration. Record a build ID covering mechanical/PCB/firmware revisions, critical component lots, driver parameters, wire recipes, calibration and safety validation. Any affecting change invalidates the relevant approval and tests.

### 8.3 Maintenance and degradation

Design a service plan from wear mechanisms, not arbitrary calendar intervals. Initial fan-flow checks each shift for the first ten shifts are a reasonable conservative starting protocol for the earlier candidate; extend only using dust/flow data. Define roller tread/diameter/friction limits, blade edge/gap limits, belt tension and condition, lubrication, brake wear, spool roller traction and sensor cleanliness. Monitor trends where it improves real reliability at low cost.

The final manual must show consumable/spare IDs, access, tools, lockout, replace/check/recalibrate sequence and acceptance after service. Replacing a wheel, roller, blade, bearing, coupling, driver, firmware or tension setting must invalidate/recheck the relevant calibration and approval. A replacement that fits physically is not automatically functionally equivalent.

## 9. Universal simulation and evidence standard

### 9.1 Purpose, credibility and independent oracles

Every model must state the decision it supports, the requirements and exact measurands, the validity envelope, source revision and geometry, units, inputs with provenance, boundary/initial conditions, constitutive/contact/control assumptions, uncertainty, solver/version/options and limitations. Separate code verification, numerical solution verification, physical model validation and application to the final design. Use the NASA-STD-7009B credibility lifecycle as a useful process reference; it does not certify this machine.

The model must not receive the desired truth from the same controller/spec calculation it is supposed to test. A plant returns sensor values from material contact and physical state, not commanded steps. A BOM checker must compare real manufacturer footprints, mating identities and geometric occurrence counts, not only internally duplicated strings. A force/safety model must include the actual path and failures rather than assuming its conclusion in the boundary conditions.

Use analytic benchmarks, manufactured/simple solutions, independent implementations/solvers or coupon data to verify critical calculations. A second mesh generator or solver agreement is valuable, but two models sharing a wrong load or support are not independent physical validation. Keep discrepancy and calibration error explicit.

### 9.2 Numerical convergence and balance

Plan at least three systematic spatial refinements and independent time-step refinements for critical decisions, with local feature refinement at every governing seat/root/contact/edge, not only one convenient bolt. Use Richardson/GCI or a justified error estimate when applicable, and report apparent order, non-asymptotic behavior and raw trends. Tight residuals alone do not establish discretisation accuracy.

Retain the original adapter gates: peak-stress last-pair change<=5%, displacement change<=3%, and peak<=216.67 MPa for the stated certified material/factor. Add a numerical uncertainty allowance to decisions near a limit. Show force/moment equilibrium and work/energy balance; for structural studies use a declared normalisation and target residuals no worse than 1e-7where meaningful. Report contact penetration, frictional work and local reactions as well. Do not hide an unconverged peak by averaging, percentiles, discarding exterior cells or redefining the load patch solely to produce a pass.

Model real fillets, washer contact and material behavior where ideal sharp discontinuities produce mathematical singularities. If a raw peak cannot converge under a physically defensible model, the design/stress gate remains unresolved; do not waive it with 'FEM singularity'. A justified alternative failure measure requires a documented engineering change and validation, not a convenient plot exclusion.

For CFD/thermal models, track heat and mass balances, domain/mesh/boundary sensitivity and device-inlet temperatures. A proposed numerical target is energy imbalance<1%, mass imbalance<0.1% and critical temperatures changing<0.2 K on final refinement; tighten these if the remaining margin requires it. These are handoff numerical-quality targets, not manufacturer or regulatory limits. For dynamics, choose step size from the highest retained excitation/mode/contact timescale and demonstrate response/phase/peak convergence; multi-rate methods need verified coupling errors.

### 9.3 Uncertainty, robust optimisation and coverage

Separate aleatory variation from epistemic uncertainty. Use bounded corners/intervals for hard limits, correlated manufacturing/thermal/friction inputs, global sensitivity, space-filling samples and targeted optimisation of worst cases. Sampling alone does not prove a bound in a high-dimensional domain. Search for minima of each margin function using physical adversarial cases and validated surrogates; confirm limiting cases in the high-fidelity model.

Use at least physically coupled full/mid/empty reel states, wire extrema and intermediate constructions, tolerance/material/property bounds, cold/hot, clean/dirty, aging/wear, voltage extremes and all commanded/fault load signs. Do not combine incompatible states without labelling a deliberately conservative envelope. Include model-form discrepancy and parameter-identification confidence. Do not assume independence merely because a random generator can draw inputs separately.

A useful exploration may begin with hundreds or thousands of parameterised low-fidelity cases and an adaptive set of high-fidelity extremes; the sample count is not an acceptance criterion. Set sample size from desired tail/confidence/error precision and simulator validity. Rare-event importance sampling/subset simulation may improve numerical efficiency, but needs correct likelihood weights, independent repeats, effective sample size, coverage diagnostics and physical plausibility. A billion idealised runs cannot prove omitted physics.

### 9.4 Reproducibility and negative controls

Every executed study must emit a machine-readable manifest: input/source/toolchain hashes, geometry/mesh checksum, seed or deterministic ordering, solver options, material tables, load-case IDs, stdout/stderr, exit code, timing, raw fields/time histories, convergence/balance data and individual pass/fail margins. Store failed runs and reasons; do not delete awkward outputs or reuse another revision's pass. Scope-limited test exits must not conceal unrelated full-design failures.

Add meaningful rejection cases: wrong component/lead/grade, undersized receiver, wrong connector/pin, shallow thread, low friction, wrong encoder location, missing sensor, stuck bus, wrong polarity, blank/duplicate/fabricated measurements, failed correction, overvoltage and clogged flow. Each should fail for its intended independent reason. Maintain the distinction between a passing negative-test harness and a functioning machine.

Use a multi-fidelity programme to avoid wasting effort: analytic bounds and cheap coupons first, then component/contact/field solves, then integrated flexible multibody/control and HIL, then whole-machine correlation. Increase fidelity when an omitted effect can consume the allocated margin. Do not run a huge model with unknown boundary conditions as a substitute for identifying them.

## 10. The mandatory simulation and analysis programme: 48 work packages

These are **work instructions for the completion agent**, not a claim that 48 studies have already run. Use one register with IDs M01-M48, linked requirement IDs, owners, prerequisites, input revisions, model class, execution state, exact decision, pass/fail margins, physical correlation and retained artifacts. Each item inherits section 9. A simple model may close an item if a conservative independent bound demonstrates the decision with adequate margin. A high-fidelity solve is mandatory when the simplifying assumption is not conservative or its uncertainty can consume the margin. Explain that choice; do not silently omit the physics.

The universal case axes are wire construction/size/OD/lot, finished length and both independent strips, small/large reel full/mid/near-empty, speed/acceleration/direction, voltage/frequency, cold/hot/transient, clean/dirty, tolerances, friction, wear and faults. Test their important **interactions**, rather than running each in isolation. For every requirement define a signed margin: positive means the demonstrated lower/upper bound meets the requirement after numerical, manufacturing, measurement and model uncertainty allowances. Maintain a vector of margins; an aggregate score may never compensate a failed hard limit.

### M01 — Requirements, units, configuration and data lineage

**Decision and inputs:** establish the single active design contract before optimising it. Import all 47 goals and later user instructions, the exact CAD coordinates, component revisions, firmware scales, electrical nets and model material identities. Explicitly distinguish working, proof, fault and ultimate loads; rated operation from storage; RMS from peak current; phase current from supply current; steps from quadrature edges; mass from force; degrees from radians; ambient air from case temperature.

**Cases and checks:** trace each active consumer of every critical constant. Deliberately perturb screw lead, pulse count, wire height, encoder sign, a part revision, a connector position and one kit quantity. Detect the affected CAD, analysis, firmware, manufacturing, cost and manual outputs. Check ranges and dimensional consistency at import boundaries; reject missing/NaN/infinite values and unlabelled unit conversions. Validate CSV width and structured schemas before totals.

**Acceptance and artifacts:** one versioned requirements register, source-of-truth parameter set and dependency graph; zero ambiguous active scales; every original goal has a verification method and evidence owner; every released output identifies its input hashes. Rebuilding from the same inputs reproduces semantically identical geometry, tables and binaries, allowing documented nondeterministic metadata only. Mutation checks must fail for the intended reason. This establishes consistency, not physical validity.

### M02 — Complete CAD, continuous kinematics and installation/removal

**Inputs:** final solids for every made part and purchased interface, fastener tips/heads/washers, actual screw/nut/support/guide internals, springs, wire, guards, cabinet, harness bend radii, moving cable envelopes, reel adapters and output support. Model the **independent** left/right jaw coordinates, nip-opening mechanism and dancer travel.

**Cases:** homing with unknown initial position, both jaws independently at all travel combinations, cutting, asymmetric stripping, wire threading, jam recovery, spool loading, maintenance and end-stop fault. Check minimum/maximum wire and tolerances, shortest part, largest strip, driver/fan intrusion, rail and motor access. Use continuous collision detection or adaptive interval bounds; a few sampled poses cannot exclude a collision between samples. Separate intended contact from interference and declare minimum functional clearances including deflection and thermal growth.

**Acceptance:** real load/motion paths fit; each assembly and removal path exists using listed tools and actual fastener insertion directions; no trapped pulley, washer, bearing, inaccessible nut or cable bend. Inspect STEP readback by assembly occurrence, topology, volume, bounding dimensions, coordinates and part identity. Retain interference pairs, limiting poses and true exploded views. An independent builder must reproduce the sequence in M45 and section 17.

### M03 — Manufacturing, geometric and thermal tolerance propagation

**Inputs:** actual supplier tolerances, machining datums, screw end lengths, bearing fits, pilot/PCD errors, guide straightness and parallelism, blade/backing thickness, spring locations, wheel runout, encoder shaft geometry, lead and thermal coefficients. Include inspection uncertainty and machining setup correlations.

**Cases:** worst-case intervals for contact/interference, thread engagement, grip margins, nip travel and blade gap; correlated stochastic analysis for a validated production distribution. Include cold assembly, hot steady state and differential transient. For perspective, a 440 mm steel/aluminium differential chain with coefficient difference 11e-6/K and 20 K change can move about 0.0968 mm: do not dismiss thermal expansion compared with a 0.020 mm gap or +/-0.20 mm strip tolerance.

**Acceptance:** complete stack diagrams and derivatives identify governing contributors; all hard limits pass at allowable extremes or the drawing/adjustment method is changed. Statistical yields require evidence of distributions and confidence, not arbitrary normal inputs. Generate inspection/selection rules that a shop can actually execute at the required resolution. Validate with measured part stacks and hot/cold dial/laser measurements. Replacing an unexplained tolerance with zero is a failure.

### M04 — Whole-machine static and proof load path

**Inputs:** the actual assembled stiffness and load application areas from the wire/blades through holders, backing, carriage, guide blocks, screw/nut/support, stationary receiver, motor adapter, base, frame and floor. Include gravity, belt loads, full reel and eccentricity, cable forces and clamping preload.

**Cases:** all eight sign combinations of the three head working components, 2500/750/375 N; corresponding double proof components without weakening the original interpretation; each jaw alone and both together; 1 N m feed with 2.5 reversal factor, 5 N m fault, +/-480 N belt and +/-60 N roller in both planes; actual 75 N maximum nip preload wherever relevant. Preserve the original load definitions and distinguish self-equilibrating internal loads from external reactions. Assess simultaneous physically credible loads and separately labelled conservative envelopes. Add hard-stop/stall bounds from M18/M34, which may exceed these minimum screens.

**Acceptance:** stress, separation, yielding, contact pressure, displacement and blade-to-wire error all meet their individual gates. Proof must not introduce permanent change that prevents later accuracy. No falsely rigid cut-through support; include real mounting flexibility. Hand force/moment equilibrium and simple beam/plate bounds must agree with the integrated solution. Retain reactions for every downstream component and instrument a representative assembled proof test with pre/post geometry checks.

### M05 — Bolted joints, washers, receiving threads and contact

**Inputs:** actual screw class/length/thread tolerances, full thread depth, incomplete tips and runout, countersinks, washer geometry, material certificates, surface finish/coating, lubrication, torque-tool uncertainty, thread/underhead friction and settlement. R7's proposed 0.10-0.18 friction band is a condition to qualify, not a property of all screws.

**Cases:** min/max torque and both friction extremes, nonuniform preload, worst washer eccentricity, receiver flexibility, thermal preload loss, transverse reversals, head torque, rear motor loading, proof and repeated assembly. Resolve annular washer contact, counterbore floor, head bending, local plasticity, interface lift/slip and receiving-thread stripping. Compare torque-to-preload analytics against nonlinear contact FEM and a torque-tension coupon. Include prevailing torque/threadlocker in the actual tightening method if used.

**Acceptance:** screw proof and fatigue, thread stripping, washer/floor bearing, joint slip/separation and residual preload all pass after uncertainty. The actual thread engagement and drill-tip clearance pass at every stack extreme. Peak stress must meet the original converged adapter gate; do not import lost R7 results as evidence. Deliver joint cards with parts, finish, preparation, tightening order, torque tolerance, reuse rule and inspection. Physical proof includes marked-interface slip measurement and retained preload/geometry, not simply 'bolt did not break'.

### M06 — Ball screw, supports, guide groups and axis internal mechanics

**Inputs:** exact 10 mm-lead screw/nut grade/preload, root diameter, support bearing arrangement and ratings, axial/rotational stiffness, actual machined ends, locknut, seals, lubricant, guide block coordinates and certified capacities. Include C7 lead error if considering C7; it does not inherit a C5 claim.

**Cases:** working/proof/stall thrust, both directions, eccentric head moments, full stroke and minimum bearing spans, hot/cold lubricant, misalignment and preload scatter. Calculate screw buckling/critical speed/DN, support axial capacity, guide-group force/moment sharing, carriage and rail mounting compliance, life with the real reversal spectrum and support/guide friction versus motor reserve. The 4900 N support cannot pass a 5000 N proof by rounding. At 8.5 N m and 10 mm lead, an efficiency-one thrust bound is 5340.708 N; actual motor current/torque tolerance or impact can raise the design bound further.

**Acceptance:** every actual component and receiving feature meets strength, life, stiffness, speed and drag criteria at the same time. Prove a real four-block layout fits, rather than dividing a moment by a convenient catalogue number. Derive locating/floating bearing strategy and assembly alignment/preload procedure. Validate drag, backlash, reversal, lead map and proof on the built axis; retain supplier drawings and measured torque/displacement traces.

### M07 — Shafts, fits, bearing contact and collars

**Inputs:** actual machined shaft sections, fillets, threads/keyways/retaining grooves, heat treatment, roughness, surface damage, bearing internal geometry/clearance class and measured paired seat diameters. Model both driven lower and passive upper rollers correctly; passive upper rollers do not add commanded drive torque.

**Cases:** original 191-section audit plus adaptive checks around every discontinuity; both bending planes and torque reversals, belt/nip loads, 7 kN collar clamp plus 5 N m fault, fit extremes, housing elasticity and coupled temperature fields. Account for bearing contact load zones, race expansion, clearance consumption and induced friction. A rigid cylindrical clamp is not a validated bearing contact model.

**Acceptance:** no stiffness loss relative to the required reduced model, continuous section bound or adequately refined geometry, fatigue and contact margins, acceptable cold/hot internal clearance and actual 3-6 um matched interference where specified. Verify F109 torque transfer, key/contact slip, local yielding and fretting. Instrument representative fits and measure assembly force/clearance/drag and hot differential temperature; record individual matched pairs. Include a practical replacement/matching route and its cost.

### M08 — Blades, backing, incision depth and loaded face gap

**Inputs:** certified blade and backing materials, hardness/toughness, grind geometry, actual edge radius, contact surfaces, shims, clamping, holder stiffness and all tool-to-wire datum chains. Use cut forces identified in M25 rather than an unvalidated generic force.

**Cases:** both jaws alone/together, transverse and vertical force signs, full/minimum wire, incision without severing strands, strip pull, cut completion, hot/cold and worn edge. Resolve local edge/support deformation and load transfer through H007/backing and real fasteners. Couple the complete assembled model's relative displacement to the cut/strip model.

**Acceptance:** retain the 0.020 mm nominal **loaded** face gap; derive an allowed actual interval from demonstrated cut quality, tool manufacturability and damage avoidance, then allocate its budget. Verify shimming range/resolution and measurement access. Nominal unloaded CAD spacing is not acceptance. Inspect blade grind/edge/backing and measure gap/relative displacement under qualified loads; correlate cut quality and edge life. Specify the exact regrind/replacement threshold, shim procedure and recalibration test.

### M09 — Stability, buckling, slack and support limits

**Inputs:** screw and frame slenderness, actual support compliance, 3 m wire bending/torsional stiffness and initial curl, guide spacing, reel stand footprint, centre of mass, caster/feet states and floor slope.

**Cases:** Euler bounds followed by geometric-nonlinear imperfection cases where relevant; reverse feed, pushing 0.5 mm2 stranded wire into a tray, a jammed discharge, unbalanced full reel, spool loading and operator service forces. Stranded-wire effective bending stiffness is not automatically solid-copper EI. Include loss of tension and lateral contact.

**Acceptance:** no screw/frame/stand instability at the applicable load and uncertainty; wire support/conveyor prevents uncontrolled loops, snags or compressive length errors. Quantify tipping/sliding/anchoring margin and practical load procedure. Validate with worst-curl wire and the real 3 m route, and load the stand using the declared full reel and loading method. An unsupported 3 m push cannot be approved from a rigid straight-line animation.

### M10 — Multiaxial fatigue and load-spectrum damage

**Inputs:** material-specific S-N/strain-life data and scatter, heat treatment, notch sensitivity, surface/size corrections, residual stress, weld detail class, mean load and the actual time history of bending/torsion/preload. Keep low-cycle proof/jam events distinct from high-cycle running loads.

**Cases:** rainflow or justified event-counting of realistic usage mixtures, repeated reversals, starts/stops, belt modulation, blade cycles, reel changes and bounded jams. Include mean-stress and multiaxial interaction appropriate to the failure mechanism, not automatically independent scalar Miner sums. Assess bolts, shafts, springs, welded frame/stand and support parts.

**Acceptance:** demonstrate the required 10,000 h / 1.8e7-cut design duty with a stated spectrum, reliability basis and uncertainty. Those numbers imply 0.5 cuts/s averaged over that duty; they cannot imply the same instantaneous rate for every 3 m part. Preserve the requirement while defining an achievable workload. Validate critical strain/load predictions and representative endurance; unexplained material properties or fatigue scatter cannot be replaced with optimistic handbook means.

### M11 — Tribology, lubrication, wear and friction evolution

**Inputs:** actual roller tread material/hardness, wire PVC composition, pulley/guide finish, bearing seals/lubricant, screw/rail lubrication, blade wear and environmental debris. Identify load, sliding speed, temperature, contact pressure and contamination effects from supplier/coupon evidence.

**Cases:** run-in, steady wear, dirty/dry/wet surfaces if inside scope, dust embedded in tread, tread glazing, low-friction wire, long dwell compression, bearing/guide drag increase and lubricant depletion. Model changing radius, traction and backlash jointly. Use Archard or other wear laws only after calibrating their coefficients to this contact pair and regime; otherwise provide conservative service/inspection bounds.

**Acceptance:** derive useful-life and replacement limits that preserve speed, grip, marking, metrology and thermal margins. Feed-radius wear must be compared against the current 0.1056 mm nominal radius headroom at the 8000 Hz ceiling. Validate trends by actual accelerated-but-physically-representative coupon and machine tests. Do not claim lifetime from one assumed coefficient or assign lubrication intervals without a duty/environment basis.

### M12 — Polymer viscoelasticity, creep, relaxation and aging

**Inputs:** PVC jacket and roller-tread time/temperature behavior, printed/moulded guides if used, nylon spacers/screws, belt properties and elastomer mounts. Use actual grades and temperature/moisture histories, not a generic 'plastic' elastic modulus.

**Cases:** 40 C room plus local heat, nip dwell overnight, repeated 50-75 N loading, jacket deformation, tread compression set, nylon screw relaxation, belt tension drift, humidity and cleaning chemicals. Where needed fit a Prony/creep model from coupons over the relevant duration; time-temperature superposition requires a justified material regime and shift factors.

**Acceptance:** residual nip force, screw retention, dimensional calibration, belt tension and jacket quality remain within requirements until the stated service interval. Include rest-time effects on relaxed part length and indentation recovery in metrology. Validate initial and aged parts, and choose metal/other materials where a cheap polymer creates uncontrolled long-term drift. No unqualified printed structural part may carry a precision/safety load merely because it fits.

### M13 — Assembled prestressed modal behaviour

**Inputs:** final full-machine mass/stiffness, contact/preload, real foundation and cabinet/frame coupling, wire/nip constraints, motor/rotor inertia, reel states and open/closed guard conditions. A fixed-base isolated shaft is only one submodel.

**Cases:** cold/hot, joints at preload limits, full/empty reel, belt-tension bounds, practical bench/floor supports and service states. Calculate mode shapes and participation/relative motion at blade edges, wire support and measuring wheels. Check mesh/mode-count convergence and rigid-body modes. Include prestress/contact effects where they change governing modes.

**Acceptance:** identify every mode that can couple to operating excitations; use response-based accuracy/stress criteria in M14 rather than an arbitrary 'natural frequency above X' alone. Validate mode frequencies, mode shapes and damping using an instrumented impact/shaker test on the actual assembled machine and representative reel. Retain sensor locations, calibration, windows, coherence, FRFs and model updating rationale. Do not tune arbitrary stiffness/damping solely to conceal a discrepancy.

### M14 — Harmonic response, operational vibration and speed maps

**Inputs:** M13 model plus actual rotating imbalance/eccentricity, belt tooth modulation, motor detent/current harmonics, screw/guide excitation, fan forces, cutter cycle and reel disturbances. Nominal belt tooth-pass is 79.5775 Hz at 0.50 m/s for the stated drive; derive all orders from actual tooth counts and speed.

**Cases:** Campbell/order maps over acceleration/deceleration and approved speeds; all relevant harmonics, modulation/sidebands and interacting source phases. Vary damping over measured confidence bounds and evaluate full/near-empty reel and dirty/faulted fan imbalance. A no-resonance check at one rated speed is insufficient because acceleration crosses resonances.

**Acceptance:** relative blade-wire and wheel-wire vibration, fatigue stress, measurement bias and noise meet allocated budgets in steady and transient operation. Track phase as well as amplitude; do not add incoherent sources coherently or RSS correlated ones without justification. Validate with accelerometers and a suitable displacement/laser/reference measurement, tachometer/order tracking and actual cuts. A large absolute frame acceleration is not interchangeable with the small relative displacement that controls accuracy.

### M15 — Transient shock, cutting impact, jams and stops

**Inputs:** moving masses/inertias, measured cut-force rise time, stiffness/contact restitution/damping, motor current control, belt/screw windup, mechanical stops and safety delay distributions. Include kinetic, magnetic, gravitational and elastic stored energies.

**Cases:** hardest qualified cut, stalled blade, misthreaded wire, abrupt feed snag, reel brake engagement, dancer hitting an end stop, ordinary stop and E-stop at every relevant phase. Consider asymmetric loss of one drive and release after a trapped load. Use explicit/implicit dynamics as appropriate; demonstrate time-step, contact and energy balance convergence.

**Acceptance:** no component load exceeds its validated fault/proof limits, no dangerous fragment or released reel/wire, no guard release before hazard cessation, and recovery cannot count a damaged part as good. A commanded low acceleration does not limit a physical jam impact. Where uncontrolled impact exceeds the structure, implement and qualify a genuine energy/torque limit or compliant absorber. Validate with safely contained instrumented tests and correlate peak/duration/load path, not just final position.

### M16 — Belt, torsional, rotor and drive-train dynamics

**Inputs:** actual pulley/tooth/belt dimensions and stiffness, 180-200 N initial tension, shaft/coupling stiffness/backlash, rotor inertias, screw torsional compliance, alignment and real support bearings. Separate the two driven lower rollers from passive upper rollers and establish the actual power split.

**Cases:** reversals, simultaneous nip traction extremes, 1 N m running with reversal factor, 5 N m fault, belt +/-480 N, hot/worn belts, pulley runout, tooth engagement and torsional resonance. Include shaft critical speeds/whirl where credible; a low operating speed can close that item with a conservative analytic bound. Evaluate coupling misalignment losses, heat flow and torsional lag together; a thermally isolating coupling cannot be selected independently of stiffness and torque.

**Acceptance:** no tooth jump/slip, unacceptable backlash/windup, bearing overload or tension loss; actual transmitted torque and material motion remain inside accuracy and life budgets. Validate belt tension and force sharing, shaft twist/reversal response and loaded tracking. Provide exact tension measurement span/frequency/force method, not only 'set to 190 N', and verify that tightening/adjustment access exists.

### M17 — Foundation, bench, frame coupling and acoustic response

**Inputs:** actual bench/frame/floor stiffness and damping, feet/anchors/casters, reel stand coupling, enclosure panels, fan mounting, service openings and surrounding clearance. Include the installation the builder is instructed to use; an infinitely rigid laboratory base cannot qualify an unspecified bench.

**Cases:** approved support extremes, uneven floor/shimming, full reel, periodic drive/cutter/fan forcing, loose-but-credible service panels, startup and stopping. Evaluate structural/acoustic transmission, panel resonances and wire-entry/output motion. Use measured FRFs as boundary data when floor/bench properties cannot be reliably derived.

**Acceptance:** installation limits and any anchoring are explicit; precision margins remain valid and noise is measured/declared against the risk assessment and applicable rules. Compare low-cost stiffening/isolation by relative tool-wire motion and stability, not by subjective quietness alone. Verify that isolating the cutter does not create harmful motion relative to a separately supported payoff or output conveyor. Preserve the environmental and operator measurement geometry in the evidence.

### M18 — Hot motor/driver capability and nonlinear motion dynamics

**Inputs:** exact delivered motors, winding/current convention, V20 driver configuration, bus voltage limits, real torque-speed/current-temperature data, inertia, guide/screw/seal drag, efficiency and load spectrum. Distinguish continuous operating torque, transient pull-out/holding torque and current-regulated stall torque. Catalogue room-temperature holding torque is not a hot moving torque guarantee.

**Cases:** feed 0.05 m/s commissioning through 0.50 m/s, 260 rpm ceiling, minimum loaded radius, head 12.5 mm/s and 300 mm/s2, voltage minima, hottest inlet/winding, cold viscous lubricant, inertia/cutting extremes and parameter tolerances. Include both axes together, driver current adaptation/idle reduction, following-error behavior, acceleration reversal and loss of encoder/logic. Current and torque tolerances can exceed a nominal 8.5 N m holding value: derive the actual fault upper bound.

**Acceptance:** required torque/force with the original minimum running margin remains available throughout the trajectory, with no lost steps/following error, rating exceedance or hidden speed reduction. The earlier calculated head margin 1.2688 against 1.25 is narrow; reallocate all added drag honestly. Validate hot loaded torque/response and actual driver parameter readback; record motor case/winding inference, supply voltage/current and position. If drive limits are used for structural protection, show they are enforceable under the relevant faults or add a real independent limit.

### M19 — Complete loss budget and thermal resistance network

**Inputs:** final bill of power consumers, supply efficiency curves versus load/temperature/voltage, driver switching/conduction losses, actual motor heat split, fans, coils, protection, PCB resistor banks, expander and payoff electronics, bleeders/braking devices and stand-by loads. Include heat conducted into the cabinet and solar/process sources if inside the installation scope.

**Cases:** continuous high-load mixed duty, idle holding, worst cut/feed overlap, low efficiency corner, voltage/frequency extremes and fan/control modes. Measure or conservatively bound missing loss functions; an allocation is not a guaranteed maximum. Build a heat-path network for device junction/case/inlet, cabinet air and enclosure wall before expensive CFD.

**Acceptance:** all losses reconcile with measured electrical input minus mechanical/output power and stored-energy change, within instrument uncertainty; no omitted payoff or PCB heat. Preserve 40 C room operation. Evaluate ventilation cost and fan heat **together**: a smaller efficient fan arrangement can lower heat and cost, but must prove installed flow and fault margins. The prior 650 W/800 m3/h is a candidate, not an immutable goal or a measured fact. Retain the assumptions that produce density>=1.05 kg/m3 and verify the actual altitude/pressure/humidity envelope.

### M20 — Cabinet airflow, device hotspots, filters and conjugate heat transfer

**Inputs:** final 3D cabinet, fan/outlet cutouts and curves, actual filter clean/dirty pressure drops, device heat maps, fins/orientation, wiring bundles, baffles, leakage, wall conduction and local air properties. Include the 150 mm PF66000 intrusion and real service clearances. Match the exact fan voltage/frequency and sleeve-bearing orientation/life conditions.

**Cases:** clean installation, service-limit filters, nonuniform blockage, bypass/leakage, fan failure, reverse/incorrect mounting, blocked outlet, inlet recirculation, one hot device, cabinet door positions and fan speed/voltage variation. Solve the intersection of fan P-Q and system resistance; parallel paths do not automatically deliver twice a single free-air value. Examine recirculation and stagnant zones rather than only average air rise.

**Acceptance:** every device inlet/case/junction respects its individual normal and fault rating with uncertainty. Candidate normal hottest inlet<=45 C and selected V20 inlet<=50 C; stricter PSU/relay/sensor limits still apply. At the earlier 44.286 C estimate, only 0.714 K remains to a 45 C normal gate; unproved 1.5 K hotspot and density assumptions cannot consume that invisibly. Validate installed volumetric/mass flow, pressure drop and a spatial temperature map at measured loss. Correlate CFD predictions and sensor placement to real data; a vane presence switch is not an 800 m3/h measurement.

### M21 — Thermal transients, sensor lag and protective shutdown

**Inputs:** thermal capacitances/contact resistances, individual device time constants, thermostat sensor error/lag/filtering/location, relay thresholds/dropout times, software and hardwired delays, motor residual heat and fan coast behavior. Derive sensor-to-critical-hotspot transfer functions from M20 and measurements.

**Cases:** startup already hot, full-load step, partial clog with both fans still detected, one fan lost, both lost, thermostat probe open/short/stuck, low 24/5 V, logic-loss sequence, welded permission contact, power restoration and post-trip heat soak. Evaluate spatially adverse airflow patterns, not only uniform reduction. Include failure modes where the sensor cools while a driver hotspot heats.

**Acceptance:** the complete measurement/trip/dropout/stop/soak trajectory stays below all relevant operating limits and does not unlock guards prematurely. Recompute the candidate 45+2+1.5+0.5=49 C bound using **measured, separately justified** error/mapping/overshoot terms. Simulate tolerance and fault corners, then physically test section 11.3. State which protective functions are equipment protection and which are safety-related; ordinary thermostats cannot acquire a safety rating by appearing in two circuits.

### M22 — Motor-to-shaft, bearing and housing thermal paths

**Inputs:** measured/catalogued motor loss and heat split, shaft/coupling geometry and conductivity, flange contacts, bearing friction and lubricant behavior, race/housing contact conductance, convection and structural conduction. Do not prescribe a housing at room temperature without demonstrating that boundary.

**Cases:** maximum approved continuous/reversal duty, idle holding, hot motor start, stopped fan, worn bearing, different fit/contact pressures, coupling variants and uncertain thermal contacts. Resolve axial/radial gradients at both bearing seats. Couple heat generation to preload, clearance and friction; iterate to a stable solution or identify a thermal runaway possibility.

**Acceptance:** the specified shaft rise<=20 K above the relevant measured housing holds across the operating envelope, while remaining clearance, torsional stiffness and all bearing temperatures pass. The stored 29.6603 K result is a failing conditional case, not a reason to change the requirement. Validate motor/flange/coupling/shaft/race/housing temperatures and warm-up/cool-down curves with suitable non-perturbing instrumentation. Derive an actual design change, such as a qualified heat path/coupling/bearing arrangement, and recheck M03/M07/M16/M18 rather than reducing an arbitrary imposed heat source.

### M23 — Thermoelastic accuracy and calibration drift

**Inputs:** M20/M22 temperature fields, actual coefficients, anisotropy where relevant, component constraints, roller/wheel radius-temperature dependence, screw lead and sensor bracket expansion. Include differential left/right heating and wire temperature/tension relaxation.

**Cases:** cold start, drift during production, hot restart, fan cycling, one-sided motor loading, calibration at one temperature followed by operation at another, and maximum differential gradients. Propagate strains to finished relaxed length, both strip lengths, blade-face gap, wheel phase/runout and bearing fits using the same coordinates as the controller and drawings.

**Acceptance:** residual errors after any **implemented** compensation fit internal guarded budgets throughout time, not only at final steady state. A temperature-compensated model provides no benefit unless the required sensor, algorithm and approval invalidation are real. Validate drift against independent dimensional measurements, record warm-up policy if needed and include its impact on screenless operation and throughput. Requiring an unknown builder warm-up ritual is not a completed design.

### M24 — Humidity, contamination, condensation and local PCB heating

**Inputs:** actual site temperature/RH/pressure and storage/transport profile, dust size/type/loading, insulation/copper debris, enclosure/filter grades, PCB geometry/coating and heat sources, corrosion susceptibility and connector/photorelay limits. Do not silently reduce the RH envelope to suit a 70%-limited candidate.

**Cases:** long hot humid operation, cold-to-warm condensation if credible, conductive debris ingress, blocked filter, coating defects, corrosion/cleaning, local resistor-bank heating and PCB airflow variations. Use appropriate simplified deposition/dewpoint/thermal models before any particle CFD. Local TLP ambient is board/device environment, not cabinet bulk air.

**Acceptance:** no loss of insulation, leakage-induced input error, unsafe corrosion, filter bypass or unqualified component environment. The current Toshiba photorelay sheet has a 50 mW input-power limit derated 0.5 mW/K above 25 C: at 60 C it allows 32.5 mW, below the older candidate's 32.893 mW worst LED power. Re-evaluate the LED series resistor independently of the bleeder; a 2.7 kOhm/1%/1 W candidate is worth checking for lower heat and adequate guaranteed hot/aged trigger, but is **not** selected by this handoff. Retain source-revision-specific limits and perform hot-all-inputs-on tests. Qualify coating/process if used; do not imply that a filter or component isolation voltage makes a whole board humidity-proof.

### M25 — Wire cutting, incision, stripping and material damage

**Inputs:** actual conductor strand count/diameter/lay, copper temper, PVC formulation/thickness/temperature/rate behavior, adhesion, wire eccentricity, blades' measured edge geometry and friction, backing stiffness and tool wear. Cross-section area and OD alone do not determine stripping forces or strand damage.

**Cases:** material/lot extrema and risk-based intermediate constructions, minimum/maximum incision, both ends, 3/25 mm and asymmetric strips, disabled ends, hot/cold, curled wire, worn/contaminated tools and maximum qualified speed. Begin with instrumented cut/strip coupons. Only use nonlinear elastoplastic/fracture/cohesive/explicit models after identifying appropriate parameters; mesh-dependent element deletion is not an independent conductor-damage prediction.

**Acceptance:** peak/time-dependent force stays inside the actual axis capability; no unacceptable nicked/severed strands, pull-out, insulation residue, jacket tear or end deformation. Freeze measurable inspection and any tensile/electrical acceptance appropriate to the declared use; obtain applicable customer/application criteria rather than claiming an arbitrary electrical standard. Validate force traces, cut surfaces, strand microscopy and strip dimensions. Establish a recipe/edge-wear envelope with safe adjustment limits and actual software enforcement. Retain failed coupons as well as good ones.

### M26 — Roller/wheel contact, traction and wire strain

**Inputs:** actual two nips per feed cassette, formed by two driven lower rollers and two passive upper rollers (four nips/eight rollers for the machine), measured normal-force/travel curves, tread mechanics, wheel/encoder drag, guide friction and wire axial stiffness. Use actual minimum friction evidence; do not count passive rollers as independent drive traction.

**Cases:** each nip 50-75 N, wheels 6-19 N, every OD, tolerance/misalignment, contamination, worn tread, acceleration/reversal, stripping and buffer tension. Include rolling indentation, creep, slip thresholds, jacket marking and elastic extension/relaxation. For two measuring modules, a 0.01 N m encoder starting torque divided by 0.03183 m radius is about 0.314 N **per module**, before bearing/guide losses. At a 6 N wheel force and friction coefficient 0.1 the available tangential friction is only 0.6 N: a low-tension dancer does not remove this drag or its uncertainty.

**Acceptance:** all real contacts retain motion/accuracy authority, no damaging indentation or stretch and sufficient hot torque; roller effective radius stays within the qualified speed/calibration range. Validate force, drag, loaded circumference, slip and jacket recovery on actual wires and surfaces. Define six force stations (four machine nips and two measuring wheels), each at both OD boundaries as a minimum twelve-row force record, plus both belt tensions separately. Preserve individual values rather than an average that hides one weak nip; extend the data for force/travel/hysteresis/temperature cases.

### M27 — Independent material/event plant and sensor observability

**Inputs:** actual wire topology, cut/insulation boundaries, material points, compliance/slack, contact occupancy, real encoder positions and direction, sensors, blades, nip states and waste/output routes. The controller may command actuators; only the plant determines actual material movement and observations.

**Cases:** entire prime/feed/cut/backfeed/strip/eject/clear cycle, both strip-disable combinations, minimum length/maximum strips, pauses/retries, slip, broken wire, no wire, welded/slipping nip, encoder missing/stuck/reversed/noisy and wrong sensor location. Model the instant severing creates two independent bodies; stock-side sensors cease to observe the piece. Preserve the supplied counterexample on unchanged upstream as a negative control. In a revised plant, record occupancy-valid intervals and origin transfer for each encoder; invalidate counts outside contact rather than fabricating them.

**Acceptance:** every decision required for an accepted piece is physically observable or conservatively verified by a real alternative. Produce state-by-state observability and error-detection tables with known blind spots, latency and fault coverage. Prime waste, both slugs and finished parts must have distinct reachable removal/detection mechanisms. Compare plant sensor traces with actual video/encoder/logic captures. A passing controller simulation that derives truth from commanded motion fails this work package.

### M28 — Reel mass/radius/inertia, drive, brake and structural dynamics

**Inputs:** frozen reel envelope and actual empty-spool/flange masses/inertias, winding geometry/packing, eccentricity, flange stiffness, drive roller or arbor interfaces, motor/gearbox/brake data and floor loading. Include the small-reel cassette and all real adapters.

**Cases:** coupled full/mid/near-empty states, minimum/maximum core/flange/width/bore, low rim friction, warped flange, eccentric winding, start/stop, stuck reel, wire snag, reel removal and power loss. Derive changing-radius dynamics and gear/reflected inertia. For cradle drive evaluate both rims/traction/contact stress, axial walk/retention, rim-speed slip and brake torque; for arbor drive evaluate cones/shaft/bore torque path and loading ergonomics.

**Acceptance:** reel drive replenishes the buffer without exceeding the wire's qualified tension, losing control or consuming available stroke; structure and retention pass working/proof/fault cases. Demonstrate safe 30 kg loading with an actual included procedure/fixture, not a hidden lift or unsupported cantilever. Brake energy, duty, heat, wear and fail state pass, including full-speed power loss. Validate motor torque/current, rim/arbor slip, tension and stop distance on real full and near-empty reels. Do not apply a 20 kg OEM reference rating to this 30 kg design.

### M29 — Dancer/buffer mechanics and hybrid control stability

**Inputs:** exact reeving, pulley mass/inertia/drag, carriage/counterweight/spring geometry, position sensor, end stops, drive controller, delays/saturation/deadband, wire bend/axial behavior and cutter demand traces. Derive storage and tension from virtual work/Jacobians for the **actual** reeving; do not count every drawn span as active storage.

**Cases:** full cycle demand including reverse strip motion, repeated fastest short parts, 3 m feeds, buffer recovery, large/small/full/empty reel, controller sample delay and sensor error, high/low friction, wire break, reel end, jam and power loss. Compute required storage from the integral of cutter demand minus payoff delivery plus uncertainty and physical end margins. Analyse linear stability where valid, then nonlinear/hybrid transitions, saturation, integrator windup and limit cycles.

**Acceptance:** no dancer bottom/top impact in normal operation, bounded wire tension inside the metrology/damage budget, sufficient recovery and no unstable hunting. Reversals must be absorbed without loose rewind/entanglement. Position/health limits stop the cutter before storage is exhausted, allowing the actual stopping latency and reserved stroke. Validate the complete tension-versus-position/demand response with an independently calibrated tension sensor and timestamped position/current records. Provide the selected gains, sample rate, limits and commissioning method in source and manual.

### M30 — Slug/prime removal, flexible output and part clearance

**Inputs:** actual blade/waste geometry, 10 mm prime stub, 3-25 mm insulation slugs, adhesion/static charge, wire curl/stiffness, output tray/conveyor/drive and real clear/presence sensors. If using air, include real pressure/flow, valve response, compressor/site interface, filtration, hoses, exhaust and noise; compare a mechanical sweep/passive guided route by total cost and reliability.

**Cases:** prime stub too short to reach a 75.5 mm nip; both slugs retained/released/partially attached; 200 mm and 3 m finished parts; small flexible wire, stacked/double pieces, full bin, bounced part, blocked chute, sensor stuck/dirty and early/late presence-to-clear events. Use flexible multibody/contact or experimentally bounded trajectories as needed; adhesion and electrostatics require coupon data rather than arbitrary friction alone.

**Acceptance:** every waste body has a reliable removal path and verified clearance before the next hazardous motion; every accepted part reaches a supported destination or is positively quarantined. Capture part-present/clear edges during the physical ejection interval, including events completed before `PieceDone`. No hardcoded `part_clear=true`, disabled production check or unobserved roller-only ejection. Validate prolonged material-lot/wear tests and injected obstructions, quantify jam/reject rate and recovery. Include bins, liners, sweep wear parts and all sensors/mating connectors in the BOM.

### M31 — Encoder, index, sampling and complete error budget

**Inputs:** exact encoder/wheel geometry and electrical timing, loaded circumference, runout/harmonics, angular index/phase repeatability, bearing drag, line-driver/receiver thresholds, sampling latency and firmware arithmetic, actual wire tension/temperature and reference metrology. Separate quantisation, bias, cyclic error, slip, compliance and common-mode errors.

**Cases:** every feed/strip direction and material-valid interval, shortest/longest piece, hot/cold/worn wheel, index lost/spurious, startup arbitrary phase, uneven wheel contact, counter wrap, edge loss and dual-sensor correlated errors. If phase mapping is selected, identify it using a real independent reference, validate held-out phases/wires/temperatures and implement index/phase capture and map interpolation on target. Do not fit and validate on the same records or credit a synthetic map in production.

**Acceptance:** a full bound/validated probability model meets relaxed finished-length +/- (0.20+0.0005 L) mm and each strip +/-0.20 mm after internal guard bands. Explicitly budget final correction residual, calibration uncertainty, tension relaxation, blade datum, thermal drift, output handling and measurement decision uncertainty. Current whole-length residual tolerance cannot consume the entire external allowance; strip motion must not inherit an extra length-dependent tolerance that exceeds the strip budget. Validate with independent references over phases and lengths, not with another counter sharing the same wheel assumption.

### M32 — Electrical steady state, startup and transient circuit integrity

**Inputs:** manufacturer-guaranteed limits for the exact photorelay revision, reverse diode/TVS, resistors/PTCs, GPIO/receiver/buffer, supply rails, sensors/ALM outputs, cable resistance/capacitance, ground offsets, connector pinout and USB/VSYS power paths. Include local component temperature and resistor/LED derating, not just room values.

**Cases:** all tolerance and temperature corners, all ten inputs on, minimum/maximum field voltage, sensor ON drop/OFF leakage, GPIO internal pull accidentally enabled, broken/shorted/reversed wire, contact bounce, common-return rise, startup/reset/brownout/bootloader, 24/5/3.3 V order and USB hotplug/backfeed. Analyse relevant transient/injection/TVS energy with SPICE or equivalent verified circuits; use worst-case guaranteed bounds before Monte Carlo.

**Acceptance:** guaranteed logic HIGH/LOW margins and timing, adequate source/contact wetting duty, ALM current below its 50 mA limit, no thermal/absolute-rating exceedance or unintended actuator permission. LED trigger margin must hold hot and aged; 'recommended typical current' alone is not a guaranteed switching bound. Verify exact pin/pad coordinates and physical connector mates independently of ERC. Correlate hot board measurements, rail/input waveforms and fault injections; keep safety circuits outside ordinary MCU assumptions.

### M33 — Short circuits, inrush and protection coordination

**Inputs:** actual PSU short-circuit/foldback/hiccup behavior and output capacitance, mains source impedance/prospective fault current, branch cable/terminal/PCB limits, DC fuse/breaker curves and breaking capacity, contactor topology, XPS protection and thermal derating. A PTC's headline hold current is not its hot-trip curve.

**Cases:** dead short and resistive fault at each source/remote end, partial return fault, high/low mains, hot cabinet, repeated start/stop, simultaneous capacitor charging, stalled motor and failed braking component. Include current-limited supplies that cannot deliver enough current to operate an upstream fuse promptly. Evaluate capacitor discharge I2t and wiring/PCB energy separately from continuous fault current.

**Acceptance:** all faults are interrupted or continuously limited to safe conductor/component temperature with an assessed fault state; nominal current matching is insufficient. Exact DC voltage/pole arrangement and load make/break duty are valid; no AC-only rating is substituted. Coordinate fuse, contactor, wire, TVS/PTC and PSU behavior, including nuisance trips and welded contacts. Validate selected inrush/interruption/short-circuit cases in a properly contained competent test setup; document source conditions and actual clearing/temperature traces. The manual gives exact fuse class/rating and replacement, not 'same amps'.

### M34 — Regeneration, bus voltage, discharge and stored energy

**Inputs:** actual moving inertia/mass, gravitational and elastic energy, driver regeneration efficiency/path, PSU sink capability if explicitly specified, minimum bus capacitance after tolerance/aging, ESR, initial maximum voltage, braking/discharge resistance, switch/control and thermal ratings. Treat each isolated branch after contactor opening separately.

**Cases:** simultaneous maximum-speed deceleration, reversal, jam springback, head lowering if relevant, payoff braking, ordinary/E-stop/power loss, open/short braking resistor, failed switch and lost logic. Use energy/charge conservation and transient circuit simulation. For reference, `V_final=sqrt(V_initial^2+2E/C)`; the 36 V/100 uF/+0.1 J example reaches 57.411 V, not a safe 36 V branch.

**Acceptance:** peak bus and driver-terminal voltage including cable inductance remains below the conservative exact driver/PSU/capacitor ratings with margin; resistor pulse/continuous and switch SOA pass. Discharge is deterministic and verified at worst capacitance/resistance/voltage, with measurement of every retained branch. Safe-energy/standstill thresholds and guard delay derive from actual hazards, not an invented universally safe voltage or a generic timer. Show failure behavior and residual torque. Validate oscilloscope/current/voltage captures and actual accessible-energy/stop measurements; bleeders and brake hardware are priced and located in CAD/thermal models.

### M35 — EMC, signal/power integrity and electrical immunity

**Inputs:** real cable types/lengths/shield terminations, cabinet bonding/PE, separation of motor/mains/logic, encoder line drivers and termination, input filters, switching edges, grounding and actual environmental severity. Determine applicable test standards/levels from jurisdiction, installation and product classification; do not invent an EMC certificate.

**Cases:** signal-integrity models for encoder/step/direction timing and ground bounce; common-mode/differential interference, ESD at touch points, EFT/burst on supply/field cables, surge where applicable, conducted/radiated RF immunity, emissions and mains dips/interruptions. Test motor braking, contactor/coil switching and long payoff cables as disturbance sources. Whole-cabinet electromagnetic FEM is justified only if it supports a decision; circuit/cable analysis plus competent physical testing is often more useful and cheaper.

**Acceptance:** no unsafe actuation, lost/extra accepted part, corrupted approved recipe or undetected encoder error under the declared criteria; any allowed recovery/degradation is defined before testing and cannot count a suspect part as good. Measure actual pulse/receiver margins and test with final harness/filters/bonding. Preserve setups, calibrated generator settings, coupling networks, photos and results. Software simulation cannot substitute for radiated immunity/emissions measurements. Any rerouting/filter change invalidates affected evidence.

### M36 — Target real-time execution, PIO and counter arithmetic

**Inputs:** pinned MCU silicon/SDK/compiler, real PIO programs, peripheral clocks, encoder/step edge rates, DMA/FIFO/interrupt behavior, scheduling, serial traffic, logging, expander bus delays and watchdog. Include the rejected/allowed silicon path and actual boot pin states.

**Cases:** maximum pulse demand at minimum loaded radius, both axes/cassettes active, simultaneous quadrature/index transitions, maximum UART load, flash activity, stuck I2 °C and interrupt storms. Capture pulse-high/DIR setup/hold including startup and reversal. Analyse wrap at signed/unsigned boundaries and time-base rollover, not only a short happy-path run. At about 20,500 edges/s, 10,000 h yields about 7.38e11 counts; an unextended 32-bit counter wraps in about 58.2 h and a signed half-range in about 29.1 h. One million pieces can outlast these horizons.

**Acceptance:** demonstrable worst-case timing and sensor integrity, no silent pulse truncation/FIFO starvation/count loss, bounded stop latency and safe watchdog/reset. Raise the current 8000 Hz limit only after qualifying the entire target path and driver timing; retain 0.50 m/s. Bench capture real pins with a calibrated logic analyser/oscilloscope under stress; compare actual pulse/encoder counts and timestamps with independent generation. Record the exact firmware image/hash and test silicon; a host C++ pass does not establish target timing.

### M37 — State-machine invariants, formal checks and fault injection

**Inputs:** explicit machine states/transitions, material topology, hardware permission/disable polarities, event timing, part/slug/buffer sensor validity, safety diagnostics, approval and persistent counters. Write invariants independently of current code before testing it.

**Cases:** model checking or exhaustive bounded exploration of reachable event orderings, delays, simultaneous faults, command races and all stop/restart phases. Key properties include no motion without commissioned current identity and valid permissions; no accepted count before physical cut/strip/clear evidence; no auto-restart after reset/power restore; no clear of a latched cause by MF alarm reset; no guard-release request treated as proof of standstill; no stale expander data as healthy. Cover both independent jaws and off-normal recovery.

**Acceptance:** prove or bounded-check the declared properties with assumptions and scope stated; reproduce counterexamples and fix the implementation. Add meaningful mutation tests for inverted polarity, removed interlock, missed edge history and incorrectly accepted scrap. Check liveness/deadlock as well as safety so a machine that never runs cannot pass by vacuity. Validate transition logs in HIL and hardware, including human panel sequences. Independent hardwired safety remains valid if the MCU hangs or lies.

### M38 — Protocol robustness, persistence, power-loss and flash lifetime

**Inputs:** real host/panel command grammar, lengths/ranges/units, parser, settings/calibration/recipe/approval journal format, CRC/versioning, exact flash part endurance/retention and write/erase behavior at temperature. A Pico module's advertised flash size does not establish its endurance.

**Cases:** grammar-based and coverage-guided fuzzing, truncated/overlong/invalid packets, NaN/Inf, extreme signed arithmetic, one-million-piece quantities, duplicates/replays, stale identities and disconnect mid-command. Interrupt power at each record/erase/write/approval boundary; inject corruption, full journal, worn/failed sector and mixed firmware versions. Model wear levelling and actual record frequency. At 1.8e7 cuts, per-piece persistent updates can consume a large erase budget even if settings rarely change; calculate it from physical record size and erase geometry.

**Acceptance:** no out-of-bounds/undefined behavior, unsafe motion or falsely valid approval; torn/corrupt/unsupported records fail safely and produce a recoverable diagnostic. Counters preserve an explicitly defined conservative relation between produced/accepted/scrap across power loss; never overclaim accepted output. Demonstrate hot retention/endurance margin with the exact flash data and design, plus executable fault-injection tests. Separate update authority, service mode and ordinary screenless production behavior.

### M39 — MIL/SIL/HIL and hardware correlation

**Inputs:** independent M27 material plant, drive/reel/thermal models, actual production controller and target, sensor/actuator electrical emulation and timestamped reference traces. Define the interfaces and physical fidelity of model-in-loop, software-in-loop and hardware-in-loop separately.

**Cases:** entire recipe matrix, worst interacting corners, slips, absent/mislocated sensor, asynchronous part-clear, buffer depletion, driver alarm/logic loss, noisy edges, lagging sensors, brownout and communication faults. Run identical physical scenarios through low-fidelity models and the actual MCU with independent generated sensor signals; then replay measured machine traces without altering them to match the desired result.

**Acceptance:** observed physical quantities and event order agree within declared model discrepancy/timing/measurement uncertainty; all stated fault reactions and invariant checks pass. Confirm actual target step/MF/enable signals and input filtering; HIL cannot inject ideal signals through a path different from the production board without declaring the gap. Retain raw input/output waveforms, seeds, truth trajectories, event tags and mismatch reports. Do not call HIL a physical cutting or thermal test.

### M40 — Risk analysis, safety reliability and common causes

**Inputs:** hazard inventory covering blades, nip points, screws/belts, reel/cradle, dancer weight/springs, wire whip, ejected fragments, electrical/thermal/fire, output handling, loading, service and foreseeable misuse. Use current applicable machinery standards and exact certified safety-component data, mission/demand rates, diagnostic coverage and common-cause conditions.

**Cases:** derive each required safety function and PLr/SIL basis from risk, then FMEA/FMEDA/fault tree as appropriate: welded contactor/relay, short across channels, common supply/return failure, guard defeat, failed lock, probe/control failure, power loss/restoration and maintenance. Calculate B10d/MTTFd/PFH or other required quantities with actual utilization and category constraints; do not assign PL d/e from two wires alone. A quantitative fault tree with independent probabilities cannot erase common-cause, systematic software or physical defeat.

**Acceptance:** risk reduction and validation demonstrate the required performance for the whole function, including input, logic, output, diagnostics, stopping/energy and guard locking. Derive safe-release thresholds from hazards and measured worst cases. MCU MF is not STO; ordinary thermal relays and I2 °C are not certified safety channels. Deliver a complete electrical/mechanical safety specification, verified schematics, diagnostic/reset procedures and physical validation evidence, reviewed by a competent machinery-safety engineer before a safety/compliance claim.

### M41 — Guard reach, human factors and recoverable maintenance

**Inputs:** final guard openings/distances, approach directions, stock/outlet passages, locking actuator/hinge/frame stiffness, control layout, operator tasks, reel weight/size and all required tools. Use applicable reach/crush/interlock/ergonomics standards after current authoritative edition/applicability checks.

**Cases:** reaching around/over/under/through guards, flexible wire used to access hazards, multiple-person access, loading 30 kg reels, small-reel changes, threading, blade/filter replacement, jam clearance and loss of lighting/power. Evaluate visibility of status and waste build-up, mistaken connectors/reeving/directions, pinch risks during assembly and stored-energy release during rescue/service. Digital human modelling can screen access; it does not replace actual task trials or the applicable standard's measurements.

**Acceptance:** risks are reduced with robust design and guarding; routine tasks are understandable without a screen or improvised bypass. Safe maintenance isolation and restraint are explicit; a locked door must not trap a person in an accessible volume without assessed means of release. Independently walk through tasks with the one manual, record errors/time/ambiguous steps and revise design and instructions. Include every guard bracket/interlock fastener and service fixture in the BOM and CAD.

### M42 — Global uncertainty, robust optimisation and adversarial cases

**Inputs:** margin functions and validated domains from all prior models, correlated tolerances and physical states, epistemic bounds, measured parameter distributions, supplier variation, aging and model discrepancy. Keep separate uncertainty budgets for safety bounds, performance/yield and economic estimates.

**Cases:** interval/corner analysis for hard limits, global sensitivity, low-discrepancy/space-filling designs, surrogate models checked on held-out points, targeted worst-case optimisation and rare-event methods where justified. Search interacting weak points: hot/high-drag/low-voltage axis, worn-small-radius/high-demand feed, full-inertia/restricted-buffer start, low-friction wheel/high encoder drag, local hot PCB/high field voltage and dirty-filter/probe-cool hotspot. Include intermediate extrema; not every maximum lies at the corners.

**Acceptance:** no hard constraint is traded for average score or cost; predicted worst cases are confirmed at adequate fidelity, with conservative model/parameter uncertainty. Report sensitivity, effective sample sizes, confidence and coverage, and retain counterexamples. Use value-of-information to identify measurements that meaningfully shrink uncertainty before buying precision parts or running huge simulations. A broad probabilistic campaign cannot justify an unknown sensor topology or unspecified reel envelope.

### M43 — Measurement-system analysis and process capability

**Inputs:** independent calibrated length/strip reference system, fixtures, measuring tension/straightness, relaxation/temperature policy, operator/repeatability/reproducibility, resolution and calibration uncertainty; actual consecutive-piece data with lot/build/time/recipe provenance.

**Cases:** external reference across 200-3000 mm and both strip ends, multiple operators/fixtures/days where relevant, cold/hot drift and representative lots, minimum/maximum/asymmetric strips and boundary wires. Conduct bias/linearity/stability and gauge R&R analyses before calculating capability. Use rational subgrouping, check temporal stability/autocorrelation and distribution assumptions; investigate every outlier rather than deleting it for a better Cpk.

**Acceptance:** original Cpk>=1.33 and >=30 unique consecutive pieces per defined cell remain. Section 12 specifies additional confidence and guarded measurement criteria. Deliver raw rows, calibration IDs, uncertainty budget, distributions/control charts and reproducible calculations. If nonnormal or autocorrelated, use an appropriate validated capability/nonconformance method and report it clearly; do not report a normal Cpk formula as a universal yield guarantee. Internal encoders cannot independently validate their own absolute length accuracy.

### M44 — Reliability, endurance and accelerated life

**Inputs:** mission profile, failure definitions, load/temperature/humidity/dust spectra, service and replacement policy, component life/quality data, software/event horizon and planned prototype/unit sample. Distinguish wear-out, random failure, infant mortality and assembly errors.

**Cases:** 10,000 h / 1.8e7-cut design duty, representative wire/recipe mix, start/stop, power interruptions, reels/filters/blades/rollers/belts service and controlled degraded conditions. Combine mechanistic fatigue/wear/thermal aging evidence with whole-machine cycling. Accelerate only where the governing failure mechanism remains the same and its stress-life relationship is established; increasing speed can change vibration, heat and friction rather than simply scaling time.

**Acceptance:** quantified service life and failure-rate/confidence claim for the actual scope, including sample size and censoring; section 12 gives zero-failure calculations. No lifetime claim from one bearing L10 or a short demonstration. Track accepted/scrap/jam counts, maintenance, wear and dimensional capability throughout endurance; passing the start/end mean alone is insufficient. Investigate failures, implement corrective action and rerun affected evidence. If full physical life testing is unfinished, label that truth while delivering all executable design and qualification work.

### M45 — Manufacturability, assembly simulation and builder errors

**Inputs:** actual found parts/packs, drawings/processes, shop capability, tool list, fixtures, tolerances, work instructions and assembly occurrence graph. Include procurement kit contents and all mating hardware, not only nominal assemblies.

**Cases:** simulate machining/tool/clamp access and sequence; check complete bolt/thread stack and wrong-length insertion, mirrored/handed parts, wrong bearing support, reversed photorelay/diode/connector, wrong Pico footprint, omitted washer, incorrect reeving, cable misroute and sensor polarity. Run an independent manual-only build from the counted kit, or a detailed digital walk-through when hardware is unavailable, explicitly distinguishing the two.

**Acceptance:** no builder invents a part, hole, shim, torque, wire endpoint or component variant; all necessary tools/materials are supplied or precisely listed and costed. Errors are prevented or detected before energy/motion. Record real assembly and adjustment time, rework, first-pass tests and every clarification. Use those results to simplify interfaces and the manual rather than burdening the builder with judgement. All moved/changed parts trigger the relevant BOM/CAD/model/manual updates.

### M46 — Transport, storage, handling and service aging

**Inputs:** machine/stand/reel module masses and centres, packaging, lift/tie-down points, exposed precision interfaces, bearings, coatings, electrical storage limits and actual transport route. Define shipping with or without reel and restrained dancer/head; they are different states.

**Cases:** credible transport vibration/shock/drop/tilt loads, handling strains, loose-module impact, corrosion/humidity/condensation, storage dwell, cable/connector damage and filter contamination. Use a relevant specified packaging/environmental test profile if needed; do not invent a certification from an arbitrary drop height. Assess precision datum/rail/bearing brinelling and retained preload after transport.

**Acceptance:** safe handling and unpacking, no permanent damage or calibration change outside permitted requalification, clear transport locks and removal checks. Label exact lift points/weights and never use a guard/cable/precision shaft as a lifting point. Validate packaging/transport assumptions to the intended route and perform arrival inspection/recommissioning. Packaging, restraints, corrosion protection and any lifting aid are real cost/part lines.

### M47 — Integrated discrete-event throughput, yield and availability

**Inputs:** validated motion profiles, cut/strip times, settle/homing/correction, buffer replenishment, slug sweep, output clearance, physical sensing latencies, panel operations, maintenance and observed jam/reject distributions. Include any compressor/conveyor or payoff capacity limits.

**Cases:** 200/1000/3000 mm and intermediate bottlenecks; all strip-enable combinations and 3/25 mm extremes; wire/reel types; hot steady operation; start-up/recipe/reel change and common recoverable faults. Model concurrent tasks only when hardware/control genuinely supports concurrency and collision/safety limits permit it.

**Acceptance:** publish achievable rate, good-piece yield and availability with uncertainty for each declared condition, and compare with the frozen throughput requirement. At 0.50 m/s a 3 m feed alone takes at least 6 s before ramps/other work; do not claim 30 pieces/min for that case. Separate theoretical cycle, measured cycle and sustained good output. Validate timestamped real state/sensor logs and independent output counts. Do not meet rate by omitting handling, quality checks or discarded parts.

### M48 — Robust cost optimisation, market drift and value engineering

**Inputs:** normalised complete baseline, every exact sourced/made part, quote validity/MOQ/packs, machining/finishing/metrology/labour, freight/tax/duty/payment/FX, fixtures/tool amortisation, yield/rework and maintenance. Attach engineering constraints and supplier authenticity/capability evidence to each candidate.

**Cases:** Chinese and western suppliers, alternate complete architectures, local make/buy, fan/heat tradeoffs, common SKUs versus local derating, powered arbor versus cradle/cassette, PCB size/layer/assembly tradeoffs and feasible accuracy grades. Evaluate landed single-machine cost separately from production-volume cost. Stress cost by FX/shipping/lead-time/quality variation; a cheap uncertain listing is not a qualified quote.

**Acceptance:** choose the lowest adequately evidenced complete alternative among a declared broad search, disclose the search boundary and Pareto choices. Target final equivalent cost<=0.30 times baseline; compute actual reduction and cost uncertainty, never force an unsupported 70% result. Demonstrate robustness improvement with measured/verified margin, failure-mode removal, service/assembly effort and quality control. Deliver alternative comparison, dated line-level price captures, complete totals and explicit unmatched scope. Global cheapest and universal reliability cannot be proved by a finite catalogue search.

## 11. Physical verification and qualification: executable, instrumented and honest

### 11.1 Prerequisites, records and common measurement rules

No physical tests were performed for this handoff. The completion agent must run every possible source/model/target test, prepare exact physical procedures and execute them if the actual hardware/instruments are available. A desk-only environment cannot close a physical qualification gate. Do not fabricate rows, substitute a synthetic fixture, or mark 'physical_results=true' merely because an identity lacks a `SYN-` prefix. Require traceable acquisition evidence and review. The old qualification tool's narrow 15-30 C/200-and-1000 mm cells do not qualify the full 40 C/3000 mm factory envelope.

Create one test-record schema with: test ID/revision; linked requirements/models; machine/build/hardware/firmware/calibration/recipe hashes; all relevant component/material lots; operator/reviewer; date/time and timezone; ambient pressure/temperature/RH; actual mains voltage/frequency; setup photo/diagram and coordinates; calibrated instrument identity/range/resolution/uncertainty; sampling/time synchronisation; input settings; raw unedited readings; calculated uncertainty/margins; predeclared acceptance; deviations; outcome and evidence links. Preserve rejected, interrupted and failed tests. A rerun is a new record with a reason, not an overwrite.

Choose bandwidth/sample rate from the physics and demonstrate anti-aliasing. Thermal trends may be logged at >=1 Hz as a starting point; rapid contactor/regen/stop/input events need appropriate oscilloscope/logic capture, often orders of magnitude faster. A 1 Hz logger cannot measure a millisecond protective delay. Time-align material video, reference displacement, tension, encoders, commands and safety signals when judging cause/effect. IR camera emissivity/reflections must be calibrated; bare shiny shafts cannot be trusted from a default IR setting.

Use independent hard temperature/energy/force stop limits and a competent contained test setup for fault/proof tests. Simulated probe faults can establish control response, but do not alone demonstrate actual device temperature protection. Never destroy a driver or expose an operator to validate a model; stop safely, record the bound and redesign the failed margin. The exact final prototype is the physical reference; a different open bench arrangement does not qualify a closed cabinet.

### 11.2 Ordered physical test families

| ID | Work and essential measurements | Pass basis |
| --- | --- | --- |
| P00 | Incoming inspection: quantity, exact labels/MPNs/suffixes, authenticity/channel, critical dimensions, material/heat-treatment records, actual connector mates, screw lengths/classes, MCU silicon and motor/driver parameters | Every selected identity and critical property traceable; reject substitutions until affected analysis/approval is redone |
| P01 | Machined/welded part inspection, actual bearing/shaft pairing, guide/rail/base datums, blade grind, washer/receiver threads and actual assembly stacks | Drawings plus guarded uncertainty; measured fit/preload/clearance and no unrecorded rework |
| P02 | Bare-board/assembly fixture on current-limited isolated bench supplies: continuity, shorts, rail sequence, all inputs/outputs, silicon rejection, hot corners and firmware identity | Native design/fixture contracts, no unintended actuator permission; fixture image cannot authorise production |
| P03 | De-energised cabinet point-to-point, PE/bonding, insulation and protective-device/wiring inspection, then controlled power/inrush/bus checks | Applicable electrical test plan, exact protection coordination, actual measured startup/rail states and no hidden backfeed |
| P04 | Independent E-stop/guard/reset/EDM/lock tests, stopping and every retained bus/energy branch, power-loss/restoration and selected faults | Risk-derived safety specification and measured safe-release condition; ordinary software status cannot substitute |
| P05 | Axis/receiver/adapter/shaft/frame/reel structural proof and post-proof geometry; torque-tension and collar slip tests | Original working/proof/fault loads, no disallowed permanent set/slip/damage; all inspection and accuracy restored after proof |
| P06 | Individual four machine nip-force curves, two wheel-force curves, two belt tensions over min/max OD, direction/hysteresis/temperature; loaded roller/wheel circumference and drag | Guarded 50-75 N nips, 6-19 N wheels, 180-200 N belts, required grip/calibration/speed margins; no averaging away failed stations |
| P07 | Instrumented wire cut/strip force and microscopy/quality coupons over selected lots, temperature, settings and wear | M25 quality criteria, real axis reserve and bounded recipe/incision/edge-life envelope |
| P08 | Complete payoff/buffer/output: actual small/large reels, full/mid/empty, speed/reversals/stop, tension/position/drive/slip, loading and retention | 30 kg design, declared geometry, storage/tension margins, no uncontrolled rewind/impact/entanglement and safe fault response |
| P09 | Impact/modal/FRF and operating vibration/order tracking, relative blade-wire displacement and actual cuts | Correlated M13-M17 response within accuracy/fatigue/installation limits |
| P10 | Full cabinet/device and motor/shaft thermal campaign, including section 11.3 and 11.4 | Every actual normal/fault rating and original shaft-rise gate with uncertainty |
| P11 | Actual-target pulse/encoder/index/protocol/HIL and control fault tests, power interruption, long-counter/flash tests | M36-M39 contracts; no unsafe start, lost event, false accepted count or invalid persistent approval |
| P12 | Final-harness EMC immunity/emissions and applicable environmental tests | Risk-derived/applicable product criteria, recorded competent measurements and configuration |
| P13 | Full dimensional/strip/process capability and independent measurement-system study | Every goal and section 12; original required cells plus extended factory/material/length scope |
| P14 | Sustained throughput/yield/endurance/service/replacement and transport/recommissioning tests | M44/M46/M47, actual workload and quantified confidence; trace every failure and service action |
| P15 | Independent builder follows the one canonical final visual PDF from counted kits | No missing part/tool/geometry/wiring/acceptance choice; recorded corrections incorporated and regression checked |

The ordering is a dependency rule, not a mandate to wait idly: procurement research, CAD, low-voltage fixtures, numerical studies and draft instructions can advance independently. Safety and structural prerequisites must be satisfied before powered cutting/proof scenarios that depend on them.

### 11.3 Cabinet thermal steady-state campaign: at least 18 primary records

First measure real final-design losses by consumer and duty. Choose three **actual** limiting duties: D1 highest continuous motor/driver running loss at achievable cycle mix; D2 idle/holding plus all continuously energised coils/inputs and worst relevant control load; D3 the different spatially worst allowed mixed duty, including payoff/braking/output electronics if co-located. If one duty bounds all others in both total heat and every local hotspot, demonstrate that with evidence; do not choose three easy duplicate runs. A calibrated distributed heater test can establish enclosure heat-removal capacity at a design allocation, but it does not validate a driver's hot torque or internal loss.

Use actual declared minimum and maximum mains voltage at the final frequency. At 40 C ambient run each duty under three installed filter distributions: F1 clean; F2 uniform service-limit pressure drop; F3 realistically uneven service-limit loading that is adverse for a critical device while all required normal permissions remain healthy. That is `2 voltages x 3 duties x 3 filter conditions = 18` primary steady-state records. Additional voltage/load/internal-regulation extrema found by the models must be tested as needed; the count is a minimum matrix, not a certificate. Qualify 60 Hz separately if it is claimed; two fans' 50 Hz results do not transfer to their higher 60 Hz power.

For every run retain:

- Ambient around each inlet, pressure/RH/density; total and path flow/pressure drop with installed mats and closed guards/doors; voltage/frequency/current and real input power, not only VA.
- Temperature at **each driver inlet**, relevant driver cases, every supply inlet/case/critical terminal, hottest PCB resistor/photorelay/MCU/receiver/PTC regions, coil/protection/brake devices, fan motors, outlet and thermostat probe. Add points from preliminary thermal mapping; document their coordinates.
- Motor/flange/coupling/shaft/bearing/housing temperatures and differential rises under the simultaneous mechanical duty. Cabinet cooling does not automatically fix a motor-to-shaft conduction failure.
- Thermostat reading, calibrated probe error, permission states, actual fan speed/flow, driver parameters/load traces and time since energisation.

Declare steady state before collecting acceptance values. A conservative starting rule is all critical points changing <=0.2 K/h over the final 60 minutes after sufficient identified thermal time constants; tighten or extend if a slow bearing, coil, enclosure or motor has not equilibrated. Log the complete warm-up and power-down soak. Give measurement uncertainty and a guarded maximum for each component. Normal inlet<=45 C and each selected component's individual derating/rating apply simultaneously; a 50 C driver does not raise a 45 C supply/sensor limit. Compare predicted and measured heat balance, flow and local temperature; resolve discrepancies and rerun affected cases.

Include added cold-start/hot-restart and production-interruption tests. The final service interval is tied to a measurable filter pressure/flow/temperature bound, dust-loading evidence and replacement instructions. A presence vane staying closed is not proof of adequate service flow. A failed 18th run cannot be averaged with 17 passing runs.

### 11.4 Thermal and permission faults: 17 explicit starting scenarios

For each scenario specify the initial hottest qualified duty, injected fault, independent containment/abort limits, observed sensor response, all permission/MF/contactor states, motor stop, peak/post-trip temperatures, every bus decay and restart requirements. Include worst component/sensor tolerance and actual measured delay. Use safe emulation where required and label what physical effect it does and does not reproduce.

| Fault ID | Scenario | Required observation/design consequence |
| --- | --- | --- |
| TF01 | Fan 1 fails electrically/mechanically | Detect or thermally protect worst affected zone before its guarded limit; no unsafe auto-restart |
| TF02 | Fan 2 fails | Same for its different flow path; symmetry cannot be assumed from a drawing |
| TF03 | Both fans lose common supply | Common-cause protection and post-trip soak remain adequate |
| TF04 | Progressive/uneven inlet clog while both presence contacts remain closed | Actual temperature/flow protection catches the adverse hotspot; proves presence alone is insufficient |
| TF05 | Outlet restriction or inlet/outlet recirculation | Protect stagnating device zones and document installation clearances/baffles |
| TF06 | Thermostat probe open/disconnected | Exact delivered controller's fail behavior removes permission and latches the right diagnostic |
| TF07 | Probe short/cross-wire | Same; a falsely cold reading must not leave unprotected operation |
| TF08 | Probe stuck at plausible low value or displaced from mapped position | Demonstrate independent protection/diagnostics or redesign; a timeout on a plausible temperature is not guaranteed detection |
| TF09 | One thermostat/permission output contact welded | Other risk-required path performs its function; evaluate common coil/supply/logic causes |
| TF10 | 5 V logic/DC-OK drops or slowly browns out | Motor permission is removed independently before outputs become uncontrolled; USB cannot backfeed a false healthy state |
| TF11 | One head-driver logic supply/permission path is lost | Actual MF logic loss cannot leave powered uncontrolled head motion |
| TF12 | One feed-driver logic supply/permission path is lost | Same for feed/piece handling; quarantine current work |
| TF13 | 24 V field/control supply slowly ramps through relay/logic thresholds | No chatter, false healthy threshold, contact cycling or unexpected restart; actual monitors have defined thresholds |
| TF14 | Total mains loss at hottest/highest-energy state | Safe stop/brake/lock direction, discharge and thermal soak; no assumed cooling after fan loss |
| TF15 | Local heat fault near a driver/PCB component, remote probe remains relatively cool | Spatial protection mapping actually covers local credible faults or independent component protection is proven |
| TF16 | Power/temperature recovers with RESET or START held, or following a watchdog reset | No automatic production restart or clearing of unresolved cause; partial part counted as scrap/quarantine |
| TF17 | Fan-presence/flow contact stuck healthy, broken return or short across one diagnostic path | Protection does not rely on an undiagnosed optimistic input; assess single/common causes and actual fault coverage |

Expand this list for the final architecture, especially braking resistor overheating, missing/incorrect filter, high-humidity component limits and any additional payoff supply. Do not pretend all listed faults are covered by one thermostat unless the independent analysis and physical response demonstrate it. Report testable coverage and residual risk; implement required corrections.

### 11.5 Integrated wire, reel and handling qualification

Use independently sourced representative wire lots covering 0.5/2.5 mm2, OD1.2/4.5 mm and intermediate worst constructions, not the physically impossible assumption that all area/OD extremes are one wire. Preserve actual strand/jacket/temperature ranges and supplier variation. Include 200/1000/3000 mm plus any interior maximum-error length, strips 0/0, 0/3, 3/0, 0/25, 25/0, 3/3, 25/25, 3/25 and 25/3 as starting boundary combinations. Add nominal settings and worst incision recipes identified by coupons. A zero strip setting means that end is intentionally disabled, not a zero-length slug assumed to be removed.

Cross the governing recipes with small/large reels, full/mid/near-empty, cold/40 C-hot, new/service-limit tread/blade and low/high friction. Use risk/sensitivity to reduce an exhaustive Cartesian campaign only with a documented bounding/coverage argument; every hard boundary and demonstrated interaction needs evidence. Predefine qualification cells and sampling rather than pooling good data after seeing results. Capture at least 30 unique consecutive pieces per capability cell, including failures and scrap with their identities; section 12 controls statistical interpretation.

Inject a prime stub that does not reach the downstream nip, attached/double slug, blocked waste chute, no wire, wire break, empty/end-of-reel, failed dancer sensor, near-end buffer at start, rim slip, stuck output, dirty/stuck part-clear sensor, early/late/double clear event and one failed encoder. Check controlled stop, intact guarding, correct counts/quarantine and deterministic recovery for each phase. Repeat reel changes/threading and blade/filter/roller service using the manual. Confirm the last part before a fault and first accepted part after recovery with independent metrology. Run a supported 3 m output in the actual installation, not curled on an improvised floor.

## 12. Accuracy, statistical confidence and reliability claims

### 12.1 Define the measurands before measuring

Finished length means the final cut wire between the specified end reference planes, after a defined relaxation time, at recorded temperature, straightened by a defined sufficiently low measuring tension that does not conceal or introduce error. Specify whether the dimension references copper or insulation ends, how bent/uneven ends are handled and the allowable straightening method. Strip length on each end is its own measurand, from the controlled cut end datum to the insulation edge, with a defined rule for a slanted/ragged edge. Coupon quality must prevent an apparently accurate edge formed by unacceptable jacket damage. Neither motor steps nor a temporary tensioned length is the final relaxed part measurement.

For command length L mm, the allowed half-width is `T_L=0.20+0.0005*L` mm. For either enabled strip, `T_S=0.20` mm. Derive **smaller internal residual limits** after allocating physical calibration/slip/relaxation/thermal/blade/handling uncertainty. Do not use `T_L` as the encoder stopping allowance and then claim the remaining unbudgeted errors are free. A strip move does not acquire the full-part 0.05% length allowance. Keep strip-disable behavior separate from capability statistics for an enabled strip.

### 12.2 Traceable uncertainty and measurement-system adequacy

Use an uncertainty budget consistent with the measurand and an appropriate GUM-style framework: calibration/bias, resolution, fixture/straightness/tension, temperature/expansion, operator, repeatability, relaxation time, edge location and reference stability. Identify sensitivity coefficients, distributions, correlations, degrees of freedom/coverage assumptions and expanded uncertainty at stated coverage. A `k=2` label is not automatically exact 95% coverage for every nonnormal small-sample problem.

Retain the original length gauge expanded uncertainty<=0.05 mm and force<=1.0 N where applicable, and improve if the actual margin requires it. As an **added engineering target**, aim for tolerance-half-width/expanded-uncertainty>=4; for +/-0.20 mm strips that means U<=0.05 mm. This ratio is defined here to avoid ambiguity over full-width TUR conventions. For 3-6 um matched interference, use a dedicated uncertainty budget with micrometre-appropriate instruments, thermal control and repeatability; a generic caliper or nominal 1 um display is inadequate.

Choose and document a conformity decision rule before testing. A conservative individual-part rule is `abs(error)+U<=T`, with appropriate coverage and correlation handling. For process capability, compute the bias/variance and uncertainty treatment consistently; do not subtract the same random gauge variation twice or hide calibration uncertainty inside an unexplained Cpk adjustment. Perform gauge R&R, bias/linearity and stability studies, and retain calibration certificates and actual measured temperature/tension. Select tools/fixtures by these needs and include their make/buy/rental/outsourced cost.

### 12.3 Capability, stability and coverage

Preserve the original requirement **Cpk>=1.33 with at least 30 unique consecutive pieces per cell**. For a stable approximately normal process, `Cpk=min((USL-mu)/(3*sigma),(mu-LSL)/(3*sigma))`; demonstrate why that model and subgrouping apply. The point estimate alone from 30 pieces is uncertain. As an **additional proposed release criterion**, use a one-sided 95% lower confidence bound on Cpk>=1.33 for each declared qualification cell, with a statistically valid calculation and sufficient sample size. This makes confidence explicit; it is not a claim that the original user specified that confidence level. If the requirement owner adopts a different confidence policy, record the decision and retain the original threshold rather than silently weakening it.

Analyse time order, drift, autocorrelation, lots, operators, start-up and warm-up; do not pool distinct recipes/materials/temperatures into a single broad distribution that hides a bad cell. For nonnormal data use a justified distribution-free or fitted-tail method with goodness-of-fit and uncertainty, plus the original capability reporting where meaningful. Bootstrap schemes must preserve dependence and have enough tail information. Planned multiple-cell/failure-mode claims need a stated simultaneous-confidence policy; separate 95% intervals do not automatically make a 95% statement about all cells.

Every part produced in a consecutive run gets an identity and outcome, including rejected/unfinished pieces. Report first-pass yield, correction frequency, scrap, jams and downtime. A controller that rejects every difficult piece cannot satisfy useful accuracy/throughput by reporting only the selected survivors. Keep counts of attempted, completed, accepted, quarantined and scrap consistent across retries and power loss. Any excluded measurement needs a predeclared scientifically valid reason and a preserved raw row.

### 12.4 Reliability and the limits of a universal guarantee

For n independent representative trials and zero failures, the one-sided confidence bound on per-trial failure probability is `p_upper=1-alpha^(1/n)`. For illustration only, proving `p<0.001` at 95% confidence under that model needs at least 2995 zero-failure trials. This is **not** a new project failure-rate requirement and not evidence that 2995 ideal simulations qualify the machine. Dependence, heterogeneous conditions, selective rejects and wrong failure definitions invalidate a naive binomial interpretation.

For zero failures over total exposure T under a justified constant-rate exponential model, the one-sided rate bound is `lambda_upper=-ln(alpha)/T`. At 95%, it is about `2.9957/T`. Wear-out mechanisms generally require other models; use Weibull/physics-of-failure with justified parameter confidence, censoring and mission profile. Component L10, mean life, MTBF and a guaranteed life are different claims. A 10,000 h test of one machine does not prove every unit will survive 10,000 h, and parallel units' hours do not necessarily demonstrate a single unit's wear-out life.

Tie reliability targets to the declared operating mix and service policy, and obtain a requirement decision for any unstated numerical failure-rate target rather than inventing one as a proven fact. Accelerated life needs evidence of unchanged failure mechanism and an identified acceleration relationship. Structural safety margins, stochastic capability, software coverage and zero-failure endurance answer different questions; none is a universal 100% guarantee. Finish all feasible design/test work and state the precise evidence-backed release scope.

## 13. Embedded, I/O and screenless-operation implementation contract

### 13.1 Signals, timing and ownership

Produce one exhaustive signal table: physical sensor/actuator ID and exact MPN, measurand/function, operating voltage/current, polarity/normal/fault states, wire endpoints and terminal/pin numbers, shielding/grounding, MCU/expander/peripheral assignment, filter/debounce, acquisition timestamp, maximum latency, validity interval, diagnostic behavior and independent safety relationship. Include part/slug sensing, real encoder indices if needed, payoff/dancer limits/status, driver ALMs, permission/MF relays, guard/EDM diagnostics and all panel indicators. No signal may be represented only by a boolean constant or a vague 'spare input'.

Keep fast step/direction/encoder/index capture on an actually validated target path. A slow I2 °C expander can host appropriate panel/status functions only with explicit fail behavior, readback, timeouts and data age. Check actual pinmux/PIO occupancy and firmware source rather than assuming the proposed example allocation fits. Failure/staleness of any production-critical sensor prevents a new accepted cycle. Safety functions use the independent assessed hardware path; MCU diagnostics do not carry their safety claim.

Write a hardware abstraction contract based on physical meaning, not side-index coincidence: stock and piece encoders, occupancy validity, actual motor positions versus material displacement, part/waste events, and timestamps. No sensor count follows a motor command unless the **physical** model establishes grip and contact. Define sign/origin transformations once and test all directions. Establish boot/reset/brownout states at hardware and firmware level, including pulling/disabling outputs before configuration, unsupported silicon rejection, watchdog and logic-supply loss.

### 13.2 Material states, quality and recovery

Represent a piece's identity through prime, feed, cut, both independent strip operations, correction and ejection. Track stock and severed piece separately, plus waste/insulation bodies where needed. Define exactly when a piece is attempted, cut, finished, quality-approved, cleared and accepted; accepting requires all required real evidence in order. A prime stub is waste, never a good part.

Handle early/late/bounced/stuck part-clear transitions with a time window tied to physical movement and recorded prior occupancy. Do not reset event history after the only useful event has passed. Define ambiguous/slipped/unobserved outcomes conservatively as quarantine/scrap and log the cause. Prove all strip-enable combinations and every stop phase, including one end stripped and the other unfinished. After a fault, perform an explicit safe recovery/rehome/rethread sequence; do not resume a half-known part merely because coordinates persisted.

Persist enough state to avoid overcounting after power loss without exhausting flash. Separate total completed, accepted, rejected and current-batch counts. Specify how an uncertain in-flight part is removed/quarantined and how the operator verifies it. Counter/journal rollover and actual quantities up to the declared maximum must be tested on boundary values and long durations.

### 13.3 Approval identity and ordinary panel workflow

Approval binds the actual build, firmware and relevant parameter version; silicon/PCB revision; exact motion configuration; driver current/microstep parameters; metrology/calibration/index map; wheel/roller and blade/service state; wire material/recipe; payoff variant; required sensor topology and safety/thermal acceptance. Some identities need physical service/receiving controls rather than magical automatic detection; document who records them and how an unverified change prevents production. CRC detects accidental corruption; do not describe CRC alone as security/authenticity.

Document the panel sequence from first power to commissioned ready, ordinary START/STOP/RESET/repeat, quantity completion, empty reel, jam/thermal/drive fault, recovery, service and power restore. Use unambiguous lamp/buzzer states with labels and a timing table; colour alone is insufficient. Routine use of an approved retained recipe must be screenless. Recipe setup/approval can use a defined service tool if that is necessary, but the builder/operator must have its exact connection, compatible software and instructions. Engineering/fixture mode must be visibly distinct and incapable of recording approved production output or bypassing independent safety.

### 13.4 Target/build evidence

Pin SDK/compiler/dependencies and provide real reproducible production and factory-test images, source hashes, board mapping and flash procedure. Run both cross-builds and actual target timing/boot/fault checks. An image built before the final I/O/PCB or MF wiring is not the final image. Verify that production and fixture both disable unwanted internal input pulls and enforce unsupported-silicon rejection. Preserve host sanitizers and add only meaningful regression/plant/boundary tests, not implementation-mirroring assertions.

## 14. Standards, safety and acceptance governance

Use the actual intended jurisdiction and placing-on-market/use circumstances, not a generic 'CE compliant' badge. The final agent must determine which standards/regulations apply and obtain competent review as required. Cite exact edition, amendments, applicable clauses/method and component declarations; a standards title is not evidence of compliance.

Authoritatively rechecked for this handoff on 29 October 2026:

- **ISO 12100: 2010**: risk assessment/reduction framework; the ISO catalogue indicates confirmation in 2022. A draft replacement is not the current published edition.
- **ISO 13849-1: 2023**, edition 4: safety-related control design; it does not prescribe a universal required PL for this machine. Derive PLr from the actual risk/function and validate the implemented function using the applicable companion validation approach.
- **IEC 60204-1: 2016+AMD1: 2021 CSV**, edition 6.1: electrical equipment of machines; the IEC catalogue lists it as valid. A future edition/draft is not a current requirement merely because it appears in search results.
- **NASA-STD-7009B**, 2024: a useful model/simulation credibility reference, not a machinery certification or a requirement imposed by this user's jurisdiction.
- NIST engineering-statistics material on capability: methodological support, not a substitute for a calibrated measurement system or this project's actual data.

Check current authoritative applicability/editions for reach/opening distances, guards, interlocking/locking, emergency stop, prevention of unexpected start, electrical/EMC/environmental tests, earthing/insulation, machinery-safety validation, ergonomics/manual handling and application-specific wire quality. Candidate families include ISO 13857, ISO 14120, ISO 14119, ISO 13850, ISO 14118 and the applicable IEC 61000/60068 test methods; they are **topics to verify**, not a claim that their current full text has been retrieved/applied here. Do not invent clause numbers or exact limits from memory. Obtain lawful access to needed normative text and preserve source/version metadata.

Maintain a safety-function specification separate from equipment-protection logic. For each function state hazard, initiating event, required response/time/energy condition, modes, PLr/SIL rationale, architecture/component data, diagnostics, fault exclusion justification if any, common causes, reset/restart, proof/service tests and validation. Include guard-lock power-loss direction and rescue/service behavior. Claims must cover the actual full chain and final assembly, not a certified relay by itself.

One final release matrix links each requirement to: selected build revision and exact components, design analysis with margin, numerical verification, physical validation/qualification, manufacturing/inspection controls, firmware enforcement, manual steps and residual limits. Use statuses that describe evidence (`defined`, `implemented`, `numerically verified`, `physically validated`, `qualified for stated scope`); do not collapse them into an optimistic PASS. A failed hard gate requires a correction and rerun. An unavailable physical test remains explicitly scheduled, never fabricated or hidden by a document validator.

## 15. Ordered execution plan: turn this handoff into a finished machine

Work through the following sequence while advancing independent sourcing, low-cost coupons and drafting in parallel where sensible. Each phase ends in a concrete reviewed artifact and revision, not another list of blockers. A failure triggers a design change and dependency rerun. Keep a concise decision log explaining selected alternatives and rejected cheaper options with the failed requirement, rather than preserving abandoned choices as active instructions.

| Phase | Implement the work | Required tangible output and exit condition |
| --- | --- | --- |
| A — Recover and pin | Read fresh upstream/AGENTS, compare against pinned HEAD, preserve user edits, import this kit/history and re-run supplied host/counterexample checks | Reviewable branch/worktree, provenance and difference log; exact current starting configuration; no claim that lost c83bca9/R4/R7 is recovered |
| B — Freeze requirements | Convert all 47 goals and later instructions into atomic IDs, measurands, uncertainty/margins and tests; propose reel/site/throughput details using actual catalogues/materials | One requirements/envelope register, explicit unresolved owner decisions and conservative provisional cases; continue work independent of those decisions |
| C — Fix material observability and handling | Develop actual sensor/validity topology, prime/slug removal, supported output, part-clear history and real I/O; compare sensor/repackaging alternatives by total adequate cost | Full phase diagram/independent plant, actual selected mechanisms and CAD interfaces, pin/signals table, regression reproduces old defect and verifies new behavior without fake feedback |
| D — Close metrology/traction | Identify wire/force/friction/drag/strain, wheel/index/calibration and internal budgets; resolve feed timing/radius margin and all contact forces | Implementable error budget and calibration, selected encoder/wheel/roller hardware, measured/coupon plan, guarded performance margins; no synthetic phase credit |
| E — Finish real cutting axes | Select exact screw/guide/support/coupling/receiver; design all internals and moving geometry; reconstruct/challenge adapter/washer/joint, loaded tooling gap and fault load bounds | Two complete native axes and drawings with actual load/torque/life/fit margins and access; scope-correct converged evidence; no generic BK12/HIWIN rating confusion |
| F — Resolve feed shaft heat and precision | Rebuild full heat/contact/clearance model, implement adequate heat path/fit/bearing/coupling design, qualify manufacture/inspection | Coupled thermal/fit/fatigue solution satisfying <=20 K shaft rise and original fits/life; matched-pair process and actual make/buy quote |
| G — Finish both reel modes | Freeze real reel packages, choose cradle/cassette versus powered arbor/belt alternative, complete buffer, brake, guards/loading and control | One selected architecture with both explicit modes, all adapters/drive/control/retention parts and exact reeving; dynamic/tension/storage/structural margins and safe loading method |
| H — Rebuild electrical/PCB/protection | Final I/O first; hot field inputs/real sockets/connectors/silicon; all power, MF/permission, regen/discharge, fuses, safety and harness | Complete current native schematic/PCB, source/export parity, routed ERC/DRC clean board and manufacturing files; physical-coordinate/MPN independent audit and actual target images |
| I — Optimise cabinet cooling | Joint loss/airflow/fan heat/cost optimisation; actual layout, filters, sensors/interlocks and service | Ventilation solution satisfying every device at 40 C, numerical/fault evidence and executable 18+17 physical campaign; cooler removed from active BOM/docs and no unsupported all-in 70% claim |
| J — Complete sourcing and manufacture | Reconcile every occurrence/mate/process/tool; broad China/western exact-offer search; landed cost, tolerance-grade authenticity and shop process capability | Zero undefined selected lines or hidden assembly parts/operations; dated line-level prices/valid quotes and full baseline/new total; draft exact unsent RFQs for genuinely private prices |
| K — Integrated verification | Execute M01-M48 at justified fidelity, native readbacks, uncertainty/worst cases, source/target/HIL and independent safety review | Complete trace/margin ledger with raw hashes; fix failures and rerun affected cases; numerical design claim limited to executed evidence |
| L — Prototype and qualification | Incoming/assembly/electrical/safety/mechanical/coupon/thermal/EMC/process/endurance trials in safe dependency order; integrate measured discrepancies | Actual P00-P15 records where hardware is available, corrected design and validated envelope; future scheduled tests remain honest and actionable |
| M — One manual and release | Generate final manual from frozen source/parts, independently build/walk it, visually inspect every page, reconcile counts/cost/revisions, clean release bundle | Exactly one final visual assembly PDF, complete editable sources/native exports/data, reproducible release and concrete evidence-backed scope; no conflicting active guide or missing part |

Do not postpone sourcing until after designing around a nonexistent component. Conversely, do not freeze a bargain component before its load/accuracy/temperature/mating constraints are known. Iterate phases C-J with a controlled configuration and cheap feasibility bounds/coupons first. Stop spending effort on a candidate as soon as a real hard constraint disqualifies it, record why and select a viable alternative.

When a needed supplier value is private, prepare a complete RFQ packet: exact drawing/revision/configuration, material/grade/tolerances/inspection, quantity and sample/production split, included kit items, delivery destination/Incoterm, currency/tax, lead time/validity, certificate/acceptance and substitution prohibition. Search available exact public alternatives and complete dependent engineering with explicit conservative bounds. Sending the RFQ is a separate user-authorised action; this chat authorises drafting, not messaging suppliers. Do not replace a missing quote with zero, an unqualified price-range minimum or a made-up firm offer.

Keep source checkpoints durable in git and provide reviewable patches/commits. Do not lose days of implementation as unpushed ephemeral work again. Follow the user's repository/publication authority and any applicable instructions; no forced rewrite, destructive cleanup, merge or purchase follows automatically from this handoff. A final reviewable branch/patch and actual artifact set are preferable to a narrative claiming changes that are unavailable.

## 16. Reproduction, tools, evidence layout and automation

### 16.1 What is in this kit

- `START_HERE.md`: concise takeover entry point.
- `ENGINEERING_HANDOFF.md`: this entire task, requirements, implementation standards, models, qualification and manual contract.
- `data/`: original 47 goals, 217 historical parts rows, 54 fabrication rows, 52 historical price rows, fresh limited price observations, model/goal mappings and unexecuted thermal-test planning templates. Historical rows remain explicitly dated; they are not final selected/priced build records.
- `images/`: the three supplied actual photographs, with the limitations in section 5.
- `history/`: recovered audit Markdown and extracted historical engineering-report text. Old defects/statuses are context, not an assertion that they persist after the upstream fix wave.
- `evidence/`: tree inventory, actual fresh host-test/provenance records, independent arithmetic/CSV review, pinned source-fact ledger and the deliberate stock-side counterexample source/log/result. No raw new native FEM or physical test is represented here.
- `tools/`: runnable host/counterexample reproduction and handoff package/inventory validation helpers. They operate on a separately obtained source tree; the kit is not a duplicate of the repository.

No PDF is supplied in this handoff. Exclude compiled test executables, credentials, SDKs, caches, a whole duplicate repository and unrecovered imaginary patches from the kit. Supplied executable **source** and build commands are sufficient to reproduce the demonstrated counterexample. The completion agent must create the actual finished machine deliverables, including its one final PDF, later.

### 16.2 Establish a complete checkout and repeat the actual handoff tests

Use the connected GitHub capabilities or an authorised git connection. The following are commands for the completion agent, **not claims that this handoff executed a fresh full clone/native build**:

```bash
git clone https://github.com/sbelakho2/Cutting.git Cutting
git -C Cutting status --short
git -C Cutting rev-parse HEAD
git -C Cutting log -1 --format=fuller
```

If HEAD has advanced, inspect its diff from `0f28aa3289cf052409ad48f54dc5d23b130fd441` and preserve the new work. Reproduce the pinned historical demonstration in a separate detached worktree if necessary, not by resetting a user's working tree. Read all applicable `AGENTS.md` files. The handoff environment had only a sparse source checkout; a fetch/restore of additional blobs encountered a network-policy restriction. A missing local file is not proof it is absent upstream; use the tree inventory/connected read. Documents 45/46 and lost R4/R7 source really are absent from the pinned tree, as distinguished above.

Run from the unpacked handoff directory, with absolute paths substituted:

```bash
bash tools/run_host_checks.sh /absolute/path/to/Cutting /absolute/path/to/host-evidence
```

This compiles the pinned core with GCC C++20 warnings-as-errors plus ASan/UBSan, runs the 21 host groups, then compiles/runs the sensor counterexample. A successful **counterexample reproduction** means the old design defect is exposed; it is not a machine-function pass. Read the Python tool before applying it to a materially changed fixture/controller. Its pattern/hash expectations are intentionally narrow. Do not alter production source to make the historical demonstration appear to pass.

The observed compiler for this handoff was GCC 13.3.0 on Ubuntu; exact source hashes and sanitiser scope are in `evidence/review-provenance.json`. No current Pico cross-build or native CAD/FEM run is claimed. Source-level arithmetic in `source-and-calculation-review.json` has different scope from a mesh solve.

### 16.3 Full native and target programme

Inspect the repository's current `CMakePresets.json` and CMake definitions before running. Pinned presets include `core`, `sanitize`, `engineering` and `engineering-sanitize`; they define different build directories and dependencies. Typical commands are:

```bash
cmake --preset core
cmake --build --preset core
ctest --preset core
cmake --preset engineering
cmake --build --preset engineering
ctest --preset engineering
```

Run these in the complete repo. A whole-repository CTest run includes integrity/record/geometry gates beyond the core host executable; source fixes and honest unqualified physical-record gates may require interpretation by scope, not suppression. Preserve the full output and distinguish test-harness success from final design acceptance. Do not amend a fail-closed test to accept blanks merely to make CI green.

The pinned engineering CMake requires Eigen3, **OpenCASCADE 7.9.3**, CHOLMOD configuration and LibXml2; Qt6 Gui/Svg supports drawing generation, and actual mesh generation/optimisation uses the relevant source/tool versions. Pin gmsh/Netgen and any solver library, compiler, BLAS and threading configuration used. A stale `/usr` OpenCASCADE header/library can silently replace the intended version: inspect configure/cache/link output, exported package configuration and runtime libraries. Record native solver/toolchain versions, resource caps and exact options. Do not assume package-manager latest is ABI-compatible or physically validated.

The real target is a **separate** project in `software/machine/rp2350`, with `PICO_BOARD=pico2`. After installing/pinning an actual supported Pico SDK, ARM toolchain and picotool, configure a separate build, for example:

```bash
cmake -S software/machine/rp2350 -B build-rp2350 \
  -DPICO_SDK_PATH=/absolute/path/to/pinned-pico-sdk -DPICO_BOARD=pico2
cmake --build build-rp2350 --target starz-machine if-b01-factory-test
```

Check generated UF2/ELF/map and reserved journal sectors against the **actual delivered flash** and runtime implementation. The pinned target reserves two 4 KiB sectors from a 4 MiB Pico flash map and assumes core 1 is unused for flash safety; a later multicore change invalidates that assumption. Compile success is not proof of silicon-version interpretation, real-time execution, flash wear or safe boot. Test both images on their intended hardware/fixture and never use the factory image to operate production machinery.

Regenerate native KiCad from the final controlled source, then route/review without a generator overwriting the routed result. Pin KiCad and footprint/symbol library versions; verify the source/export relationship, all ERC/DRC/unconnected/parity checks and actual MPN/pad/mate geometry. Export Gerbers, drills, IPC/net/placement/BOM and inspect the readback. Similarly export each actual CAD part/assembly and drawing, re-import and compare. Tests must include negative cases that reject the original wrong connector, screw, sensor position and unfinished interface.

### 16.4 Required final evidence organisation

Use a clear structure such as `requirements/`, `configuration/`, `procurement/`, `manufacturing/`, `hardware/`, `software/`, `analysis/Mxx/`, `tests/Pxx/`, `assembly/` and `release/`. Adapt repo names to existing conventions; these are logical owners, not a demand to move every original file. Each study includes a `manifest.json`, assumptions/validity, inputs, raw outputs, convergence, balance, uncertainty, limits and result/margin table. Large fields belong in durable versioned artifact storage with hashes and retrievable locations; a link without retained data is insufficient.

Automate these meaningful release checks:

1. Every original/new requirement has implemented verification and current evidence or an explicitly unclosed physical gate; no fake fully-qualified status.
2. Every CAD/PCB/harness occurrence and mating half maps to a BOM/make line; counts, purchased packs and assembly step quantities reconcile in both directions.
3. Every selected part is exact and adequately rated/qualified, with a current price or valid firm quote, and every fabrication operation/tool/consumable/service item has a cost basis.
4. Every manual part/torque/dimension/wire endpoint comes from the frozen current definition; every part and operation is covered; all images are current geometry.
5. Generated source/output/firmware/analysis hashes match the frozen build. A changed support, LED resistor, fan, wire recipe or sensor invalidates its dependent evidence, rather than inheriting an old green report.
6. Cost totals balance from decimal line calculations; currency/tax/landed scope is consistent, no kit double counting and no blank treated as zero.
7. Exactly one builder-facing PDF exists in the final release, with the required complete content; historical/vendor PDFs may remain research sources in the repo but are excluded as competing builder instructions from that release bundle.
8. Visual QA and an independent builder review are recorded; programmatic ID coverage does not alone prove that an illustration is readable or a part can be assembled.

Make checks fail on meaningful counterexamples. A statement such as 'all parts verified' in a JSON file is not independent evidence. Never hash an unsupported assertion and call the hash proof of truth.

## 17. The completion agent's single visual PDF: exact deliverable contract

### 17.1 One canonical, complete builder document

**The handoff agent does not create this PDF. The completion agent must create exactly one final `Cutting_Assembly_Manual.pdf`.** It must describe the actual found, selected, purchased and made parts and the final verified configuration. All builder-facing assembly, fabrication/detail sheets, cabinet/wiring, payoff, commissioning, maintenance and acceptance instructions must be in that one PDF. Do not deliver several partial PDFs, a menu of incompatible variants or a compilation that tells the builder to resolve contradictory original guides.

Native CAD/STEP, PCB/Gerber, firmware, CSV/JSON data and evidence may accompany it; they support the one manual. Manufacturer information can be referenced for traceability, but the essential dimension, connection, torque, assembly or operating step cannot depend on a disappearing supplier link. Historic/vendor/reference PDFs in the research repository are not additional final assembly manuals and must not be presented as competing builder instructions in the release package.

The manual must identify release/build ID, design and firmware revisions, date, actual supported wire/reel/environment/rate scope, required skill/tools and evidence-backed qualification status. If the prototype has not physically passed qualification, it must clearly be a controlled pre-release build/qualification manual, not a claim of a guaranteed production machine. Do all feasible design/procurement/document work; do not omit unresolved parts under a polished 'final' cover. It becomes the final released manual only when the release gates are met.

### 17.2 Required contents in builder order

1. **Orientation and finished machine.** Accurate overall/isometric and functional views, coordinates, cutter, stock/piece side, cabinet, both reel modes, buffer, waste and output; actual footprint/clearances/weights, installation/floor/bench/air/mains/site requirements and safe lifting/transport locks. State the qualified operating limits clearly.
2. **One complete purchasing and make list.** All actual selected MPNs/suffixes/supplier links, machine quantities, pack/MOQ purchase quantities and contents, made-part drawing/revisions, fabricated assemblies, consumables, fasteners, wiring, mating connectors, guards/adapters and spares. Include dated price/landed-cost summary with scope, and a label scheme for kits. No unspecified 'M3 hardware', 'suitable cable' or unpriced invented bracket.
3. **Tooling and inspection.** Exact ordinary/precision tools, gauges, crimp dies, torque ranges, fixtures and access; calibration/uncertainty needs, make/buy/rent/outsourced route and cost. Distinguish routine assembly from grinding/matched-bearing inspection and electrical/safety testing that require qualified capability.
4. **Every made part and operation.** Legible dimensioned plan/section/detail sheets or controlled manufacturing tables for all actual custom parts; materials/condition, datums, tolerances, threads, surfaces, hardness/finishes, quantity and inspection. Include weld/heat-treatment/grinding sequence and post-process distortion checks. Native files may supplement these sheets; the PDF must not force the builder to reverse-engineer a STEP model to discover a missing hole or depth.
5. **Frame/base and stationary datums.** Assembly sequence, fasteners, seating/locating surfaces, actual levelling/anchoring, squareness/flatness/alignment measurements and acceptance, with no invented rigid bench assumption.
6. **Both complete cutting axes.** Actual screw/nut/support/guide arrangement and bearing orientation/preload, receiver and stationary motor mounting, adapter/washers/bolts, correct coupling insertion and shaft gap, carriage/tooling/backing/blades/shims. Show the real load path, independent jaws, datums, full thread/drill depth, torque/preload controls, lubrication, travel/homing and loaded-gap setup/check. Explain left/right identity even if parts are shared.
7. **Both feed cassettes and all nips.** Actual shafts/bearings/pairing, race-specific fit procedure, collars/keys/pulleys/belts, driven lower/passive upper roles, springs/adjusters/locks, threading/opening, tension and force checks at all stations. Include measuring method and allowable wear/loaded radius; a nominal spring protrusion alone is insufficient.
8. **All metrology modules and real sensing.** Wheel/encoder/drag geometry, actual stock/piece placement and contact-valid phases, index/phase hardware if implemented, brackets/wiring, target gaps and calibrated force/loaded circumference. Show the independent calibration reference and how to identify/preserve each wheel/map identity.
9. **Large and small reel modes.** Actual cradle/arbor/cassette parts, adjustable range/retention, loading route for 30 kg, drive/brake assembly, guards and sensors. Provide a clear full reeving drawing with numbered pulleys/spans, direction arrows, active storage/travel, small/large changeover and every adapter. Include no-loose-rewind behavior and end-of-reel/jam/power-loss checks. The pictured OEM arrangement is inspiration; show the final project's actual geometry.
10. **Waste and finished output.** Prime stub and both slug paths, sweep/chute/air components if selected, support/conveyor for 200-3000 mm parts, bins/guards, part-clear sensors and actual adjustment/check. Show why a short stub is removed and where slugs go; do not hide them behind a closed guard illustration.
11. **Cabinet mechanical build and cooling.** Exact layout/cutout dimensions, component positions/orientations and clearances, filters/fans/outlets/baffles, glands/bend radii, separation/PE, probe/flow-device placement, service removal access, filter identifiers and airflow arrows. The retired cooler and T drivers must not remain as active illustrated choices.
12. **Electrical build and PCB/harness.** Complete readable schematics and terminal/wire schedule with endpoints, pin numbers/polarities/cross-sections/colours/ferrules/labels/torques, connector mates, PE/shields, fuses and DC pole arrangements. Show final board/component orientation and sockets/mounting, production versus fixture, all supplies/logic/MF/permissions, regen/discharge, independent safety/reset/EDM/guard locking, payoff/output and panel. Break dense schematics into indexed legible sheets **inside the same PDF**, with cross-references.
13. **Firmware and first power.** Exact approved binaries/hashes and target, factory fixture procedure, silicon check, safe flash method, driver settings and readback, uncommissioned startup, electrical/safety prerequisites, low-speed first motion, directions/homing and actual approval process. Do not energise motion before independent safety/electrical prerequisites. Factory fixture and production use must be visibly distinct.
14. **Calibration and commissioning.** Every dimensional/force/tension/polarity/calibration step, independent instruments, actual recipe limits and acceptance. Progress from 0.05 m/s to the qualified 0.50 m/s only using defined gates; qualify maximum strips/shortest and longest parts. Include cabinet steady/fault checks, bus/stop/guard release and actual panel workflow. Provide fillable/printable travellers with record IDs and outcome fields, not prefilled fabricated passes.
15. **Routine screenless operation.** Approved recipe/material loading, reel changes, START/STOP/RESET/repeat/batch completion, lamp/buzzer states, exact safe fault recovery and scrap/quarantine counting. Explain what invalidates approval and requires service/setup, including changed wire, blade, roller/wheel, driver settings, firmware or sensor topology. No routine PC connection required for an approved recipe.
16. **Service and troubleshooting.** Measurable filter/flow, roller/tread/force, blade/gap, belt/lube, brake and sensor limits; spare IDs; safe isolation/restraint; replacement/cleaning tools and steps; required reinspection/recalibration/reapproval. Troubleshooting connects a symptom/diagnostic to a test and safe corrective action, never 'disable the interlock to continue'. Include service access and maintenance records.
17. **Final acceptance and traceability.** Concise actual release scope, requirement/test references, complete build/inspection/commissioning traveller, safety and service checks, parts/step/drawing index and change/revision identification. Link detailed raw analysis as support without forcing the assembler to interpret FEM to know a bolt torque.

### 17.3 The contract for every illustrated assembly step

Give every step an immutable ID and prerequisite. Show a true view of the receiving assembly and the added parts, with an orientation cue and labelled datums. List exact part IDs/revisions/quantities and fasteners/washers in insertion order, the operation, tools/fixture, direction/depth/setting, qualified torque/preparation if relevant, cable/pin endpoints, hazards relevant to that step and an objective inspection/acceptance. Show hidden contacts/threads using a real section or cutaway. Show the assembly state **after** the step, rather than an exploded cloud with no sequence.

A useful step record has fields `step_id`, `prerequisites`, `configuration`, `added_occurrences`, `make_or_buy_ids`, `tools`, `operation`, `datum`, `setting_and_units`, `torque_and_condition`, `inspection`, `acceptance`, `drawing_view_ids` and `next_gate`. Populate it from the frozen design. Do not fill arbitrary values to satisfy a schema. A step that says 'fit correctly', 'adjust as needed', 'wire according to manufacturer' or 'use equivalent parts' is incomplete unless the exact decision and check are fully supplied in the same document.

Prefer standard tools, positive locators, simple independent alignment and pre-labelled kits. Exploded views must show fastener axes and stack order; sectional details must be real solids, including central bores/counterbores/washer seats. Use distinct stock/piece and left/right labels, colour plus line styles/symbols, and dimension arrows that cannot be confused with motion arrows. Photographs may support real built details; label them by actual build revision. Do not use generated/AI imagery as precise geometry, a wiring truth source or evidence that a prototype exists.

### 17.4 Visual quality and native-source fidelity

Generate precision illustrations from final CAD/PCB/net/drawing data. Use vector linework/text where possible and sufficiently resolved photographs/raster views. Choose a consistent page format (A4 with occasional clearly indexed larger sheets if necessary), practical print margins, embedded fonts, legible callouts and body text normally >=10 pt. Dense schematics require larger/sectioned views, not tiny labels. Colour is helpful, but the manual must remain unambiguous in greyscale. Give page/step references, bookmarks/table of contents, searchable text, units and an issue/revision footer.

Assembly pages must be meaningfully visual: actual exploded/isometric/section/detail views explaining the operation, not prose pages decorated with a generic machine render. Do not impose a fixed page limit that causes omissions. Avoid duplicated conflicting tables; generate values from controlled data where possible. Do not copy unsupported 'fabrication-ready' claims from older README/HTML/PDF pages.

Render **every page** at a useful reading resolution, inspect all pages and large detailed views, and fix clipped/overlapping/missing glyphs, off-page tables, illegible dimensions, ambiguous arrows, wrong sections and obscured small parts. Automated text bounds/link checks supplement visual review. Verify the actual final PDF after its last regeneration; a previous preview is not enough. Retain a page-level QA record, render/version hashes and a review of critical drawings against the actual native solid/netlist.

### 17.5 Bidirectional completeness and an independent build

Run an occurrence-to-BOM-to-manual coverage check: every physical occurrence, mating half, consumable/process and operation is bought/made/covered in at least one appropriate step; every step's part really exists in the selected build; counts reconcile with kits and purchase packs; every torque/dimension/wire endpoint agrees with the frozen source. Cover the invisible hardware, harness lengths, glands/ferrules/strain relief, masks/finishes, transport locks and service tools. The manual's complete costing must reconcile to the procurement register without hidden extras.

Have a competent person or independent completion review follow the one PDF from counted parts through commissioning without the chat as an undocumented instruction source. A digital walk-through is useful while hardware is absent, but label it as such; physical assembly remains a separate validation. Log every question, missing item, blocked access, ambiguous measurement and rework; correct design/source/steps and re-run coverage and visual QA. Record assembly and calibration effort and use it to substantiate 'easy to put together'. A syntactically complete manual is not proof that real components fit.

## 18. Definition of done, remaining decisions and permissible final claims

### 18.1 Hard completion gates

The engineering task is complete only when the final configuration has all of the following:

- All 47 original goals and later instructions retained, atomised and satisfied for the declared operating envelope, with no quiet requirement reduction.
- Every selected bought part is an exact adequate identity, actually found and currently priced/validly quoted; every custom part and operation is fully defined, feasible to manufacture/inspect and costed. Every kit/mate/occurrence is reconciled. No unqualified substitute remains in the selected build.
- Real integrated CAD/PCB/harness/firmware and actual full axes, sensing, automatic waste/output, both reel modes, guards and cabinet agree. No phantom sensor, impossible prime ejection, hardcoded production clearance, incompatible shaft/plug or missing access.
- Original strength/proof/stiffness/fatigue/fit/shaft-heat/loaded-gap/traction/metrology/speed and factory-environment requirements pass with justified uncertainty and margin. Exact component capacities apply to the actual purchased grade/variant.
- Ventilation replaces the EUR1669 cooler using suitable drivers and actual filtered installed flow, local thermal mapping and independent adequate protective behavior. All real heat/protection hardware and added payoff loads are counted.
- All M01-M48 decisions are closed at justified fidelity with actual reproducible evidence; physical P00-P15 prerequisites and qualification for the claimed release scope are genuinely executed and reviewed. An analytically bounded inapplicable effect has a documented rationale, not an unexplained 'N/A'.
- Target and fixture images actually build; production startup/approval/polarity/timing/storage/handling and screenless workflow are verified; independent safety is validated for the risk-derived required performance.
- One complete like-for-like baseline/new landed-cost comparison states the actual saving and uncertainty. A ~70% target is pursued across the whole build and is not falsely inferred from a cooler-core-only subtotal. Robustness improvements are demonstrated, not claimed from lower price or newer part names.
- Exactly one canonical complete visual `Cutting_Assembly_Manual.pdf`, generated and reviewed under section 17, is delivered **by the completion agent**, with native sources/exports/data and an independent assembly/commissioning review.
- Reviewable source changes and evidence are durable/retrievable; final release hashes tie every source, native export, selected part, binary, manual and test to the same build.

If hardware, normative text or private quotes are genuinely unavailable, finish every independent desk-solvable implementation, prepare concrete test/RFQ artifacts and expose the exact remaining gate. Do not declare the machine or the manual production-qualified. Do not stop at another mostly-'BLOCKED' report and do not invent the missing evidence. Keep going until the user's actual scope is finished or a specific external action is truly required.

### 18.2 Decisions to settle early without paralysing independent work

| Decision | Known now | Required resolution and work that can continue |
| --- | --- | --- |
| Final installation/delivery | Tunisia is referenced in the sourcing brief; 40 C room is required | Confirm exact address/Incoterm, mains range/frequency/earthing, altitude/pressure/RH/dust, bench/floor and any air; calculate conservative proposals and keep researching exact offers |
| Throughput matrix | 0.50 m/s feed; 200-3000 mm parts; no single complete accepted-pieces/hour target is frozen | Derive physically achievable rates and bottlenecks first, propose requirement by recipe/length and seek a concrete decision before claiming compliance |
| Reel geometry | Up to 30 kg; user wants both small and large packages | Collect actual package examples and freeze OD/core/width/bore/flange/empty mass/bend radius/loading scope; design interchangeable adapters around evidenced packages |
| Material/quality details | Stranded copper/PVC, 0.5-2.5 mm2, OD1.2-4.5 | Exact constructions/lots/temperature and application quality criteria; identify tests and safe recipe boundaries rather than assuming every PVC behaves alike |
| Process/shop capability | Custom components proposed, actual grinding/matching needed | Demonstrate shop capability or choose a priced qualified outsourcing route; own CNC access alone is not micrometre inspection capability |
| Target cost baseline | About 70% reduction requested; EUR1669 cooler reference | Reconstruct equivalent full build from dated records with consistent tax/scope; compare actual qualified alternatives and avoid invented overall savings |
| Risk/reliability confidence | Original Cpk and lifetime/duty targets exist | Derive safety functions/PLr from actual risk; adopt explicit process/reliability confidence policy and actual service/mission profile; do not invent a universal 100% guarantee |
| Physical resources | No built machine/test data demonstrated in the handoff | Plan exact fixture/instrument/prototype access and qualification while completing source/model/procurement work; no simulated result relabelled as hardware |

### 18.3 What the completion agent should say at its finish

Lead with what is actually finished and usable: exact build/revision, one PDF, source/native files, all-part sourcing/cost and the measured/verified operating scope. State important failures/limits plainly, with the concrete remaining action only if any remains. Include actual like-for-like savings, what improved and the evidence. Do not claim global cheapest, guaranteed universal 100% operation, tested hardware, certified safety or all parts priced unless the corresponding evidence really exists.

The handoff supplied here is already a finished **task specification**, not a finished machine. Its reproduced host/counterexample evidence helps the next agent begin correctly; it is not the final design or physical qualification.

## 19. Source index and evidence priorities

URLs below were read or rechecked for the stated facts on 29 October 2026 unless explicitly labelled historical. Save exact source versions/access dates and refresh time-sensitive observations before final sourcing. Primary manufacturer limits take precedence over a reseller's general description. A source proves only the exact fact/configuration it states, not every property needed for qualification.

| Source | Purpose / evidential limit |
| --- | --- |
| https://github.com/sbelakho2/Cutting/tree/0f28aa3289cf052409ad48f54dc5d23b130fd441 | Pinned upstream. `evidence/repository-tree.csv` gives actual path/blob identities; current generated reports can be stale relative to source |
| https://www.omc-stepperonline.com/y-series-v2-0-closed-loop-stepper-driver-0-7-0a-24-50vdc-for-nema-17-23-24-stepper-motor-cl57y-v20 | Direct seller CL57Y-V20 price/identity; shipping/tax/site offer not established |
| https://www.omc-stepperonline.com/y-series-closed-loop-stepper-driver-v2-0-0-8-5a-20-80vac-30-110vdc-for-nema-34-stepper-motor-cl86y-v20 | Direct seller CL86Y-V20; conservative manual-table rating governs unresolved marketing conflict |
| https://files.omc-stepperonline.com/CL57Y_V2.0_pulse_stepper_driver_manual_V1.0.pdf | Exact candidate feed-driver manual; verify delivered revision and current conventions |
| https://files.omc-stepperonline.com/CL86Y_V2.0_pulse_stepper_driver_manual_V1.0.pdf | Exact candidate head-driver manual; verify delivered revision, MF and input limits |
| https://sw-us-production.pfannenberg.com/PF-66000-230V-54-7035/11666102055 | Exact fan manufacturer's page; use exact variant, not another bearing/voltage brochure |
| https://www.pfannenberg.com/de/klimatisierung/filterluefter/filterluefter/pf-66000/ | Manufacturer fan family/curve/filter information; matched-path data, not installed-flow proof |
| https://industry-electronics.de/pfannenberg/11666102055-pf-66000-230v-54-7035-filterlfter-pf66.000-230-ac-lieske_1601220.htm | Fresh seller price observation; latest read showed only one unit, while candidate needs two; supplier lead-time is not confirmed Tunisia delivery |
| https://industry-electronics.de/pfannenberg/11760002055-pfa-60000-ip54-4.-generation-austrittsfilter-lieske_1236637.htm | Exact outlet price observation; no freight/tax inference |
| https://industry-electronics.de/rittal/3114200-digitale-temperaturanzeige-regelung-fr-100230-lieske_502047.htm | Thermostat price observation; obtain primary manufacturer's detailed behavior/ratings. Check its stated 24-60 V DC supply against actual minimum control voltage, not merely nominal '24 V' |
| https://toshiba.semicon-storage.com/info/datasheet_en_20230821.pdf?did=60600 | Current fetched content identifies TLP176AM revision 7.0.A dated 7 April 2026 despite the older filename. Use the actual document revision, input-power derating and guaranteed switching conditions |
| https://pip-assets.raspberrypi.com/categories/1214-rp2350/documents/RP-008373-DS-2-rp2350-datasheet.pdf?disposition=inline | RP2350 silicon/E9/CHIP_ID facts; check pinned SDK revision mapping and actual incoming silicon |
| https://www.schleuniger.com/en/products/peripherals/prefeeder/prefeeder-1000/ | Architecture/reference rating; not proof of the pictured model or this project's 30 kg performance |
| https://www.schleuniger.com/en-us/products/peripherals/prefeeder/prefeeder-1100 | Different belt-puller/dancer reference; do not mix its specs with other models/regions |
| https://www.iso.org/standard/51528.html | ISO 12100 catalogue/edition; full normative application still requires the applicable text |
| https://committee.iso.org/standard/73481.html?browse=ics | ISO 13849-1: 2023 edition 4 catalogue; does not assign machine-specific PLr |
| https://webstore.iec.ch/en/publication/71256 | IEC 60204-1 consolidated edition 6.1 catalogue |
| https://standards.nasa.gov/node/263 | NASA model/simulation credibility reference; not machinery compliance |
| https://standards.nasa.gov/sites/default/files/standards/NASA/B/1/NASA-STD-7009B-Final-3-5-2024.pdf | Actual 7009B reference document for model credibility |
| https://www.itl.nist.gov/div898/handbook/pmc/section1/pmc16.htm | Capability methods/assumptions; not actual machine data |

The three user photographs are supplied originals, not manufacturer-certified drawings. The historical tables and audits retain their 13 October dates and pre-fix-wave scope. Price observations are neither purchase orders nor guarantees of stock/delivery. Read the uncertainty/qualification column before treating any catalogue fact as sufficient evidence for the complete part.

## 20. Copy-ready task for the completion agent

> Take over https://github.com/sbelakho2/Cutting using `ENGINEERING_HANDOFF.md` and its supporting kit as the detailed task specification. This is an instruction to implement and finish the machine, not write another audit. Check fresh HEAD and preserve user changes; the handoff pinned main at `0f28aa3289cf052409ad48f54dc5d23b130fd441` on 29 October 2026. The earlier local R4/R7 source, binaries and raw fields were lost and are not upstream; reconstruct or improve the detailed candidates and regenerate real evidence.
>
> Preserve and satisfy every one of the 47 original goals and later user instructions: accurate single-lane stranded Cu/PVC cut-and-independent-end-strip, 0.5-2.5 mm2/OD1.2-4.5, L200-3000 mm, 0 or 3-25 mm strips, original length/strip/Cpk/load/proof/life/fit/thermal/speed requirements, robust factory operation, screenless routine use, easy assembly and complete automatic handling. Design an economical, qualified small/large-reel payoff up to 30 kg using the photos as inspiration with explicit reel limits. Find, qualify and price every purchased/made part, mating item, fabrication/inspection operation, tool and consumable. Search Alibaba/Made-in-China/LCSC and direct Chinese manufacturers as well as western sources; minimise adequate complete landed cost and pursue about 70% like-for-like savings with demonstrated robustness improvements. Do not replace exact qualified parts with unverified catalogue families or weaken requirements to get savings.
>
> Start with the demonstrated stock-side encoder/material-state contradiction, impossible 10 mm stub ejection and missing real part/slug-clear/output hardware and I/O. Preserve and use the fresh counterexample as a negative control; replace the controller's idealised motor-to-sensor test plant with real material topology/occupancy/observability. Then close actual axis internals/support/guide/adapter/tooling geometry and loads, feed pulse/radius/traction/metrology/error budget, shaft heat/fit/fatigue, full payoff/buffer dynamics, electrical/PCB/MF/logic/protection/regen/guard safety and cabinet integration.
>
> Replace the EUR1669 cooler with adequately rated drivers plus genuinely adequate filtered ventilation. Optimise actual losses/fan heat/flow/cost together, perform coupled thermal calculations/CFD/transients and protective fault analysis, run available numerical/target tests, prepare the exact 18 steady-state and 17 fault thermal scenarios and execute physical tests only with actual hardware/instruments. All M01-M48 studies and P00-P15 qualification tasks in the handoff require decision-quality, reproducible, scope-correct evidence, uncertainty, convergence, independent checks and relevant physical correlation. Never relabel synthetic data as physical, borrow ratings from another part, inherit stale reports or claim universal 100% reliability.
>
> Produce one coherent selected build with native CAD/PCB/software, complete sourced/made/costed registers, real manufacturing/inspection definitions, verified binaries/exports and durable raw evidence. **You, the completion agent, must create exactly one complete visual `Cutting_Assembly_Manual.pdf` under section 17. The handoff agent must not produce that PDF.** Base every illustration and step on the actual final selected parts; cover all custom drawings, assembly, wires/terminals/torques/tools, both reel modes and reeving, cooling, commissioning, calibration, screenless operation, maintenance and acceptance. Reconcile every occurrence/part/operation to the manual and independently build/walk it; render and inspect every final page. Keep all builder-facing PDF sheets inside that one canonical manual.
>
> Continue until the implemented goal is complete. If a genuinely external input is needed, finish all independent work and present the exact reviewable question/test/RFQ rather than a mostly-'BLOCKED' report. No supplier messages, purchases, merge or deployment are authorised merely by this task; prepare exact drafts/reviewable source without sending or buying. Report only evidence-backed completion, actual savings and the precise qualified scope. Do not import constraints from unrelated user projects.
