# Focused shaft FEM — verification record

2026-09-12. Numerical implementation and export verification only. Mechanical authority remains **head R6 / feed R6 / measurement R1**. [Studies, parameters, local results and remaining holds](../16-local-shaft-refinement.md).

**Historical46-test source state.** The subsequent [CAD-feature49-test verification](cad-feature-shaft-refinement.md) is current. Results and source hashes below remain unchanged evidence for this earlier increment; canonical test-log paths now refer to later runs.

## Final automated tests

| Configuration | Complete suite | Elapsed | Circular-bar benchmark | Focused regression |
|---|---:|---:|---:|---:|
| Native Release | **46/46 passed** | 435.55 s | 30.30 s | 20.72 s |
| RelWithDebInfo ASan/UBSan | **46/46 passed** | 835.01 s | 80.41 s | 53.69 s |

Both suites used two CTest workers and `ASAN_OPTIONS=halt_on_error=1`, `UBSAN_OPTIONS=halt_on_error=1`. They overlapped with other engineering work, so these elapsed values are not controlled performance benchmarks. No C++/CMake source changed during either final suite or the final published runs. Both subsequent Ninja dry runs reported no work.

The native log (`build-engineering/Testing/Temporary/LastTest.log`, not kept) and sanitized log (`build-engineering-sanitize/Testing/Temporary/LastTest.log`, not kept) were independently scanned:46 passed test records each, no AddressSanitizer, UndefinedBehaviorSanitizer, runtime-error or LeakSanitizer diagnostics. These log paths are overwritten by later CTest runs; the results above refer to this recorded source state.

C++20, LLVM23.1.0, OpenCASCADE7.9.3, Eigen5.0.1, Gmsh4.15.2-git, SuiteSparse7.14.0/CHOLMOD5.3.5/METIS5.1.0. Project C++ and the Eigen adapter are instrumented; prebuilt third-party libraries are not. No numerical threshold,1800-second circular-bar timeout or150000-node cap was increased.

### Added and extended coverage

- [Refined geometry benchmark](../../engineering/tests/test_refined_mesh.cpp): original planar box and curved cylinder STEP, analytic CAD volume, independent tetrahedral determinant volume, signed positive quality, expected face counts, surface nodes and facet-centroid diagnostics.
- [Focused fail-closed regression](../../engineering/tests/feed_fem_focus_gate.cmake): exact102276-node mesh that triggered the prior false rank rejection; completes the eight-basis solve and retains all20 regional rows. `--require-converged` must return2, with completed geometry audit, correct focus/field provenance, no numerical convergence claim and no fabrication release.
- [Simultaneous six-coordinate lifting](../../engineering/tests/test_distributed_constraints.cpp): three nonzero translations and three nonzero rotations under combined force, torque and body loads; exact rigid-motion displacement difference; unchanged full stress tensors, reactions and strain energy at stabilization scales0.1,1,10. Existing seven/twelve-coordinate imposed-strain and dependent-constraint checks remain.
- [Feature-guide and phase tests](../../engineering/tests/test_feed_fem.cpp): every supported guide, exact reconstruction of the original all-root minimum, finite-coordinate rejection, load signs, torque vertices and rigorous phase bound.
- [Invalid CLI requests](../../engineering/tests/feed_fem_arguments.cmake): unknown/non-feature focus, unbound background/growth options, invalid background order, out-of-range growth/minimum size, malformed or non-descending levels and invalid phase counts.
- The circular-bar five-mesh benchmark retains its4% compliance and12% full-tensor RMS limits; no coarse substitute was accepted. All pre-existing mechanical, calibration and reference-control regressions are included in the46-test totals.

## Exact six-coordinate formulation

With $KR=0$, $Cu=g$, $B=CR$ nonsingular and $A=K+\alpha P^TP$, solve

$$B^T\lambda=R^Tf,\qquad w=A^{-1}(f-C^T\lambda),\qquad q=B^{-1}(g-Cw),\qquad u=w+Rq.$$

Rigid equilibrium makes $R^T(f-C^T\lambda)=0$. Consequently $\alpha(PR)^TPw=0$; because the selected $PR$ is nonsingular, $Pw=0$. Hence $Kw=f-C^T\lambda$, and adding $Rq$ changes no strain while satisfying $Cu=g$. This avoids the unnecessary mixed-scale12×12 rank test for exactly six constraints. It neither introduces bearing springs nor relaxes a dependence threshold. Rank-six tests at1×10⁻¹⁰, full Cholesky completion and all1×10⁻⁷ residual/resultant checks remain. For seven–twelve constraints, the existing Schur rank test at1×10⁻¹² remains.

The same mesh that previously stopped now passes natively and with sanitizers. The earlier failure is resolved, not hidden by choosing another mesh. Over-budget161344/165426-node diagnostic meshes remain rejected before solving.

## Independent artifact audits

All11 final study statuses are complete, unreleased and numerically held. All26 mesh rows,208 basis rows,520 regional rows and corresponding criteria counts were checked independently. Maximum mesh143887 nodes; minimum signed inverse-condition quality0.0196478551819; maximum CAD-node distance5.72522009334×10⁻⁷ mm. Full limits and geometric exclusions are in the [study record](../16-local-shaft-refinement.md).

**All52 VTUs passed** large-document XML parsing and checks of all six arrays: exact lengths, finite numeric syntax/values, integer connectivity in range, offsets4/8/…/4N, tetrahedron type10 and nonnegative cell stress. Independent centroid-based selected-region maxima match the CSV with **zero difference at exported precision**; the comparison limit was1×10⁻⁷ MPa. No smoothing, percentile filtering or rendered-field review was used.

The all-root compatibility run reproduces all20 regional raw/bound values with maximum absolute difference5.00002×10⁻¹⁰ MPa. The A12 96-phase audit encloses all20 regional raw peaks within their48-phase bounds, minimum reserve0.0183368773 MPa. Background-refinement comparisons for neck, roller key and roller shoulder remain below5%; they are separate sensitivity checks, not full background-error bounds.

Geometry hashes below use the literal XML bytes from file start through the closing Cells tag, **without an appended newline**:

| Pair | Matching SHA-256 |
|---|---|
| [Original all-root mesh](../../engineering/generated/feed-fem/working-4.vtu) / [new compatibility mesh](../../engineering/generated/feed-fem-focused/all-root-compatibility/working-1.vtu) | d56ff3f9540af25437d8dd7c9ff4f58fe1e8459e5d9526e28cb2e531c7b00bbc |
| [A12 finest48-phase mesh](../../engineering/generated/feed-fem-focused/A12-groove/working-3.vtu) / [96-phase mesh](../../engineering/generated/feed-fem-focused/A12-phase-audit/working-1.vtu) | a0bd502208e07354e55851226cbce55a56d8c6025e19797cb81c839496b41f01 |

The historical519732f8… hash used the same all-root prefix **plus one newline**; independently reproducing that convention gives the previous full hash519732f867026f77d0787def6657ff65a5c74627e5785a01b0a114dab9aa2e80. This difference is hash framing, not changed geometry.

The first XML-audit invocation used a zsh loop variable tied to `PATH`, preventing command lookup. The corrected run used a non-special variable and the verified system XML-validator executable. All52 files then completed both audits. No package or artifact change was needed.

## Source provenance

These fingerprints identify the source used by both46-test suites and all11 final studies. The older [44-test record](feed-shaft-fem.md) retains its own historical fingerprints; its source hashes are not presented as current.

| File | SHA-256 |
|---|---|
| [Build configuration](../../engineering/CMakeLists.txt) | 66c78407dc8f72c89ddfc4d5b42eddfd9db4c0b884f4a26ca9ac9c623db4423a |
| [Elastic API](../../engineering/include/fem.hpp) | 76f285073e141260c8dff17e688cc63f5aec52cfe5ada6dfa475ed06c28e7872 |
| [Elastic solver and mesh audit](../../engineering/src/fem.cpp) | 97482a1393da4396347fe1d166c67997b84b4c3ebdc1ad8fc80e8850f1508d95 |
| [Shaft API](../../engineering/include/feed_fem.hpp) | 3b54a0dc65907a16394319f4fc4d53105be1e61ad7d657c68c63591d50b3fbd4 |
| [Shaft load/refinement model](../../engineering/src/feed_fem.cpp) | 5717f30824750233c5c76ed85b0a5cf2463adadc860de69452f3a6e8b271144f |
| [Study driver](../../engineering/src/feed_fem_analysis.cpp) | e51af29786d8c09ded1979cbf5b466b1430e9863aa7de20e5cfb034e55db734f |
| [Constraint regression](../../engineering/tests/test_distributed_constraints.cpp) | b968849e6b8a3e71c330dd88f642565439f82b4b26f02a5e665502067074ecfe |
| [Mesh regression](../../engineering/tests/test_refined_mesh.cpp) | 83d55bf47c06038bc37c4d8ce8950212aaabd13ad2fdc98926347835c545c9fc |
| [Shaft regression](../../engineering/tests/test_feed_fem.cpp) | a416cca361a984b72f426fb25f344bddb42c4c426b9c57daa1efefe4bd438405 |
| [CLI rejection regression](../../engineering/tests/feed_fem_arguments.cmake) | 56d8e7fa044b6850f0bf8a94c037aafc67f0424e473bbf24ee240132726651e2 |
| [Focused gate regression](../../engineering/tests/feed_fem_focus_gate.cmake) | 24db7eed7fbe5ef61cf4194ca5408b3e737907dc01dce6549eefa1ae0b012f31 |
| [Unchanged shaft geometry](../../engineering/src/feed_geometry.cpp) | 618cd29f8901cca97f5fccc4f470a324369d0059806fa532f70b372bd9e534e3 |
| [Unchanged feed specification](../../shared/feed_drive_spec.hpp) | 3c1c725b93413f55609dbe4e725ec562a23a30d720dc5d5d402f6361e44bd236 |

## Not a machine release

Three feature families still fail local stress-change criteria. A focused last-step pass and a background sensitivity comparison do not qualify actual threads, tolerances, fit/preload/contact, rotating fatigue, passive/follower torque paths or the physical roller-force envelope. No supplied hardware was purchased, physically tested or certified by these computations. Coupling interior geometry remains unavailable without authentication; no account, supplier contact or bypass was used. The complete machine fabrication/assembly requirement remains open; hardware outputs remain disabled.