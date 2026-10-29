# Full ctest battery at completion/2026-10-29 (2026-10-29)

Environment: macOS 27.0.1 arm64, Apple clang 17, CMake 4.4.4; repo-root build
(the engineering suite builds as a subdirectory; explicit OpenMP flags were
passed for AppleClang: -Xpreprocessor -fopenmp with Homebrew libomp).

Command: `ctest -j8 --timeout 300` over the full 98-test inventory.

Result: **98/98 passed** (452 s wall). One initial failure
(`production_record_integrity`) was caused by mechanical sourcing-evidence rows
being placed in the controls-only register; fixed by splitting the registers
(see release/decision-log.md), after which the test passes standalone
(`ctest -R production_record_integrity` -> Passed).

Coverage highlights: all machine firmware groups (incl. reworked cycle/f11 and
the observability/waste/negative_control additions), record-integrity Python
checks, GPIO allocation check, measurement/feed/mechanism/head CAD export and
rejection suites, numerical/elasticity benchmarks, feed FEM gates
(single/focused/CAD-feature/boundary-optimizer), bearing fit/contact-thermal
reports, collar contact, containment audit, measurement/finishing/qualification
CLI fail-closed suites, and the engineering model report gates.
