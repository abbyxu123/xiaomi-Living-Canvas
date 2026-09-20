#!/usr/bin/env bash
set -euo pipefail

SCRIPT_UNDER_TEST="${SCRIPT_UNDER_TEST:-$(cd "$(dirname "$0")/.." && pwd)/collect_gemini_baseline.sh}"
TEST_ROOT="$(mktemp -d)"
trap 'rm -rf "$TEST_ROOT"' EXIT

cat >"$TEST_ROOT/adb" <<'SH'
#!/usr/bin/env bash
case "$*" in
  version)
    echo 'Android Debug Bridge version 1.0.41'
    echo 'Installed as /Users/private-name/project/adb'
    ;;
  'devices -l')
    printf 'List of devices attached\n1234 device usb:1 product:adb model:adb_board device:NuttX\n'
    ;;
  *) exit 90 ;;
esac
SH
chmod +x "$TEST_ROOT/adb"

output="$($SCRIPT_UNDER_TEST --adb "$TEST_ROOT/adb" --evidence-dir "$TEST_ROOT/evidence")"
evidence_file="$(printf '%s\n' "$output" | sed -n 's/^Evidence: //p')"

if grep -q '/Users/' "$evidence_file"; then
  echo 'FAIL: evidence must not expose a macOS home-directory path' >&2
  exit 1
fi
grep -q 'Installed as <HOME>/project/adb' "$evidence_file"

echo 'PASS: evidence privacy'
