#!/usr/bin/env bash
set -euo pipefail

adb_bin="local-setup/platform-tools/adb"
evidence_dir="tests/evidence/device"

usage() {
  cat <<'EOF'
Usage: collect_gemini_baseline.sh [--adb PATH] [--evidence-dir DIR]

Collect host-side, read-only ADB evidence for Gemini-S1 serial 1234.
Device shell commands are deliberately skipped until the board's supported
command transport has been verified from authoritative documentation.
EOF
}

while (($#)); do
  case "$1" in
    --adb)
      [[ $# -ge 2 ]] || { echo "ERROR: --adb needs a path" >&2; exit 64; }
      adb_bin="$2"
      shift 2
      ;;
    --evidence-dir)
      [[ $# -ge 2 ]] || { echo "ERROR: --evidence-dir needs a path" >&2; exit 64; }
      evidence_dir="$2"
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

[[ -x "$adb_bin" ]] || { echo "ERROR: ADB is not executable: $adb_bin" >&2; exit 69; }

devices="$($adb_bin devices -l)"
target_count="$(printf '%s\n' "$devices" | awk '$1 == "1234" && $2 == "device" {count++} END {print count+0}')"
serial_count="$(printf '%s\n' "$devices" | awk '$1 == "1234" {count++} END {print count+0}')"

if [[ "$target_count" != "1" || "$serial_count" != "1" ]]; then
  echo "ERROR: expected exactly one ready Gemini-S1 with serial 1234" >&2
  printf '%s\n' "$devices" >&2
  exit 1
fi

adb_version="$($adb_bin version)"
sanitized_adb_version="$(printf '%s\n' "$adb_version" | sed -E 's#/Users/[^/]+#<HOME>#g; s#/home/[^/]+#<HOME>#g')"

mkdir -p "$evidence_dir"
timestamp="$(date -u '+%Y%m%dT%H%M%SZ')"
evidence_file="$evidence_dir/gemini-baseline-$timestamp.txt"

{
  echo "Living Canvas Gemini-S1 read-only baseline"
  echo "Captured (UTC): $timestamp"
  echo "Expected target serial: 1234"
  echo
  echo '=== adb version ==='
  printf '%s\n' "$sanitized_adb_version"
  echo
  echo '=== adb devices -l ==='
  printf '%s\n' "$devices"
  echo
  echo 'Shell collection: SKIPPED'
  echo 'Reason: adb shell command transport did not produce a verified board response; no device command is assumed safe.'
} >"$evidence_file"

echo "Evidence: $evidence_file"
