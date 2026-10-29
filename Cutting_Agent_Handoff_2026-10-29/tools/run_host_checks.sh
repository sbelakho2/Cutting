#!/usr/bin/env bash
set -euo pipefail
CUTTING_REPO="${1:?Usage: run_host_checks.sh /absolute/path/to/Cutting [output-dir]}"
CUTTING_OUT="${2:-$PWD/cutting-host-evidence}"
mkdir -p "$CUTTING_OUT"
CUTTING_REPO="$(cd "$CUTTING_REPO" && pwd)"
CUTTING_OUT="$(cd "$CUTTING_OUT" && pwd)"
g++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer \
 -DSTARZ_MACHINE_SOURCE_DIR="\"$CUTTING_REPO/software/machine\"" \
 -I"$CUTTING_REPO/shared" -I"$CUTTING_REPO/software/machine/core" -I"$CUTTING_REPO/software/machine/tests" \
 "$CUTTING_REPO/software/machine/core/controller.cpp" \
 "$CUTTING_REPO/software/machine/core/profile.cpp" \
 "$CUTTING_REPO/software/machine/core/protocol.cpp" \
 "$CUTTING_REPO/software/machine/tests/test_machine.cpp" -o "$CUTTING_OUT/test_machine"
ASAN_OPTIONS=detect_leaks=0 "$CUTTING_OUT/test_machine" all | tee "$CUTTING_OUT/host-tests.log"
python3 "$(dirname "$0")/reproduce_stock_side_mismatch.py" "$CUTTING_REPO" --out "$CUTTING_OUT/stock-side-counterexample"
