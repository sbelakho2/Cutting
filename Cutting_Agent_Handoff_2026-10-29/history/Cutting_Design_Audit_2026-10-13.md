# Cutting: complete documented-goal and buildability audit

**Repository:** https://github.com/sbelakho2/Cutting  
**Reviewed commit:** `40b8b2d99752bff7cad8c176e13fca9998a96384`  
**Date:** 13 October 2026  
**Verdict:** **NOT ready to build exactly as instructed, NOT self-consistent, and NOT demonstrated to meet the accuracy, handling, safety, or reliability requirements.**

The project contains substantial useful engineering work. Its matched-bearing records, shaft studies, source specifications, controlled templates, and conservative separation of simulation from physical evidence are worth preserving. Nevertheless, following the current guides with correctly supplied components would still encounter specific electrical, software, mechanical, and manufacturing failures. Several are deterministic; they are not merely tests awaiting a prototype.

This audit evaluates all 40 goals in `docs/41-goal-traceability-matrix.md`, the operating requirements in `docs/01-design-basis.md`, and additional build, commissioning, dynamics, endurance, and usability requirements elsewhere in the repository. The earlier original conversation could not be recovered reliably. Therefore, this is complete against the goals currently documented in the repository, but cannot certify that undocumented historical requirements have been preserved.

No physical machine was available. No hardware was energised. Repository files were not modified or pushed. An absolute 100% lifetime guarantee cannot be established through a repository review; the concrete achievable result is a complete, consistent build definition followed by successful, traceable qualification over an explicitly stated operating envelope. This commit does not yet have that result.

## 1. Evidence and verification

The repository inventory contains 1,369 tracked files, including 212 VTU files holding approximately 5.08 GB of simulation fields. Review covered requirements, BOMs, drawings, published manifests and reports, critical CAD implementations, production-controller and board implementations, IF-B01 schematic/PCB generation, assembly procedures, qualification logic, and the source validators.

The large archived FEM fields were not independently re-solved. OpenCASCADE/Gmsh/Eigen/SuiteSparse and the Pico cross-build toolchain were not available in this environment. Existing engineering-suite, KiCad ERC/DRC, and target-build claims are consequently distinguished from checks executed in this audit.

### Checks executed on the pinned source

| Check | Result | Meaning |
|---|---|---|
| Production machine core, compiled directly with GCC C++20 | All 13 existing test modes passed | Profile, validation, homing, cycle, correction, slip, stop, safety, board, jog, storage, journal, protocol logic on `SimBoard` |
| Legacy motion/control test executable | Passed; 9 internal groups | Non-hardware reference behaviour |
| Legacy protocol executable | Passed; 6 internal groups | Non-hardware protocol/HMI policy |
| Existing qualification arithmetic unit executable | Passed | Synthetic acceptance arithmetic |
| `check_production_records.py` | Passed | 116 BOM rows, 9 sourcing records, four controlled blank templates |
| `check_design_integrity.py` | Passed | The checks implemented by that script |
| `check_total_consistency.py` | Passed | The checks implemented by that script |
| `check_if_b01_gpio_mapping.py` | Passed | 29 connector assignments reconciled to source |
| `check_mechanism_outputs.py` | Passed | R4, 504 occurrences, 15 guard/payoff parts, unreleased |
| `check_if_b01_kicad.py` | Passed | 100 BOM references, 90 electrical references, 278 connected symbol pins, six intentional no-connects |
| Relative Markdown targets against the complete tracked inventory | No broken targets | Files exist; their contents can still be inadequate |
| IF-B01 manufacturing-manifest hashes | 66 available files checked; **seven source hashes stale** | Two sparse-excluded binary files were not hash-checked |

The stale manufacturing-manifest source entries are `bom/selected-cabinet-wiring.csv`, `bom/selected-controls.csv`, `docs/22-controls-and-wiring.md`, `docs/23-cabinet-electrical-assembly.md`, `hardware/if-b01/README.md`, `hardware/if-b01/if-b01-cabinet-integration.md`, and `shared/controls_spec.hpp`. The archived manufacturing bundle is not a current, fully reconciled build package.

### Additional adversarial checks

| Scenario | Observed result | Consequence |
|---|---|---|
| Healthy upstream feed; no downstream measuring contact; valid 200 mm cut-only job | `Fault`, `length_slip=0x0100`, zero pieces | Required short-part operation is incompatible with the current feedback assumptions |
| Both initial feeds read 0.3% short; corrective motion produces no additional encoder travel | `Ready`, no fault, **one piece counted**, residual **854 µm** | A 300 mm part is accepted beyond its ±350 µm design tolerance |
| Normal 8 mm lead / 6 mm tail strip recipe | Stock advances **7.980 mm** while piece backfeeds **5.969 mm** | **13.949 mm overlap** at the common blade station |
| Homed blade jogged to 1,000 µm, then requested positive distance `INT_MAX` | UBSan reports signed overflow at `controller.cpp:685` | Numeric validation does not safely reject all legal protocol integers |
| Qualification file with 120 rows but only two repeatedly reused sample identities | `all_pass=1`, four cells | The tool can mistake copied measurements for 30 independent consecutive pieces per cell |

These are software/record demonstrations, not physical measurements. The overlap test traces actual motions produced by the controller through the existing simulator; it does not simulate collision forces. The downstream test represents the missing module and a short piece that cannot reach a hypothetical wheel at X=+205 mm.

## 2. Immediate build blockers

### F01 — Pico right socket is physically reversed relative to its assigned nets

**Evidence:** `tools/generate_if_b01_kicad.py`, J1/J2 definitions; `hardware/if-b01/if-b01-placement.csv`; `if-b01.pretty/PICO_SOCKET_1X20.kicad_mod`; actual PCB and manufacturing placement export.

Both socket rows have pin 1 at Y=28 mm and rotation 0°. Left row pins correctly run from physical Pico pin 1 to pin 20. The right row is assigned from physical pin 21 to pin 40, but its footprint numbers also run downward. Pico physical numbering on the right runs upward.

| Current J2 pad | Current net | Actual Pico pin when inserted beside J1 |
|---:|---|---|
| 1 | GP16 | 40, VBUS |
| 2 | GP17 | 39, VSYS |
| 16 | 3V3_MCU | 25, GP19 |
| 19 | +5V_LOGIC | **22, GP17** |
| 20 | No-connect | 21, GP16 |

The board therefore puts the intended 5 V supply on GP17 instead of VSYS and misconnects the entire right row. A netlist that agrees with its own incorrect symbol does not detect this. This blocks board assembly/energisation.

**Correction:** use one physically verified Pico-2 carrier footprint, or reverse J2's pad positions/placement correctly while retaining the intended physical-pin mapping. Check every pad against the manufacturer's pinout and socket drawing. Regenerate schematic, PCB, assembly outputs, and fixture allocation together; add a physical coordinate-to-Pico-pin regression.

### F02 — The selected resettable fuses do not fit their footprints

**Evidence:** IF-B01 BOM specifies Bourns `MF-R050`; the PCB uses `PTC_1812`, two SMD pads, no lead holes.

The manufacturer's MF-R050 is radial leaded, with 5.1 mm lead spacing. It is not an 1812 SMD part. The specified component cannot be assembled as the board and reflow instructions require.

**Correction:** choose a real surface-mount PTC with verified voltage/current/resistance/thermal limits, or use the MF-R050 radial footprint and an appropriate through-hole process. Recheck encoder supply drop at the minimum encoder input voltage and maximum cable current.

### F03 — The claimed RU3100 receiver redesign is absent from native hardware

**Evidence:** docs/13 step 9, docs/22 J8/J9, IF-B01 README say R1–R4 are removed and receiver minus inputs receive 2.5 V bias. The generator still creates four `120R` resistors across A+/A− and B+/B−. Native intent, fabrication BOM, PCB, and `if-b01-assembly.md` retain the old differential-encoder circuit. No bias divider components/nets exist.

With RU3100 single-ended A/B connected only to the plus terminals, the minus inputs do not obtain the specified reference. With the terminators fitted they follow the driven plus inputs through 120 Ω, rather than forming the intended differential signal. Removing them alone leaves the minus inputs unreferenced. A prose instruction to fit an unspecified network is not a build definition.

**Correction:** implement the TTL input/bias circuit in the native source, with exact components, tolerance calculations, filtering, cable/open-input behaviour, test points, and assembly population. Update the bare-board fixture to use the actual RU3100 interface. The IO-Link master/cable needed to set TTL and 2048 PPR must be included in commissioning tooling.

### F04 — Dry-contact inputs violate guaranteed LOW levels with firmware pull-ups enabled

**Evidence:** the board uses 10 kΩ from a grounded contact to GPIO and 47 kΩ from GPIO to 3.3 V. Both production and factory firmware call `gpio_pull_up()` on those inputs.

The RP2350 datasheet gives internal pull-up resistance 32–86 kΩ at 3.3 V and maximum guaranteed LOW input 0.8 V. With a closed contact:

`V_GPIO = 3.3 × 10 / [10 + (47 || R_internal)]`.

This gives **1.136 V at 32 kΩ**, **0.964 V at 50 kΩ**, and **0.817 V at 86 kΩ**. None meets the guaranteed 0.8 V LOW limit at nominal external values. Start, reset, healthy STOP, safety diagnostic, and home inputs therefore lack guaranteed recognition.

**Correction:** disable internal pulls for externally biased inputs, or redesign the input network/buffer. With only the present external 47 kΩ, nominal LOW is about 0.579 V; then calculate resistor/supply/leakage corners and address the selected silicon revision's errata. Apply the change to the factory image too.

### F05 — Quadrature initialization blocks on its disabled TX FIFO

**Evidence:** `software/machine/rp2350/quadrature.pio` configures `PIO_FIFO_JOIN_RX`, initializes the SM, then calls `pio_sm_put_blocking()` to supply the initial AB state. Its program begins with `pull block`. Production `restart_encoder_sm()` and factory `restart_encoder()` repeat the same dependency.

The SDK explicitly defines RX joining as an eight-entry RX FIFO with TX disabled. The initializer consequently cannot deliver its startup word. This can prevent both firmware images from completing initialization. The production watchdog is enabled only after this initialization, so it does not rescue the initial block.

**Correction:** remove the TX dependency when RX is joined: initialize the previous AB state in the state machine explicitly and enter the sample loop, consistently at first boot and recovery. Alternatively redesign the capture with both FIFOs enabled and sufficient buffering. Verify first boot, reset, encoder reset, overflow recovery, and both encoder states on the real target.

### F06 — Required second measuring module is absent, and X=+205 cannot cover 200 mm pieces

**Evidence:** mechanism `status.json` explicitly says one of two measuring modules is modeled; the manifest has one wheel/encoder at **X=−205**. BOMs order two, but docs/30 P6 assembles only the upstream module. Firmware reads both counters from the same feed start and requires their residuals to agree.

The missing mirror is a design/integration task, not a physical-only gate. Blindly copying it to X=+205 would not solve the process: a finished 200 mm part occupies X=0…200 at cut and never contacts that wheel. On longer parts, the downstream wheel first becomes valid after arrival, so its total travel differs from upstream travel by the approach distance. During tail backfeed and stripping, validity changes again.

**Correction:** draw and integrate both modules, choose station positions compatible with the shortest part and maximum strips, model contact intervals, and define encoder validity/origin transfer in the controller. Compare counters over a common observed interval, rather than demanding equal whole-feed travel. Recalculate retained grip and wire routing after relocation.

### F07 — Production stripping contradicts the documented non-overlapping sequence

**Evidence:** docs/01 §4 specifies leading strip with 3 mm clearance, then cut, stock withdrawal `s2+10`, piece backfeed `s2`, score, and piece pull `s2+3`. `controller.cpp:514–546` instead advances stock by `s1` and backfeeds the piece by `s2` together, closes one blade pair, then reverses both motions without the clearance additions.

The reproduced 8/6 mm case creates 13.949 mm overlap. Independent lead/tail lengths cannot be reliably processed by scoring two overlapping ends at one station. Slugs are not positively cleared, and the priming slug has no defined removal confirmation.

**Correction:** implement the stated sequential process with explicit stock and piece tip positions, insulated boundaries, blade state, slug clearance, and encoder-validity conditions. Preserve independent lead/tail settings and zero-strip modes. Add a wire-body model that rejects overlap, loss of grip, scoring the wrong end, and next-stock restoration before clearance.

### F08 — Strip closure uses the wrong open-gap reference

**Evidence:** `strip_travel_um()` and `validate()` use `(blade_cut_mm − 0.2) − √2·r = 3.9 − √2·r`. With the stated 8 mm open apex gap, the correct ideal per-jaw closure is `4.0 − √2·r`. The sever position 4.1 mm includes 0.1 mm overlap per jaw, 0.2 mm in total.

The code subtracts the total overlap from a per-jaw value. It undercloses each jaw by 0.1 mm, leaving an ideal incision diameter approximately **0.1414 mm larger** than intended.

**Correction:** derive strip travel from `blade_open_apex/2`, then apply measured blade edge geometry and a qualified recipe incision diameter. Use the same geometry in validation, CAD, calibration, and tests.

### F09 — Failed length correction can still produce an accepted count

**Evidence:** `controller.cpp:478–502` attempts correction once. Afterwards, it cuts if both residuals agree and remain below the 1.5 mm gross-slip threshold, even if they remain outside the production tolerance/deadband.

The adverse test accepted one 300 mm piece with **854 µm residual**, no fault. Its design tolerance is **350 µm**. The gross slip threshold is not a quality acceptance threshold.

**Correction:** separate motion-fault thresholds from final part acceptance. After a bounded correction/recheck, require guarded residuals and appropriate settling/validity to meet the recipe's length budget; otherwise fault/quarantine. Verify every relevant strip/pull motion too, not only the long feed.

### F10 — Physical phase calibration cannot be used by the production controller

**Evidence:** the engineering `WheelCalibration` requires a phase map and a consistent latched Z-index reference. Production settings/protocol store only two counts-per-metre scalars. `measured_um()` applies only linear scale. Z is deliberately insulated; IF-B01 and the GPIO allocation provide no index capture.

The synthetic phase-map fitter can pass while the machine still operates without its correction. All 26 exposed Pico GPIOs are already allocated, so adding index, clearance, and other required sensing needs an explicit I/O redesign.

**Correction:** either integrate versioned per-wheel phase maps and reliable phase referencing in the target, or remove reliance on their error reduction and demonstrate that a scalar-only architecture meets the unchanged accuracy target. Bind calibration to wheel, shaft, encoder, force setting, material, temperature, firmware, and blade/build identity.

### F11 — Ejection, scrap removal, and good-part counting are not verified

**Evidence:** `Inputs` has no part-clear or slug-clear input. `Eject` advances the downstream roller by 230 mm, then `PieceDone` increments a good-part count. The design basis expressly requires a prior-present to clear transition and warns that roller travel after grip release does not prove part motion. There is no complete supported 3 m takeaway design.

**Correction:** provide the actual output support/conveyor/tray, slug collection, sensors and I/O. Count a completed cycle separately from an accepted piece; a jammed, retained, incompletely stripped, or uncleared part must not be counted good. Define quarantine and restart recovery physically and in firmware.

### F12 — The production image lacks a commissioning/recipe approval interlock

**Evidence:** the legacy model rejects `NotCommissioned`; the production image can load a numerically valid job using nominal calibration, home, and start. No persistent commissioning approval, qualified recipe identifier, or calibration validity exists.

**Correction:** default the production image to unapproved; require a controlled build/recipe approval and calibration provenance before production start. Invalidate it on relevant component, settings, firmware, or environment changes. Keep personnel safety independent of this software gate.

### F13 — The custom-axis guide tells the builder to use an incorrect steps/mm value

**Evidence:** docs/39 §3 mixes 400, 800, 2000 pulses/rev, and an incorrect pulses/metre calculation. Docs/31 explicitly instructs `steps_per_mm=800` for the 5 mm screw. The frozen source instead uses 1000 pulses/rev and 10 mm lead.

| Actual configuration | Correct blade command scale |
|---|---:|
| 1000 pulses/rev, 10 mm lead | 100 pulses/mm |
| 1000 pulses/rev, 5 mm lead | **200 pulses/mm** |
| 2000 pulses/rev, 5 mm lead | **400 pulses/mm** |

At 2000 pulses/rev and 5 mm lead, the prescribed 800 scale makes commanded 4.1 mm become approximately **8.2 mm physical travel**. At unchanged 1000 pulses/rev it becomes about 16.4 mm. A software limit in incorrectly scaled coordinates cannot protect the physical blade envelope.

**Correction:** select one exact screw lead and driver setting; derive command scale as pulses/revolution divided by measured lead. Record it in the BOM, source, switch-setting sheet, calibration, and firmware identity. Do not offer incompatible alternatives inside one build guide.

### F14 — The custom-axis drawing has intersecting tapped patterns and is not a manufacturing definition

**Evidence:** H006 is told to carry four M5 holes on PCD70 and four M6 holes on a 46 mm square; its SVG puts both patterns at the same four diagonal locations. Their corresponding centres are only **2.473 mm apart**. Even the M5 tap drill and M6 tap drill overlap; the major thread envelopes overlap further.

Moreover, H003 is a fixed-end motor adapter in the CAD but is described as attached to the carriage in docs/30 and docs/39. The drawing lacks complete nut-pocket, rail/block fixing, support-bearing, register-fit, tapping-depth, and tool-access dimensions. A 400 mm rail/500 mm screw procurement is being represented by the retained 340 mm KK86 rail and fixed-end envelopes.

**Correction:** finish the custom-axis design as an actual assembly, separating carriage and fixed motor support. Place every purchased rail, block, nut, screw, BK/BF support, and fastener from actual selected dimensions. Replace the conflicting H006 patterns and generate real H005/H006/support-part drawings. Re-run full moving-assembly collision and support checks for that geometry.

### F15 — Custom axis inherits HIWIN ratings and reduces the proof loads

**Evidence:** the stage name changes to HGR20/SFU1605, but `cutting_head_spec.hpp` retains HIWIN screw/guide/bearing capacities, stage mass, ±3 µm repeatability, and other catalogue properties. Docs/39 uses 1200 N working / 2400 N proof, while the shared acceptance envelope remains **2500 / 5000 N**. The goal matrix calls the custom option met and claims no goal relaxation.

Keeping H003's shape does not keep the new fixed support's material, stiffness, bearing rating, preload, or load path. The custom SFU/BK/BF components are not qualified by HIWIN's numbers.

**Correction:** restore the original acceptance loads in the custom-axis analysis; use component-specific verified capacities and joint/contact models. Treat old HIWIN results as historical. Keep the custom option unclosed until its actual assembly passes the same original requirements. Do not silently replace the structural proof goal with estimated cutting force.

### F16 — Two specified fabrication blanks cannot contain their CAD parts

**Evidence:** `bom/mechanism-custom-parts.csv` versus the current mechanism manifest:

| Part | Specified blank | Current CAD extent | Failure |
|---|---|---|---|
| F201 | 125×190×18 mm | **121×185×55.75 mm** | 18 mm stock cannot yield the required 55.75 mm block height |
| M206 | 75×115×220 mm | **74×112×231.915 mm** | The raised measuring-wheel bracket exceeds blank height by 11.915 mm |

**Correction:** update stock with machining allowances, or design a documented multi-piece/welded alternative and revalidate it. Add stock-containment checks derived from actual CAD bounds. A manifest listing extents is not a complete machining drawing.

### F17 — The ordered fabrication guide undercounts essential parts

**Evidence:** docs/31 says F101 “make 2” and F102–F112 “2 of each”. The design needs **eight F101 shafts**, eight of most rotor parts, and only two F111 motor plates. Its buying list gives four 626 bearings and one wheel-arm spring; the two-module BOM calls for ten 626 bearings and two springs. It also omits the complete second module/5 V supply build detail and changes the 24 V supply identity from the selected-controls record.

**Correction:** generate one authoritative, quantity-correct kit/fabrication traveller from the integrated occurrence BOM. Include commodity fasteners, pads, collars, nuts, cable assemblies, commissioning equipment, and consumables. Explicitly distinguish quantity per rotor, cassette, module, and machine.

### F18 — Payoff brake has no defined torque path from the spool

**Evidence:** docs/26 §3 says the spool rides on the Ø20 spindle through Ø20.5 flange bores with axial float; the brake disc is keyed to the spindle. Neither CAD nor the instructions define a key, clamping hub, drive pin, or torque-transmitting spool adapter. Shaft collars controlling end float do not establish braking torque.

The spool can rotate around the shaft rather than drive its brake disc. The claimed adjustable back-tension is therefore not a demonstrated function. The weld procedure also asks for Ø60 flange bores while the spool CAD/drawing guide uses Ø20.5; the specified 560 mm-span bearing/spool retention stack is incomplete. Docs/34 calls the horizontal spindle perpendicular to the base top when it should be parallel to it.

**Correction:** define a positively coupled spool hub and retention stack, or brake the spool directly. Draw the key/clamp, spacers, axial location, bearing retention, disc attachment and caliper. Reconcile weld bores and assembly datums. Test brake torque at the spool, not merely at the bare spindle.

### F19 — Payoff acceleration and braking are assessed only at full radius

**Evidence:** docs/42 uses a 30 kg solid-disc equivalent at r=0.2 m, giving 22.5 N acceleration tension. The spool has Ø60 core, heavy Ø400 steel flanges, and no powered prefeed/dancer control.

The flange pair alone has approximately **0.1184 kg·m² inertia**. At the 1.5 m/s² feed acceleration, its tension demand is `J·a/r²`: about **4.44 N at r=0.2 m**, **17.76 N at 0.1 m**, and **197.3 N at the 0.03 m core**, before brake or remaining-wire demand. A fixed 1 Nm brake becomes 33.3 N tension at that core. The empty/near-empty spool can be more demanding than the full-spool case.

**Correction:** evaluate the entire mass/radius range, including empty heavy flanges. Define radius-dependent acceleration and brake settings, a lighter spool, or a suitably buffered/powered payoff. Feed reversal also needs slack handling; a passive tension brake does not take back withdrawn wire. Include spool and cable loading/unloading in the operator guide.

### F20 — Traction is doubled by counting passive rollers as drives

**Evidence:** the design has two driven lower rollers and two passive upper rollers per cassette. Docs/37 and docs/42 use `2·µ·N` per nip and then sum two nips, yielding 80 N at minimum 50 N preload and µ=0.4.

The passive upper roller supplies reaction and rolling drag, not a second positive drive force. The driven-contact bound is `µ·(N1+N2)`: **40 N at 50 N/nip**, **48 N at 60 N/nip**, **60 N at 75 N/nip**, before losses. The reported 44 N full-spool acceleration demand already exceeds the 40 N minimum-preload bound. The strip-force reserve and near-core demand are worse.

**Correction:** use the actual driven-contact model, measure PU/PVC friction over allowed contamination, temperature, wear and wire lots, and redesign payoff/acceleration/preload to retain margin without crushing insulation. Do not inherit the 1.8× reported traction margin.

## 3. Accuracy, structural, electrical and usability findings

### F21 — Length error budget fails, and actual tension can exceed its allocation

The R2 uncalibrated allocation is **1.625 mm at 1 m** versus **0.70 mm** required. The calibrated-at-allocations result is still about **0.79 mm**, and the 200 mm cell is also over budget. These are already acknowledged failures in docs/37/42.

The tension calculation must be coupled to the payoff model. For 0.5 mm² copper, 1 m, E=110 GPa, 44 N tension, the simple elastic extension is **0.8 mm**. This is an illustrative stiffness calculation, not a guarantee that every recipe experiences that force; it shows why a 0.15 mm tension allocation and a separate 5 N assumption cannot close the worst envelope. Stranding, insulation, tension relaxation and wheel indentation remain additional terms.

**Correction:** first reduce/control tension and eliminate functional metrology gaps; then calibrate and independently qualify released, relaxed finished lengths. Include direction, phase, temperature, force, wire construction and wear. Preserve the original accuracy requirement rather than changing allocations to make a table pass.

### F22 — Blade gap/backing and qualified incision geometry remain undesigned

The nominal 20 µm blade-face gap is contradicted by the published roughly 300 µm plain-blade deflection at 1200 N. Calling stiffness “purchase-to-drawing” does not supply the missing backing, edge profile, material/heat-treatment, tolerances, inspection and vendor acceptance. Motor shaft feedback also does not prove blade-edge position after a slipping coupling or compliant support.

**Correction:** supply a complete tooling drawing and backing/support design, solve loaded clearances for actual force directions, and verify cut quality/strand damage and strip pull-off across the material envelope and worn edges. Include a force/deviation monitoring strategy where required for fault detection.

### F23 — Bearing thermal/fit closure is still a design task

The feed study reaches **29.66 K shaft-seat rise** versus the declared **20 K** envelope at its central conductance; sensitivity reaches 51.73 K. That is not a desk pass. Actual motor heat paths must be evaluated by shaft role: the directly coupled motor shaft does not receive the same belt thermal isolation as its follower.

The measuring 626 CN minimum clearance leaves only **0.5 µm before fit loss** at 10 K differential. Nevertheless, the assembly instructions still specify j5 shaft seats/K6 housings without a completed matching/thermal design. Feed C3 matching cannot simply be copied to this system.

**Correction:** finish the physical heat-path and measurement-bearing design, choose verified fits/clearance, calculate contact and tolerance/temperature corners, then measure cold-start and hot-duty behaviour. A test procedure alone does not resolve an unsafe or inaccurate fit choice.

### F24 — Accepted nip load exceeds the analyzed rotor-load envelope

The feed specification and shaft studies use roller components of ±60 N, while qualification accepts nip preload up to 75 N. Upper and lower rotors both react the contact load. A life calculation adding load to bearing resultants does not automatically revalidate local shaft stress/contact/deflection at the larger force.

**Correction:** include the full accepted force window, carrier weight, dynamics, belt loads and fault loads in the appropriate rotor-role analyses. Keep the 50–75 N operational requirement unless a reviewed change explicitly alters it.

### F25 — Guard release circuit is not an executable wiring design

Docs/22's EDM chain uses each K1/K2/K3 **21–22** NC contact between XPS Y1/Y2. The proposed release chain reuses those same contacts for a separately powered R2 coil. The terminal-level definition needed to show safe separation is absent. R2/T2 identities, release request input and the complete qualified chain are not frozen in a buildable schematic/BOM. Ordinary timer/relay failure behaviour has not been justified for the required safety function.

**Correction:** draw the full circuit with distinct, correctly rated/monitored contacts and exact terminal/device identities. Establish risk-required safety performance and validate fault cases. Release must be based on assessed mechanical standstill/stored-energy conditions as well as electrical decay. Do not classify this as “physical only” before the circuit exists.

### F26 — Regeneration/discharge and coast conclusions do not prove safe stopping

Docs/22 compares returned energy with **total** capacitor energy, rather than voltage headroom. A 100 µF capacitor at 60 V stores 0.18 J, but adding 0.10 J with no sink raises it to **74.8 V**. Whether that is acceptable depends on the actual drive/PSU voltage limit and isolation arrangement. PSU overload protection is not evidence of regenerative sinking. Actual capacitance and discharge components remain assumed/proposed.

The 345 mm coast “available distance” is measured from the cut plane; a running piece can already protrude through the aperture. Moreover, the reel cannot push the wire as a rigid taut linkage during deceleration: it can create slack. That requires separate reel, wire and roller stop/whip models, not one universal equivalent mass.

**Correction:** model each isolated bus with capacitance tolerance, upper voltage limit, losses and absorption path; specify actual discharge hardware. Measure complete stop behaviour at worst initial positions, spool radii, power-loss cases, jams and braking settings. Fix the guard release delay/monitor from those measured conditions.

### F27 — Protection and contactor coordination claims exceed their evidence

A 2 A IEC 60127 time-delay cartridge is not automatically equivalent to a 2 A gG fuse because nominal currents match. Clearing curves, breaking capacity, DC rating and protected-relay I²t must be compared. The 3.2 A current-limited supply may not clear a time-delay fuse promptly enough for the intended fault response. Supply self-recovery and contactors do not by themselves define individual branch protection. The contactor's driver-capacitor charging case also cannot be evaluated as `V/nameplate current` resistive L/R; an already running supply may recharge a discharged drive bus without repeating soft start.

**Correction:** finish protective coordination, fault-energy and inrush calculations from exact device data, document branch wiring and required additional protection, and verify the selected DC contactor make/break duty under actual driver-bus conditions.

### F28 — Cabinet budget and 5 V instructions are contradictory/incomplete

The 152 W cabinet figure includes three supply losses but omits a complete four-drive loss budget and the two small control supplies; it repeatedly counts two contactor coils where three are selected. Cooler sizing must include cabinet gains at the 40°C room case, airflow, setpoint/derating, condensation and overtemperature response. No complete cabinet layout is released for the unusually large 750×400 mm board.

Docs/28/BOM incorrectly change **Siemens 6EP3310-6SB00-0AY0** into a “second 24 V PSU” feeding an on-board regulator. Siemens identifies that exact device as **5 V/3 A**, and IF-B01 has no such 24-to-5 V converter. Docs/22's original 5 V domain is consistent with the real SKU; the later “correction” is wrong.

**Correction:** restore one verified 5 V supply definition; reconcile the 24 V SKU; complete all load, inrush, heat and physical-space calculations; add an explicit overtemperature response and qualify the drives at the stated room condition.

### F29 — Throughput arithmetic does not describe the supported controller/process

The matrix calls throughput met using an unsupported **100 mm** length. Docs/02 calculates 1000 mm feed as 2.000 s, ignoring acceleration/deceleration; at 500 mm/s and 1500 mm/s² it is **2.333 s**. Each jaw moves 4.1 mm for sever, not 8.2 mm each way. With acceleration, close/open takes approximately **0.739 s**. The required 230 mm ejection alone takes **0.793 s**, exceeding the entire 0.150 s ejection/communication allocation.

Even a cut-only, sequential ideal floor is about **2.266 s at 200 mm** and **3.866 s at 1000 mm**, excluding sensing, settling, priming, stripping, retries, support travel, and handling. Advertised overlap is not implemented and must not move feeds during stationary scoring/cutting.

**Correction:** generate timing from the actual state machine and allowed recipes, including both ends, clearance, correction, sensing/settling, supported discharge and payoff limits. Set a real production-rate requirement; preserve the separately stated 30 cuts/min endurance duty as an endurance load case, not an achieved rate.

### F30 — Resonance study uses the wrong tooth-pass kinematics and unsuitable closure criteria

The roller and pulley have different radii. At 500 mm/s on a Ø40 roller, a 20-tooth pulley runs at 3.979 rev/s, giving **79.58 Hz** tooth-pass, not 100 Hz. Its second/third harmonics are **159.15/238.73 Hz**. Docs/37/40 and the runbook compare modes to the wrong excitation lines. The composite rail model combines two rails' stiffness with a half-top-mass distribution without a consistently defined modal model. Harmonic energy is also not interchangeable with amplitude.

Adding damping reduces response amplitude; it does not necessarily produce the stipulated 2× frequency separation. Ordinary phone/DTI methods have no defined bandwidth adequate for all listed 245–529 Hz modes. The conditional H005 rib remains undesigned, so it cannot be a guaranteed repair instruction.

**Correction:** correct pulley-speed kinematics and modal mass/stiffness models. Evaluate resonant response and relative blade/wire displacement over the full speed sweep, including damping. Specify measurement bandwidth/sample rate, sensor mounting, limits and a drawn contingency design.

### F31 — Lifetime claims are not a complete machine reliability model

Docs/38 alternates between **20 mm diameter** and the actual **40 mm diameter** for feed revolutions/speed. Basic bearing L10 results and selected shaft fatigue screens omit machine availability, grease life, contamination, short-stroke screw wear, rolling/sliding tread behaviour, flexing cables, switches, contactors, springs, and unsupported custom-axis ratings. A named fatigue test without spectrum/sample count/acceptance is not an endurance protocol.

**Correction:** define the duty spectrum and wear/failure acceptance for the complete machine; update revolutions from actual diameters and process travel. Validate the custom load paths and consumables, specify maintenance/replacement triggers and spares, and complete representative endurance plus fault-recovery runs. Keep statistical bearing life distinct from guaranteed service life.

### F32 — Qualification is incomplete and accepts duplicated samples

The tool checks length at OD 1.2/4.5 × 200/1000 mm, at 15–30°C; it does not demonstrate **3000 mm**, strip lengths/accuracy, conductor damage, separate lead/tail/disabled combinations, all four nips, both measuring wheels, different wire constructions, wear, or the intended hotter operating condition. One force reading per side is not a force record for each nip. Reused sample identities are accepted, as reproduced.

**Correction:** reject duplicate sample identities; record machine/build/recipe/calibration/gauge provenance. Expand tests to the declared release envelope, including 3000 mm and all process modes, individual nips/wheels, relaxed length, thermal boundaries, and wear. Record strip accuracy and nick/sever quality explicitly. Use 30 samples as a minimum capability screen, with uncertainty/stability and confidence requirements appropriate to the production claim. A `physical_results` flag inferred only from absence of a `SYN-` prefix does not authenticate measurements.

### F33 — Numeric and direction contracts require completion

The positive large-jog overflow is reproduced at `controller.cpp:685`; the similar addition in `begin_jog_move()` also needs review. Use widened checked arithmetic before range decisions.

The two feed cassettes are correctly rotated 180° in CAD, but `set_direction()` and encoder count handling have no per-axis/per-wheel polarity setting. Manuals can contain driver inversion options, but the build guide does not freeze the required handed settings. The present stall check uses absolute encoder travel, which can hide wrong sign until later verification.

**Correction:** define machine-coordinate signs, freeze driver direction settings, or store validated sign multipliers. Prove stock/piece travel and encoder sign during commissioning. Add tests for every integer boundary at nonzero blade positions and for incorrect/missing feedback.

### F34 — Current validators can pass an electrically and physically wrong machine

The receiver “closure” check searches for prose such as `TTL mode`/`mid-bias` and even preserves an R1–R4 annotation; it does not require a native bias circuit. Quantity reconciliation explicitly scales the one CAD measuring module by two rather than validating a second installed module. Hard-coded engagement arithmetic omits relevant stack items, including the 3 mm frame wall in the M10 bed joint. The host simulator invents measuring counts from each feed motor's steps whenever `wire_present` is true, regardless of where any piece is located.

**Correction:** retain the useful transcription checks, then add independent physical-contract checks: Pico pin coordinates, component package footprints, receiver topology, stock containment, actual occurrences, screw stack tolerances, wire contact/phase origins, torque paths and process acceptance. Restore a complete source-hashed manufacturing release after the redesign.

### F35 — Build simplicity, sourcing and reproducibility are not achieved

Tight F101 matched fits, 1 µm cylindricity, grinding and fine-boring are demanding operations; docs/31's ordinary micrometer/0.01 mm DTI list is insufficient for all stated acceptance limits. Exact blades, couplings, brake components, cabinet hardware and dimensional supplier facts remain unconfirmed. The custom-axis US$70–115 estimate uses listing ranges and allowances, not an equivalent, inspected, delivered assembly quote. There is no complete machine cost, labour allowance, outsourcing cost, commissioning/metrology cost or spare-part total.

Engineering/sanitizer presets hard-code macOS Homebrew compiler paths. Target SDK/toolchain versions and reproducible build identities are not frozen; this audit ran host programs directly, not a target build. The old README's hardware revisions/test counts and several operational tables are stale relative to the selected custom axis and ifm parts.

**Correction:** release complete manufacturing travellers and illustrated assembly/tool-access sequences; specify adequate metrology or a qualified outside process, and include that process in cost. Consolidate exact parts/quotations and delivery to the stated Tunisia sous-douane route. Provide portable build presets and pinned target dependencies, firmware hashes and logic-analyser commissioning instructions. Mark every historical drawing/report clearly and remove its authority from active build instructions.

## 4. Independent review of every goal in the 40-row matrix

**Status definitions:** **BLOCKED** means a present design/control contradiction or known failed calculation; **DESK ONLY** means a bounded documentary/model result that is useful but does not close physical operation; **UNPROVEN** means required physical/vendor evidence is absent. A goal can have both a correct local calculation and a blocked machine-level use.

| # | Goal and unchanged acceptance | Audit verdict | Required closure |
|---:|---|---|---|
| 1 | Finished length ±(0.20 mm + 0.05% L) | **BLOCKED** | F06/F09/F10/F21; complete functional metrology and demonstrate relaxed-part errors at 200–3000 mm |
| 2 | Strip accuracy ±0.20 mm | **BLOCKED** | F07/F08/F22; correct sequence/closure and add actual strip-accuracy cells |
| 3 | Cut quality, no conductor damage, qualified incision | **BLOCKED / UNPROVEN** | Complete blade/backing definition and cut/strip qualification; counting is not inspection |
| 4 | Throughput/cycle budget | **BLOCKED** | F29/F19; supported-length, state-machine timing and a stated rate requirement |
| 5 | Cpk ≥1.33, ≥30 consecutive pieces/cell | **BLOCKED / UNPROVEN** | F32; unique actual pieces, complete process envelope, guarded metrology and stable-process evidence |
| 6 | Komax survey/benchmark without parity claim | **DESK ONLY** | Comparison is useful; update stale S1 component/feature rows, preserve the lack of measured parity |
| 7 | 0.5–2.5 mm² Cu/PVC; OD 1.2–4.5; lane clearances | **DESK ONLY / UNPROVEN** | Local clearance calculation is useful; traction, handling, actual lots and quality remain unqualified |
| 8 | 200–3000 mm; independent 3–25 mm strips; 10 mm insulated grip margin | **BLOCKED** | F06/F07/F11; minimum-part sensing and process geometry; supported 3 m output |
| 9 | 0.50 m/s feed; initial 0.05 m/s | **BLOCKED / UNPROVEN** | Command frequency fits nominally; F19/F20 mean payoff/traction do not establish the speed envelope |
| 10 | 2000 feed pulses/rev; 260 rpm ceiling; loaded radius calibration | **DESK ONLY / UNPROVEN** | Nominal operating point is 238.7 rpm/7958 Hz; qualify loaded radius, driver setting and actual motion |
| 11 | Blade 12.5 mm/s, 300 mm/s², 100 pulses/mm for 10 mm lead | **BLOCKED** | F13–F15; select/freeze screw/driver map and revalidate the real custom axis |
| 12 | Head acceptance loads 2500 N axial, 750 transverse, 375 vertical; double proof | **BLOCKED** | F14/F15/F22; do not substitute 1200/2400 N or old HIWIN capacities |
| 13 | Feed 1 Nm ×2.5; 5 Nm fault; belt ±480 N; roller ±60 N | **DESK ONLY, INCOMPLETE** | Existing bounded shaft/belt analyses useful; F24 and actual traction/jam/rotor roles must be included |
| 14 | Machined section stiffness ≥ reduced model, 191-section audit | **DESK ONLY** | Sampled nominal result retained; tolerance-continuous proof, local contacts and fabrication remain separate |
| 15 | Shaft fatigue, 10,000 h / 1.8×10⁷-cut duty | **UNPROVEN** | F24/F31; correct spectrum and validate actual shaft/manufacture under it |
| 16 | Matched j5, 3–6 µm interference, stated clearance envelope | **BLOCKED / UNPROVEN** | Conditional corner screen retained; F23/F35 require resolved temperature paths and adequate measured matching |
| 17 | Bearing/housing contact stress within allowables | **DESK ONLY** | Local stated-load study retained; assembled stiffness, actual loads and manufacturing contacts need validation |
| 18 | Shaft ≤20 K above housing | **BLOCKED** | Published central thermal bound exceeds it; redesign/model then instrument worst-duty paths |
| 19 | F109/collar survives 5 Nm plus 7 kN clamp | **DESK ONLY / UNPROVEN** | Bounded contact result useful; exact collar/key geometry, torque scatter, proof/fretting records still required |
| 20 | Adapter peak converged and ≤216.67 MPa screen | **BLOCKED** | Reported ≈313 MPa remains above the screen; custom support invalidates automatic reuse; do not substitute yield alone |
| 21 | Blade force within actual axis capability | **BLOCKED / UNPROVEN** | Actual selected screw/support/motor data and measured wire/blade force; old KK86 input-torque cap is not a custom rating |
| 22 | Blade-face gap 0.020 mm under cut | **BLOCKED** | Complete tooling/backing and assembled load-clearance verification |
| 23 | Nip force 50–75 N, target 60 | **DESK ONLY / UNPROVEN** | Measure each nip, actual sharing, damage and friction; F20/F24 |
| 24 | Each measuring wheel force 6–19 N, target 10 | **BLOCKED / UNPROVEN** | Integrate both modules and resolve CN fits; qualify both wheels individually |
| 25 | Full head/coupling fit and independent jaw sweep | **BLOCKED for selected build** | Existing envelope checks are useful historical scope; custom hardware geometry must be integrated and checked |
| 26 | Full layout/mechanism static integration | **BLOCKED** | Second module, custom internals, payoff torque path, cabinet, takeaway and guard mechanics are incomplete |
| 27 | Payoff ≤30 kg, spindle static support | **DESK ONLY / UNPROVEN** | Ø20 shaft beam result is plausible; complete retention/torque/weld load paths, weigh and load-test |
| 28 | Belt 180–200 N, target 190; dynamic margin | **DESK ONLY, CORRECTION REQUIRED** | Static tension arithmetic useful; use 79.58 Hz tooth-pass and measured resonant response |
| 29 | 40°C room; drive ambient ≤40°C | **UNPROVEN / INCOMPLETE** | Complete cabinet loss/airflow/overtemperature design, then actual 40°C mapping |
| 30 | Cabinet dissipation within cooler capacity | **BLOCKED as current budget** | Include all four drives, all supplies/coils and cabinet heat gains before declaring margin |
| 31 | K1–K3 DC make/break duty on 36/60 V branches | **UNPROVEN** | Catalogue series-pole selection useful; actual capacitive inrush, bus topology and measured interruption still required |
| 32 | Correct branch/XPS protection and coordination | **BLOCKED as claimed closure** | Fuse curves/I²t/DC interruption and PSU-limited faults need complete coordination, not nominal-current equivalence |
| 33 | Regeneration absorption and deterministic discharge | **BLOCKED** | Voltage-headroom calculation, exact components and measured branch decay/stop behaviour |
| 34 | Guard aperture safety distance | **UNPROVEN** | Assess actual opening type and nearest hazards, all paths, door/guard states and machine use; blade-only distance insufficient |
| 35 | Two-channel independent E-stop/guard safety function | **UNPROVEN** | Complete schematic/risk-required performance, fault injection, stop/energy validation |
| 36 | Guard release only after safe energy/standstill; safe power-loss direction | **BLOCKED at design level** | F25/F26; exact independent contacts, release request, timer/monitor and validated mechanics |
| 37 | Cheaper custom cut axis meeting unchanged goals | **BLOCKED** | F13–F15; manufacturable custom assembly, correct scale, actual ratings, original loads and inspected equivalent quote |
| 38 | Priced/selectable sourcing lines | **DESK ONLY, INCOMPLETE** | Dated observations remain observations; exact package/kit contents and purchase-time offers must agree |
| 39 | Remaining coupling/cooler/drive/Tunisia enquiries | **UNPROVEN** | Obtain and reconcile actual offers; complete missing items; no landed total established |
| 40 | Rust/C/C++ machine stack; independent safety; no uncommissioned start | **PARTLY MET / BLOCKED** | Language and hardwired-safety intent retained; production approval absent, target startup broken, validation incomplete |

The source matrix's “12 MET, 18 MET-WITH-PHYSICAL-REMAINDER” counts must not be interpreted as 30 operationally achieved goals. Its throughput/custom-axis/integration/software rows in particular overstate current completion. Several listed “physical remainders” contain unfinished design decisions that must close before fabrication.

### Additional goals requiring explicit rows

| Goal absent or underrepresented in the 40-row matrix | Release criterion to add |
|---|---|
| Easy, reproducible assembly | A competent builder completes a release traveller without choosing geometry, component variants or wiring; correct parts/quantities, tools, access and fixtures are supplied |
| Screenless routine operation | Safe panel-only start/stop/reset/repeat batches with retained approved recipe; distinguish partial/scrap recovery, material changes and calibration service |
| Complete automatic handling | Verified prime, independent strips, slug disposal, output clearance and supported full-length delivery |
| Factory robustness | Cold/hot start, mains/USB loss, EMI, sensor open/short/stuck faults, friction variation, spool depletion, wear and maintenance recovery |
| Resonance/structural precision | Qualified relative blade/wire displacement over the allowed speed/duty envelope, with stated sensor bandwidth |
| Whole-machine endurance/serviceability | Duty-specific endurance acceptance, preventive maintenance, spares and inspected replacement/recalibration procedures |
| Cost and insourcing | Complete equivalent delivered build cost, machining/grinding/metrology/qualification labour and consumables; exact externally supplied items identified |

## 5. Coherent correction and development order

### A. Freeze one build definition before more machining or PCB ordering

1. Retain Cu/PVC 0.5–2.5 mm², OD 1.2–4.5, L=200–3000, independent 0 or 3–25 mm strips, original length/strip accuracy, head loads and declared environment/duty.
2. Select one actual custom-axis screw lead, driver setting, support/nut/rail kit and supplier dimensions. Derive all motion scales from them.
3. Finish the custom-axis geometry, blade/backing, both measuring stations, payoff torque/retention/slack handling and full-length output support.
4. Generate a unified occurrence BOM, adequate stock list, machining drawings, electrical schematic, I/O allocation and assembly traveller. Restore source-hashed release consistency.
5. Keep unresolved choices as design holds. Do not purchase/machine geometry whose definition will be changed by those choices.

### B. Resolve deterministic hardware and software failures

1. Correct Pico socket coordinates, PTC footprints, native encoder circuit and input logic-level margins.
2. Fix quadrature startup/recovery; confirm bounded foreground latency or use suitable DMA/PIO counting to avoid transition loss. An eight-transition FIFO at approximately 20,480 transitions/s holds only **391 µs** of events.
3. Implement actual documented process sequencing, encoder-contact intervals, correct strip geometry, final residual acceptance, safe numeric arithmetic, direction polarity and qualified-recipe gating.
4. Integrate required index/clearance/diagnostic inputs and the intended calibration architecture. Allocate pins and connector terminals explicitly.
5. Define good/scrap/quarantine semantics and recovery from every phase. Test a real wire-state model instead of only independent motor-position counters.

### C. Recalculate the interacting physical envelope

Recompute traction using driven contacts; payoff from full reel to core; wire tension/relaxation; actual custom-axis/support loads; all accepted nip forces; assembled blade gap; bearing temperatures/fits; bus voltage headroom/discharge/inrush; complete cabinet losses; actual belt excitations and resonant displacement. Keep the stated requirements and expose margins/sensitivity. Select a drawn change whenever the bound fails.

### D. Build and qualify in increasing consequence

| Stage | Minimum acceptance before advancing |
|---|---|
| Drawing/BOM release | All selected components physically fit; stock contains parts; every critical dimension/fit/material/process/quantity is controlled; wiring/pin map agrees with real devices |
| Bare IF-B01 | Unpowered physical pin/footprint inspection; current-limited power; actual LOW/HIGH margins; output timing/enable defaults; correct encoders/index and open/short faults; both firmware images boot |
| De-energised mechanics | Torque/fit/force/weld records; entire insertion/tool/service access; positive payoff brake path; both wheels/guards/takeaway; hand motion without unintended contact |
| Cabinet/safety | Reviewed terminal schematic, PE/protection, capacitive switching, each-bus discharge; channel/EDM/lock/timer failures; power-loss and restart prevention; measured worst-case stop/standstill |
| Low-speed motion | Verified per-axis sign and scale; home accuracy; pulses counted on the target; independent feedback contact validity; no unintended blade/stock/piece interaction |
| Process qualification | Cut-only, lead-only, tail-only, both ends; minimum/maximum strip; 200/1000/3000 mm; wire constructions/OD extremes; relaxed length and strip accuracy; strand/insulation quality; verified clearance/quarantine |
| Thermal and dynamics | Cold/hot duty and stated room case; bearing temperatures/clearance; full/near-empty reels; accelerations/speed sweep; vibration relative to process tolerance |
| Endurance and release | Stated duty spectrum with wear, maintenance and failure criteria; repeated power/jam/recovery tests; approved recipe/build/calibration provenance and final release traveller |

For the initial 200–1000 mm/15–30°C qualification subset, label it as that subset. It cannot qualify the full 3000 mm or hotter environment by implication. A complete release must cover the intended full envelope or explicitly narrow the advertised envelope through an approved requirement change.

## 6. Regression and evidence changes needed

Keep existing tests; add tests that can disprove the relevant requirement:

- Coordinate-derived Pico physical pin/net matching and every selected component package/footprint.
- Native receiver bias/output-level corners and GPIO pull configuration; no documentation-only closure.
- Encoder joined-FIFO boot/recovery contract, observed target pulse/count timing, overflow/invalid-transition recovery.
- Wire-body contact and tip/insulated-boundary sequencing through prime, each strip combination, cut, ejection and every interrupted phase.
- No good count when correction fails, feedback is invalid, slug/part does not clear or cutting/stripping is unverified.
- Correct lead/microstep variants or one frozen variant; reject mismatched firmware/configuration.
- Stock containment, actual complete occurrence counts, nonintersecting hole patterns, full fastener stack/tolerance and payoff torque-path checks.
- Unique qualification samples, individual force stations, complete release-envelope coverage and provenance.
- Hash freshness across active source/manufacturing/evidence inputs; old simulated results remain attached to the geometry and assumptions actually solved.

## 7. Primary external checks used

These checks confirm specific manufacturer facts; they do not certify the assembled machine.

1. Raspberry Pi SDK hardware API: FIFO join and blocking TX semantics: https://www.raspberrypi.com/documentation/pico-sdk/hardware.html
2. Raspberry Pi Pico/Pico 2 physical pinout: https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf
3. Raspberry Pi RP2350 datasheet, electrical table 1436/3.3 V GPIO levels and pulls: https://pip-assets.raspberrypi.com/categories/1214-rp2350/documents/RP-008373-DS-2-rp2350-datasheet.pdf
4. Bourns MF-R datasheet, radial package and MF-R050 dimensions: https://bourns.com/data/global/pdfs/mfr.pdf
5. Siemens manufacturer product record, exact 5 V/3 A supply: https://mall.industry.siemens.com/mall/en/WW/Catalog/Product/6EP3310-6SB00-0AY0

Supplier prices in the repository were recorded in September 2026. This audit does not relabel them as current October stock, equivalent delivered quotations or confirmed shipping offers.

## 8. Reproducing the adversarial controller checks

From the reviewed checkout, compile the included controller harness with the existing production core and simulator:

```bash
g++ -std=c++20 -O1 -g -fsanitize=undefined -fno-sanitize-recover=undefined \
  -Ishared -Isoftware/machine/core -Isoftware/machine/tests \
  software/machine/core/profile.cpp software/machine/core/controller.cpp \
  software/machine/core/protocol.cpp adversarial_machine.cpp \
  -o adversarial-machine
./adversarial-machine
./adversarial-machine jog-overflow
```

The first invocation tests missing downstream contact, failed correction and strip-position overlap. The second intentionally triggers UBSan's range failure. The harness and duplicate-qualification source are reproduced below for independent review; these sources only simulate/inspect software and do not operate hardware.

### Controller harness

```cpp
#include "controller.hpp"
#include "sim_board.hpp"
#include <cstdio>
#include <climits>
using namespace starz;
using namespace starz::fw;

struct Rig {
    sim::SimBoard b;
    Controller c{b};
    Rig(){c.init();}
    void ms(unsigned n){while(n--){c.tick();b.advance(1000);}}
    bool until(State s,unsigned n){while(n--){if(c.state()==s)return true;ms(1);}return c.state()==s;}
    void home(){ms(25);c.home();until(State::Ready,21000);}
};
Job job(int length=300000,bool strips=false){Job j{};j.length_um=length;j.wire_diameter_um=2500;j.quantity=1;if(strips){j.strip_lead_um=8000;j.strip_tail_um=6000;j.strip_depth_um=600;}return j;}

int main(int argc,char**argv){
    if(argc>1){
        Rig r;r.home();r.c.jog(ctrl::axis::blade_front,true,1000,1000);r.until(State::Ready,10000);
        std::printf("blade position before large jog: %d um\n",r.c.telemetry().blade_um[0]);std::fflush(stdout);
        auto result=r.c.jog(ctrl::axis::blade_front,true,INT_MAX,1000);
        std::printf("large jog result: %d\n",static_cast<int>(result));return 0;
    }
    {
        Rig r;r.home();r.c.load_job(job(200000));r.c.start();
        for(unsigned i=0;i<30000 && r.c.state()!=State::Fault && r.c.state()!=State::Ready;++i){r.c.tick();r.b.advance(1000);r.b.encoder_[1]=0;r.b.enc_accum_[1]=0;}
        std::printf("No downstream measuring contact: state=%d faults=0x%04x pieces=%u\n",static_cast<int>(r.c.state()),r.c.faults(),r.c.counters().pieces_done);
    }
    {
        Rig r;r.home();r.b.slip_ppm[0]=r.b.slip_ppm[1]=-3000;r.c.load_job(job());r.c.start();
        bool corrected=false;std::int64_t freeze[2]{};
        for(unsigned i=0;i<30000 && r.c.state()!=State::Fault && r.c.state()!=State::Ready;++i){
            r.c.tick();
            if(r.c.phase()==Phase::FeedCorrect && !corrected){corrected=true;freeze[0]=r.b.encoder_[0];freeze[1]=r.b.encoder_[1];}
            r.b.advance(1000);
            if(corrected && (r.c.phase()==Phase::FeedCorrect || r.c.phase()==Phase::FeedVerify))for(int s=0;s<2;++s){r.b.encoder_[s]=freeze[s];r.b.enc_accum_[s]=0;}
        }
        std::printf("Corrective move produces no encoder travel: state=%d faults=0x%04x pieces=%u last_residual=%d um\n",static_cast<int>(r.c.state()),r.c.faults(),r.c.counters().pieces_done,r.c.counters().last_residual_um);
    }
    {
        Rig r;r.home();r.c.load_job(job(300000,true));r.c.start();
        unsigned previous_done=0;double left=0,right=0;bool tracing=false;
        for(unsigned i=0;i<30000 && r.c.state()!=State::Fault && r.c.state()!=State::Ready;++i){
            if(r.c.phase()==Phase::OpenAfterCut && r.b.feed_true_um[0]>200000 && !tracing){left=r.b.feed_true_um[0];right=r.b.feed_true_um[1];tracing=true;}
            r.ms(1);
            if(tracing && r.c.phase()==Phase::StripClose){std::printf("Production strip positioning: stock delta=%.3f mm piece delta=%.3f mm (stock-piece overlap=%.3f mm)\n",(r.b.feed_true_um[0]-left)/1000,(r.b.feed_true_um[1]-right)/1000,(r.b.feed_true_um[0]-left-r.b.feed_true_um[1]+right)/1000);tracing=false;break;}
            previous_done=r.c.counters().pieces_done;(void)previous_done;
        }
    }
}

```

### Duplicate qualification harness

```cpp
#include "qualification.hpp"
#include <cstdio>
int main(){
 namespace q=starz::engineering::qualification;
 std::vector<q::LengthRecord> lengths;
 for(double od:q::required_wire_od_mm)for(double l:q::required_length_mm)
  for(int i=0;i<30;++i)lengths.push_back({i%2?"same-part-A":"same-part-B",od,l,l+(i%2?0.01:-0.01),"gauge",0.02,21});
 std::vector<q::ForceRecord> forces;
 for(const char* station:{"RIGHT_NIP","LEFT_NIP","WHEEL"})for(double od:q::required_wire_od_mm)
  forces.push_back({station,od,station==std::string("WHEEL")?10.0:60.0,"force-gauge",0.5});
 auto report=q::evaluate(lengths,forces);
 std::printf("120 rows, only 2 distinct repeated sample identities: qualification all_pass=%d cells=%zu\n",report.all_pass,report.cells.size());
}

```

Observed controller output:

```text
No downstream measuring contact: state=1 faults=0x0100 pieces=0
Corrective move produces no encoder travel: state=4 faults=0x0000 pieces=1 last_residual=854 um
Production strip positioning: stock delta=7.980 mm piece delta=-5.969 mm (stock-piece overlap=13.949 mm)
blade position before large jog: 1000 um
software/machine/core/controller.cpp:685:95: runtime error: signed integer overflow: 1000 + 2147483647 cannot be represented in type 'int'
```

Observed duplicate-qualification output:

```text
120 rows, only 2 distinct repeated sample identities: qualification all_pass=1 cells=4
```

**Release conclusion:** retain the useful existing studies, correct the build-blocking design and controller contracts, then execute the staged acceptance plan. At the reviewed commit, the guides do not define a self-consistent, reliably functioning machine and the stated accuracy/performance goals are not achieved.
