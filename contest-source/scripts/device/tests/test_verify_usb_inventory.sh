#!/usr/bin/env bash
set -euo pipefail

SCRIPT_UNDER_TEST="${SCRIPT_UNDER_TEST:-$(cd "$(dirname "$0")/.." && pwd)/verify_usb_inventory.sh}"
FIXTURES="$(mktemp -d)"
trap 'rm -rf "$FIXTURES"' EXIT

cat >"$FIXTURES/gemini-only.json" <<'JSON'
{"SPUSBDataType":[{"_items":[{"_name":"Debug Bridge","manufacturer":"NuttX","product_id":"0x4e11","serial_num":"1234","vendor_id":"0x18d1  (Google Inc.)"}]}]}
JSON

cat >"$FIXTURES/duplicate-gemini.json" <<'JSON'
{"SPUSBDataType":[{"_items":[{"_name":"Debug Bridge","manufacturer":"NuttX","product_id":"0x4e11","serial_num":"1234","vendor_id":"0x18d1"},{"_name":"Debug Bridge","manufacturer":"NuttX","product_id":"0x4e11","serial_num":"1234","vendor_id":"0x18d1"}]}]}
JSON

cat >"$FIXTURES/conflicting-gemini.json" <<'JSON'
{"SPUSBDataType":[{"_items":[{"_name":"Unknown","manufacturer":"Unknown","product_id":"0x9999","serial_num":"1234","vendor_id":"0x9999"}]}]}
JSON

cat >"$FIXTURES/all-devices.json" <<'JSON'
{"SPUSBDataType":[{"_items":[{"_name":"Debug Bridge","manufacturer":"NuttX","product_id":"0x4e11","serial_num":"1234","vendor_id":"0x18d1"},{"_name":"Radar","manufacturer":"Espressif","serial_num":"RADAR-TEST-SERIAL","vendor_id":"0x303a"},{"_name":"Camera","manufacturer":"Espressif","serial_num":"CAMERA-TEST-SERIAL","vendor_id":"0x303a"}]}]}
JSON

assert_success() {
  local name="$1"; shift
  if ! "$@" >"$FIXTURES/out" 2>"$FIXTURES/err"; then
    echo "FAIL: $name should succeed" >&2
    cat "$FIXTURES/out" "$FIXTURES/err" >&2
    exit 1
  fi
}

assert_failure() {
  local name="$1"; shift
  if "$@" >"$FIXTURES/out" 2>"$FIXTURES/err"; then
    echo "FAIL: $name should fail" >&2
    cat "$FIXTURES/out" "$FIXTURES/err" >&2
    exit 1
  fi
}

assert_success "Gemini alone is a valid safe baseline" "$SCRIPT_UNDER_TEST" --inventory-json "$FIXTURES/gemini-only.json"
grep -q 'Gemini-S1: PRESENT' "$FIXTURES/out"
grep -q 'mmWave: UNCONFIGURED' "$FIXTURES/out"
grep -q 'Camera: UNCONFIGURED' "$FIXTURES/out"

assert_failure "duplicate Gemini identity is rejected" "$SCRIPT_UNDER_TEST" --inventory-json "$FIXTURES/duplicate-gemini.json"
assert_failure "conflicting Gemini identity is rejected" "$SCRIPT_UNDER_TEST" --inventory-json "$FIXTURES/conflicting-gemini.json"
assert_failure "strict mode requires optional peripherals" "$SCRIPT_UNDER_TEST" --require-all --inventory-json "$FIXTURES/gemini-only.json"

assert_success "all configured identities pass strict mode" env \
  MMWAVE_SERIAL=RADAR-TEST-SERIAL \
  CAMERA_SERIAL=CAMERA-TEST-SERIAL \
  "$SCRIPT_UNDER_TEST" --require-all --inventory-json "$FIXTURES/all-devices.json"
grep -q 'mmWave: PRESENT' "$FIXTURES/out"
grep -q 'Camera: PRESENT' "$FIXTURES/out"

echo "PASS: verify_usb_inventory"
