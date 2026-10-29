#!/bin/bash
# Host-compat shim: macOS 27 arm64 hangs in dyld __malloc_init for ASan-instrumented
# binaries (verified via /usr/bin/sample). Strip only -fsanitize=address so the
# unmodified handoff tools run with UBSan. No demonstration logic is changed.
args=()
for a in "$@"; do
  case "$a" in
    -fsanitize=address,undefined) args+=("-fsanitize=undefined") ;;
    *) args+=("$a") ;;
  esac
done
exec /usr/bin/g++ "${args[@]}"
