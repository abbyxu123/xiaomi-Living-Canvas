#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
esptool="${ESPTOOL_BIN:-esptool}"
expected_mac="${EXPECTED_ESP32_MAC:-}"

test -n "$expected_mac" || {
  echo "Set EXPECTED_ESP32_MAC to the target board MAC before flashing." >&2
  exit 2
}

set -- /dev/cu.usbmodem*
if [ "$#" -ne 1 ] || [ ! -e "$1" ]; then
  echo "Expected exactly one /dev/cu.usbmodem device; found $#" >&2
  exit 2
fi
port="$1"

for required in \
  "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.bootloader.bin" \
  "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.partitions.bin" \
  "$project_dir/build/arduino/boot_app0.bin" \
  "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.bin" \
  "$project_dir/build/spiffs.bin"; do
  test -f "$required" || { echo "Missing build artifact: $required" >&2; exit 2; }
done

mac_output=$("$esptool" --chip esp32s3 --port "$port" --baud 115200 read-mac)
printf '%s\n' "$mac_output"
printf '%s\n' "$mac_output" | grep -Fq "MAC:                $expected_mac" || {
  echo "Refusing to flash: target MAC is not $expected_mac" >&2
  exit 3
}

"$esptool" --chip esp32s3 --port "$port" --baud 115200 write-flash \
  --flash-mode dio --flash-freq 80m --flash-size 16MB \
  0x0 "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.bootloader.bin" \
  0x8000 "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.partitions.bin" \
  0xe000 "$project_dir/build/arduino/boot_app0.bin" \
  0x10000 "$project_dir/build/firmware/living_canvas_esp32s3_preview.ino.bin" \
  0x610000 "$project_dir/build/spiffs.bin"
