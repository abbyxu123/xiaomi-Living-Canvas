#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source_dir="${LC_ASSET_SOURCE_DIR:-$project_dir/assets/source}"
circle_video="${LC_CIRCLE_VIDEO:-$source_dir/circle.mp4}"
menu_source="${LC_MENU_SOURCE:-$source_dir/sitting.png}"
frames_dir="$project_dir/build/circle_frames"
extractor="$project_dir/build/extract_video_frames"
python_bin="${PYTHON_BIN:-python3}"

test -f "$circle_video" || {
  echo "Missing home animation: $circle_video" >&2
  echo "Set LC_ASSET_SOURCE_DIR or LC_CIRCLE_VIDEO to the approved source files." >&2
  exit 2
}
test -f "$menu_source" || {
  echo "Missing menu artwork: $menu_source" >&2
  echo "Set LC_ASSET_SOURCE_DIR or LC_MENU_SOURCE to the approved source files." >&2
  exit 2
}

mkdir -p "$project_dir/build" "$project_dir/data" "$project_dir/generated"
find "$frames_dir" -type f -name '*.png' -delete 2>/dev/null || true
mkdir -p "$frames_dir"

clang -fobjc-arc \
  -framework Foundation -framework AVFoundation -framework CoreMedia \
  -framework CoreGraphics -framework ImageIO \
  "$project_dir/tools/extract_video_frames.m" -o "$extractor"

metadata=$($extractor "$circle_video" "$frames_dir" 18)
interval_ms=$(printf '%s\n' "$metadata" | sed -E 's/.*interval_ms=([0-9]+).*/\1/')
printf '%s\n' "$metadata"

"$python_bin" "$project_dir/tools/build_assets.py" \
  --source-dir "$source_dir" \
  --menu-source "$menu_source" \
  --frames-dir "$frames_dir" \
  --data-dir "$project_dir/data" \
  --manifest "$project_dir/generated/living_canvas_asset_manifest.h" \
  --frame-interval-ms "$interval_ms"
