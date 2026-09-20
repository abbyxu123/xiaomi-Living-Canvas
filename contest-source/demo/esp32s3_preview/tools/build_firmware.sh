#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
arduino_cli="${ARDUINO_CLI:-arduino-cli}"
mkspiffs="${MKSPIFFS_BIN:-mkspiffs}"
fqbn="esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=custom,PSRAM=opi"

mkdir -p "$project_dir/build/arduino" "$project_dir/build/firmware"

"$arduino_cli" compile \
  --fqbn "$fqbn" \
  --build-path "$project_dir/build/arduino" \
  --output-dir "$project_dir/build/firmware" \
  "$project_dir"

test -d "$project_dir/data" || {
  echo "Missing generated data directory; run tools/build_assets.sh first" >&2
  exit 2
}

"$mkspiffs" -c "$project_dir/data" -b 4096 -p 256 -s 0x960000 \
  "$project_dir/build/spiffs.bin"

(
  cd "$project_dir"
  shasum -a 256 \
    build/spiffs.bin \
    build/firmware/living_canvas_esp32s3_preview.ino.bootloader.bin \
    build/firmware/living_canvas_esp32s3_preview.ino.partitions.bin \
    build/arduino/boot_app0.bin \
    build/firmware/living_canvas_esp32s3_preview.ino.bin \
    > build/checksums.sha256
)
