#!/usr/bin/env bash
set -euo pipefail

SCRIPT_UNDER_TEST="${SCRIPT_UNDER_TEST:-$(cd "$(dirname "$0")/.." && pwd)/collect_gemini_baseline.sh}"
TEST_ROOT="$(mktemp -d)"
trap 'rm -rf "$TEST_ROOT"' EXIT

cat >"$TEST_ROOT/adb" <<'SH'
#!/usr/bin/env bash
printf '%s\n' "$*" >>"$ADB_CALL_LOG"
case "$*" in
  version)
    echo 'Android Debug Bridge version 1.0.41'
    ;;
  'devices -l')
    printf 'List of devices attached\n1234 device usb:1 product:adb model:adb_board device:NuttX\n'
    ;;
  *)
    echo "unexpected adb call: $*" >&2
    exit 90
    ;;
esac
SH
chmod +x "$TEST_ROOT/adb"

export ADB_CALL_LOG="$TEST_ROOT/adb-calls"
output="$($SCRIPT_UNDER_TEST --adb "$TEST_ROOT/adb" --evidence-dir "$TEST_ROOT/evidence")"
evidence_file="$(printf '%s\n' "$output" | sed -n 's/^Evidence: //p')"

[[ -f "$evidence_file" ]]
grep -q '1234 device' "$evidence_file"
grep -q 'Shell collection: SKIPPED' "$evidence_file"
if grep -q '^shell' "$ADB_CALL_LOG"; then
  echo 'FAIL: collector must not call unverified adb shell commands' >&2
  exit 1
fi

echo 'PASS: collect_gemini_baseline'
