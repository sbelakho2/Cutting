# Revised-tree host evidence (post Phase-C observability fix), 2026-10-29

Environment: macOS 27.0.1 arm64, Apple clang 17, UBSan-only via
`gxx-ubsan-shim.sh` (ASan startup hang on this host; see
`../asan-startup-hang-sample-macos27-arm64.txt`).

- `host-tests.log`: `tools/run_host_checks.sh` host groups (24 groups incl. the
  new `observability`, `waste`, `negative_control`) — `ok`.
- `result.json` / `result.log`: files copied by the tool from the pinned kit;
  the tool overwrites `result.json` only on success. On the REVISED tree the
  tool's narrow criterion (length_slip at the ejection gate) is not met:
  stdout recorded `state=1 faults=0x00080000 pieces_done=0 pieces_accepted=0
  left_um=9990.265 right_um=9990.265 enc0=409 enc1=409` and exited with
  `COUNTEREXAMPLE_NOT_REPRODUCED`. This is the intended semantic change: the
  revised controller faults at the handling-verification gate
  (`fault::waste_not_cleared`, 0x80000) on the legacy adapted fixture instead
  of reaching the ejection-length mismatch, and never fabricates an accepted
  count (pieces_accepted=0 in both cases).

The historical defect reproduction stays frozen at
`../stock-side-counterexample/` against pinned commit `0f28aa3`
(`counterexample_reproduced: true`). The maintained regression for the defect
class is the in-tree `negative_control` host-test group: the legacy
motor-coupled fixture cannot complete or accept a cycle under the revised
contract, while the `observability` group proves piece verification uses only
the downstream station and the `waste` group proves the stub/slug handling
chain.
