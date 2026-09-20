#!/usr/bin/env bash
set -euo pipefail

host_dir="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$host_dir/test_lc_state.dSYM/Contents"
touch "$host_dir/test_lc_state"

make -C "$host_dir" clean >/dev/null

if [[ -e "$host_dir/test_lc_state" || -e "$host_dir/test_lc_state.dSYM" ]]; then
  echo 'FAIL: make clean must remove the test binary and macOS dSYM bundle' >&2
  exit 1
fi

echo 'PASS: host-test clean target'
