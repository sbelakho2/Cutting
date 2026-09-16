# STARZ Electronics · precision wire cutting and stripping

## Current work: S1-B parts-first engineering

**Development design. Not released for fabrication, energisation or production.**

**Building? Start here:** [fabrication guide](docs/31-fabrication-guide.md) —
ordered procurement, machining, assembly, energisation and first-article
steps with every check named. Evidence rules and the consolidated warnings:
[master plan](docs/24-production-and-release-plan.md).

- [Selected cutting-axis hardware and scope](engineering/README.md)
- [Manufacturer drawings, actual ratings and rejected mismatches](engineering/sources/catalogue-evidence.md)
- [Shared mechanical/control definition](shared/cutting_head_spec.hpp)
- [Selected-head 3D assembly](engineering/generated/head/head-assembly.step)
- [Part manifest](engineering/generated/head/manifest.csv) and [CAD validation scope](engineering/generated/head/status.json)
- [Dimensioned R6 adapter A3 PDF](engineering/generated/drawings/H003-R6-adapter.pdf) and [browser drawing review](engineering/generated/drawings/index.html)
- [F101 R6 shaft A3 dimension sheet](engineering/generated/drawings/F101-R6-shaft.pdf), [machining-detail sheet](engineering/generated/drawings/F101-R6-details.pdf) and [shaft drawing review](engineering/generated/drawings/shaft-review.html)
- [F101 selected tapping tool and physical-tip setup](docs/14-f101-tapping-operation.md), [fabrication-tooling BOM](bom/fabrication-tooling.csv) and [blank shaft inspection record](docs/inspection/F101-R6.csv)
- [M201 measuring-encoder plate A3 drawing](engineering/generated/drawings/M201-R1-encoder-plate.pdf), [eight-part encoder mount STEP](engineering/generated/measurement/encoder-mount.step) and [measurement assembly/calibration record](docs/13-measuring-interfaces-and-calibration.md)
- [P002 payoff spindle A3 drawing](engineering/generated/drawings/P002-R2-payoff-spindle.pdf), [P004–P007 payoff plates A3 drawing](engineering/generated/drawings/P004-P007-R2-payoff-plates.pdf) and [G101–G106 guard panels A3 drawing](engineering/generated/drawings/G101-G106-R2-guard-panels.pdf) with browser reviews (`payoff-spindle-review.html`, `payoff-plates-review.html`, `guard-panels-review.html` in the same directory)
- [Feed cartridge STEP](engineering/generated/feed/feed-cartridge.step), [four-roller layout STEP](engineering/generated/feed-cassette/feed-cassette.step), [machining and assembly record](docs/12-feed-cartridge-fabrication.md), [shaft/belt calculations](engineering/generated/feed-analysis/report.md)
- [Combined head/eight-roller placement STEP](engineering/generated/layout/head-and-feed-layout.step) and [cross-subsystem verification](engineering/generated/layout/status.json)
- [507-occurrence R3 full-machine STEP](engineering/generated/mechanism/full-machine.step) with pressure carriers, wire guides, measuring module, bed, welded frame, R2 guard selections and R2 payoff (spool, spindle, 6204 pockets, brake disc/caliper); [manifest](engineering/generated/mechanism/manifest.csv), [mechanism BOM](engineering/generated/mechanism/mechanism-bom.csv), [scope](engineering/generated/mechanism/status.json); [selected hardware](bom/selected-mechanism.csv) and [custom parts](bom/mechanism-custom-parts.csv)
- [Precision finishing cards](docs/21-precision-finishing.md): per-shaft matched j5 seat grinding and housing fine-boring cards from measured bearing bores
- [Physical qualification procedure and tool](docs/20-physical-qualification.md): ±(0.05 % + 0.20 mm) length cells, nip/wheel force windows, Cpk ≥ 1.33, fail-closed
- [Matched j5/C3 bearing fit](engineering/generated/bearing-fit/report.md) and [shaft matching limits](engineering/generated/bearing-fit/matching-limits.csv)
- [Feed R6 machined-section stiffness audit](engineering/generated/feed-sections/report.md):191 nominal B-rep sections, including the new chamfers and washer/key blends
- [Feed-shaft four-mesh 3D stress study](engineering/generated/feed-fem/report.md) and [model equations](docs/15-feed-shaft-stress-model.md); [torus-root rerun](docs/26-feed-shaft-torus-root-rerun.md); [regional submodel study](docs/28-feed-shaft-submodel-record.md)
- [Earlier focused full-shaft refinements](docs/16-local-shaft-refinement.md): six feature families,26 meshes,three background checks and52 audited fields
- [Earlier CAD-feature refinement and peak audit](docs/17-cad-feature-shaft-refinement.md)
- [Current A12 boundary-topology and stress investigation](docs/18-a12-boundary-topology.md)
- [Exterior-centroid diagnosis and torus-focused resolution](docs/19-exterior-centroid-diagnosis.md)
- [Current design basis and process geometry](docs/01-design-basis.md)
- [Current analytic-only report](engineering/generated/current-analytical/report.md) and [current numerical checks](engineering/generated/current-analytical/checks.csv); [archived R4 full FEM report](engineering/generated/analysis/report.md) and [historical sensitivity studies](engineering/generated/analysis/sensitivity.csv)
- [Adapter FEM with the preloaded elastic-foundation support](engineering/generated/adapter-r6-analysis/report.md)
- [Measuring-wheel calculation and calibration report](engineering/generated/measurement-analysis/report.md)
- [Production controller source](software/machine/): host-testable sequencing core, Pico 2/RP2350 target source and USB commissioning CLI
- [Legacy reference controls](software/firmware/firmware_controller.cpp): retained fault-injection model; does not operate hardware
- [IF-B01 placed/routed draft and manufacturing outputs](hardware/if-b01/README.md)
- [Cabinet controls and wiring design input](docs/22-controls-and-wiring.md)
- [Preliminary guard/payoff CAD accommodation](docs/25-preliminary-guard-and-payoff.md)
- [Guard/payoff R2 selection record](docs/26-guard-and-payoff-selection.md)
- [Machine assembly sequence and interface-fit register](docs/30-machine-assembly-sequence.md)
- [Physical contradiction audit](docs/28-physical-contradiction-audit.md)
- [Master production and release plan](docs/24-production-and-release-plan.md): stepwise procurement, fabrication, assembly, inspection, commissioning and first-article gates; templates remain blank until actual work is performed.
- [Auditable sourcing evidence](bom/sourcing-evidence.csv): direct public-page observations and exact-SKU comparison boundaries; no delivered-cost claim without equivalent quotations. Added 2026-09-13: LC1D12BD EUR70.40 (Automation24), SKF bearing/retention GBP prices (Simply Bearings), with FAG/KOYO quality-equivalent comparisons in [catalogue evidence](engineering/sources/catalogue-evidence.md).
- [IF-B01/RP2350 GPIO allocation](hardware/if-b01/if-b01-gpio-allocation.csv): machine-readable connector-to-firmware allocation checked against the native intent, cabinet schedule and RP2350 source.

The new cutting head uses two complete HIWIN KK8610P340A1F0 axes with real10mm lead, two34E1K-85/CL86T-V41 motor/driver kits and R+W MK2/100/50/8/14 couplings. The motors use original supplier STEP geometry, including cables and connectors, with the supplied removable shaft key removed for the keyless hub. Other purchased head components use catalogue-interface geometry.

The measuring-wheel source audit corrected the free circumference tolerance to **100±0.1 mm** and the uncalibrated1m allocation sum to **1.65 mm**, above the0.70 mm target. The new C++ phase-map fitter checks signed counts, reversals, observable data and independent validation residuals; its example data are **synthetic**, not measured calibration. The encoder tip is20 mm from its mounting face. The full-machine source includes the measuring shaft, pressure carriers and wire guides; each remains unbuilt and unqualified. The287-part combined head/feed layout is unchanged.

## Reproducibility

C++20; CMake/Ninja; OpenCASCADE7.9.3; Eigen; Gmsh; SuiteSparse/CHOLMOD with METIS. The `engineering` CMake preset builds the current head and numerical verification. The `engineering_generate` build target writes CAD/readback artifacts and engineering reports. The root `core` and `sanitize` presets build/test the host-side machine core and retained legacy reference code; `sanitize` enables AddressSanitizer and UndefinedBehaviorSanitizer. Root CTest also executes Python source-record validators for production evidence, IF-B01/RP2350 allocation and published mechanism artifacts; the C++ replacement engineering suite itself has no Python runtime dependency. When a compiler or cache option changes, configure with `cmake --fresh --preset <name>` so preset values are fully reapplied. The separate Pico SDK project under `software/machine/rp2350` builds target UF2 images. No build is physical qualification or permission to energise equipment.

The current **head R6 / feed R6 / measuring R1 / mechanism R3** source state extends the prior 492-part R1 mechanism to 507 source occurrences: eight R2-selected guard/actuator solids and seven R2 payoff solids (welded spool, Ø20 spindle, two 6204-pocket posts, base, brake disc, caliper bracket). Its mechanism test set covers the two wire diameters, STEP export/readback and four rejection cases (shifted bushing, slack carrier spring, light wheel and blocked guard wire route). Historical verification records for the earlier source states (64/64, 52/52, 54/54 native and ASan/UBSan runs, A12 topology, exterior-centroid diagnosis) are preserved in [docs/verification/](docs/verification/). The torus-root rerun ([record](docs/26-feed-shaft-torus-root-rerun.md)) and the [regional submodel study](docs/28-feed-shaft-submodel-record.md) close the feed-shaft regional gates; the adapter FEM now uses a preloaded Winkler elastic-foundation support ([report](engineering/generated/adapter-r6-analysis/report.md)). **The engineering suite is 83/83 native tests passing in 1847.54 s** at this source state; the core and sanitize presets pass 18/18 each.

Head:55 occurrences; cartridge32; four-roller layout116; combined head/eight-roller placement287; separate encoder face mount8. Head/feed moving parts retain17.5 mm X separation for all independent jawY positions. Feed report:25 checks,0 failures; nominal machined-section audit:191 sections,0 sampled stiffness failures. Bearing fit:256 corners,minimum1.28 µm residual clearance **conditional on its declared thermal envelope**,not a measured temperature prediction. Measurement report:392 synthetic held-out observations,0 residual failures; factory accuracy and the626 CN thermal screen remain open. Tests verify their declared software/model scope,not physical machine certification.

## Repository contents (cleaned 2026-09-13)

Every remaining file is a deliverable or its evidence. Build trees are not kept; regenerate with the presets below.

| Path | Content |
|---|---|
| [shared/](shared/) | Frozen C++ specifications: cutting head, feed drive, measurement, machine limits |
| [engineering/](engineering/README.md) | C++20 CAD generators, FEM/fit/calibration models, mechanism integration, finishing and qualification tools, 81 currently configured CTest regressions, vendor originals ([sources/](engineering/sources/catalogue-evidence.md)), public STEP/STL/PDF/CSV outputs and all numerical study records ([generated/](engineering/generated/)) |
| [docs/](docs/01-design-basis.md) | Design basis, Komax benchmark, fabrication/assembly/inspection/calibration records, stress-study records and [verification records](docs/verification/) |
| [bom/](bom/README.md) | Active mechanical and controls selections, custom parts and fabrication tooling; only limited mechanical subtotals are calculated and no delivered machine/cabinet cost exists |
| [software/](software/README.md) | Legacy reference controller and Qt monitor, host-testable production-controller core, Pico 2/RP2350 source and USB commissioning CLI; none is physically qualified |
| [tools/capture_vendor_documents.cpp](tools/capture_vendor_documents.cpp) | Local-only receiver used to archive supplier documents; no network client |
| [branding/](branding/README.md), [brand-board.jpg](branding/brand-board.jpg) | Brand board and approximate trace |

Superseded S1-A CAD, withdrawn BOM/cost/physics/software-architecture documents, duplicate downloads and the final CTest logs are archived outside the repository as `Cutting-superseded-2026-09-13.tar.zst` and are not authority for anything.

## Manufacturability and cost

- Custom metal parts are limited to H001/H002/H003/H004, F101–F112, M201, the guard G101–G106 and payoff P001–P007 parts; all are prismatic, turned or welded-laser parts for the available laser/brake/CNC, with dimensioned A3 sheets for H003, F101, M201, P002, P004–P007 and G101–G106. F101 bearing seats (12/15 j5 matched to measured bores, cylindricity1 µm, Ra0.4 µm) and the blade seat require outsourced grinding.
- Purchased parts are catalogue items from HIWIN, STEPPERONLINE, R+W, MISUMI, SKF, SEEGER, Kübler, Autonics, Rittal and the documented controls suppliers. Observed prices and quotation requests: [sourcing evidence](bom/sourcing-evidence.csv) and [master plan §2](docs/24-production-and-release-plan.md).
- Komax's published length repeatability is ±(0.2%+1 mm); the S1 target ±(0.20 mm+0.05%) is tighter on paper and **unmeasured**; see [the benchmark](docs/02-benchmarks-and-sources.md).

## Warnings (consolidated)

- This is development design: nothing is released for fabrication, energisation or production. All `physical_results`, `fabrication_released` and `full_machine_integrated` flags are false.
- Physical measurements that block first use — matched bearing fits, torque/preload records, weld inspection, spring-force settings, blade edge qualification, stop-time and bus-decay, risk-assessment sign-off, full-spool weighing, Cpk runs — have defined procedures and blank templates and cannot be closed on a desk.
- Simulation state: the adapter raw-peak gate is REFINE (pressure-patch edge; contact analysis with measured joint stiffness pending); the 34-exterior centroid whole-shaft audit and preload/contact/fatigue/thermal qualifications remain held; no FEM number releases a part.
- Electrical: the LC1D12BD DC switching duty answer is outstanding; the XPS fuse is DF2CN2 (2 A); cabinet energisation is prohibited until the docs/22 holds have evidence.
- Guard: the 30 mm apertures pass the desk reach calculation but require risk-assessment sign-off; full-spool mass ≤30 kg until the spindle deflection re-check.

## Brand and fabrication

[Original brand artwork](branding/brand-board.jpg); [approximate SVG trace and limitations](branding/README.md). Custom load-bearing parts are metal; the Bambu printer is for noncritical guides/covers. Availability of a CNC or laser is not evidence of grinding-level accuracy, heat-treatment capability or machine safety compliance.