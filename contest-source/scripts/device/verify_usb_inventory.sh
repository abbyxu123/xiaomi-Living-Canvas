#!/usr/bin/env bash
set -euo pipefail

require_all=0
inventory_json=""

usage() {
  cat <<'EOF'
Usage: verify_usb_inventory.sh [--require-all] [--inventory-json FILE]

Read-only USB identity check for the Living Canvas hardware set.
By default, Gemini-S1 is required and disconnected peripherals are reported.
Use --require-all only when validating the complete connected hardware set.
EOF
}

while (($#)); do
  case "$1" in
    --require-all)
      require_all=1
      shift
      ;;
    --inventory-json)
      [[ $# -ge 2 ]] || { echo "ERROR: --inventory-json needs a file" >&2; exit 64; }
      inventory_json="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "ERROR: unknown argument: $1" >&2
      usage >&2
      exit 64
      ;;
  esac
done

if [[ -n "$inventory_json" ]]; then
  [[ -r "$inventory_json" ]] || { echo "ERROR: cannot read $inventory_json" >&2; exit 66; }
  input_path="$inventory_json"
  cleanup=0
else
  input_path="$(mktemp)"
  cleanup=1
  trap 'rm -f "$input_path"' EXIT
  system_profiler -json SPUSBDataType >"$input_path"
fi

python3 - "$input_path" "$require_all" <<'PY'
import json
import re
import sys

path, require_all_text = sys.argv[1:]
require_all = require_all_text == "1"

with open(path, encoding="utf-8") as handle:
    root = json.load(handle)

records = []

def visit(value):
    if isinstance(value, dict):
        if any(key in value for key in ("serial_num", "vendor_id", "product_id")):
            records.append(value)
        for child in value.values():
            visit(child)
    elif isinstance(value, list):
        for child in value:
            visit(child)

visit(root)

def normalized_hex(value):
    match = re.search(r"0x[0-9a-fA-F]+", str(value or ""))
    return match.group(0).lower() if match else ""

def serial_matches(serial):
    return [item for item in records if str(item.get("serial_num", "")) == serial]

failed = False

gemini = serial_matches("1234")
if len(gemini) != 1:
    print(f"Gemini-S1: ERROR expected 1 device with serial 1234, found {len(gemini)}")
    failed = True
else:
    item = gemini[0]
    valid = (
        normalized_hex(item.get("vendor_id")) == "0x18d1"
        and normalized_hex(item.get("product_id")) == "0x4e11"
        and str(item.get("manufacturer", "")).lower() == "nuttx"
        and str(item.get("_name", "")).lower() == "debug bridge"
    )
    if valid:
        print("Gemini-S1: PRESENT serial=1234 vid:pid=18d1:4e11 manufacturer=NuttX product=Debug Bridge")
    else:
        print("Gemini-S1: ERROR serial 1234 has conflicting USB identity")
        failed = True

peripherals = (
    ("mmWave", "10:BD:A3:9F:6A:10"),
    ("Camera", "A4:CB:8F:D1:4F:5C"),
)

for label, serial in peripherals:
    matches = serial_matches(serial)
    if not matches:
        print(f"{label}: MISSING expected_serial={serial}")
        failed = failed or require_all
        continue
    if len(matches) != 1:
        print(f"{label}: ERROR expected at most 1 device with serial {serial}, found {len(matches)}")
        failed = True
        continue
    item = matches[0]
    vendor = normalized_hex(item.get("vendor_id"))
    manufacturer = str(item.get("manufacturer", ""))
    if vendor != "0x303a" and "espressif" not in manufacturer.lower():
        print(f"{label}: ERROR serial {serial} has conflicting USB identity")
        failed = True
        continue
    product = str(item.get("_name", "unknown"))
    product_id = normalized_hex(item.get("product_id")) or "unknown"
    print(f"{label}: PRESENT serial={serial} vid={vendor or 'unknown'} pid={product_id} product={product}")

sys.exit(1 if failed else 0)
PY
