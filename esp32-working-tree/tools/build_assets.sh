#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source_dir="${SOURCE_DIR:-$project_dir/media-source}"
circle_video="${CIRCLE_VIDEO:-$source_dir/Circle video.mp4}"
menu_source="${MENU_SOURCE:-$source_dir/newnew-sitting.png}"
takeout_icon="${TAKEOUT_ICON:-$source_dir/takeout.png}"
mixbox_icon="${MIXBOX_ICON:-$source_dir/mixbox.png}"
eatathome_icon="${EATATHOME_ICON:-$source_dir/eatathome.png}"
frame_count=101
frames_dir="$project_dir/build/circle_frames"
extractor="$project_dir/build/extract_video_frames"
python_bin="${PYTHON_BIN:-python3}"

mkdir -p "$project_dir/build" "$project_dir/data" "$project_dir/generated"
find "$frames_dir" -type f -name '*.png' -delete 2>/dev/null || true
mkdir -p "$frames_dir"

clang -fobjc-arc \
  -framework Foundation -framework AVFoundation -framework CoreMedia \
  -framework CoreGraphics -framework ImageIO \
  "$project_dir/tools/extract_video_frames.m" -o "$extractor"

metadata=$($extractor "$circle_video" "$frames_dir" "$frame_count")
interval_ms=$(printf '%s\n' "$metadata" | sed -E 's/.*interval_ms=([0-9]+).*/\1/')
printf '%s\n' "$metadata"

"$python_bin" "$project_dir/tools/build_assets.py" \
  --source-dir "$source_dir" \
  --menu-source "$menu_source" \
  --takeout-icon "$takeout_icon" \
  --mixbox-icon "$mixbox_icon" \
  --eatathome-icon "$eatathome_icon" \
  --frames-dir "$frames_dir" \
  --data-dir "$project_dir/data" \
  --manifest "$project_dir/generated/living_canvas_asset_manifest.h" \
  --frame-interval-ms "$interval_ms"
