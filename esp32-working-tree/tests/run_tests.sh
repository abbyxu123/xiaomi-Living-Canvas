#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir="$project_dir/build/host-tests"
mkdir -p "$build_dir"

c++ -std=c++17 -Wall -Wextra -Werror \
  -I"$project_dir/src" \
  "$project_dir/tests/test_state.cpp" \
  "$project_dir/src/living_canvas_state.cpp" \
  -o "$build_dir/test_state"

"$build_dir/test_state"

c++ -std=c++17 -Wall -Wextra -Werror \
  -I"$project_dir/src" \
  "$project_dir/tests/test_voice.cpp" \
  "$project_dir/src/living_canvas_voice.cpp" \
  -o "$build_dir/test_voice"
"$build_dir/test_voice"

cc -std=c11 -Wall -Wextra -Werror \
  -I"$project_dir/src" \
  "$project_dir/tests/test_qrcode.c" \
  "$project_dir/src/qrcode/qrcodegen.c" \
  -o "$build_dir/test_qrcode"
"$build_dir/test_qrcode"

python_bin="${PYTHON_BIN:-python3}"
"$python_bin" "$project_dir/tests/test_build_assets.py"

if [ -f "$project_dir/tests/test_source_boundaries.py" ]; then
  "$python_bin" "$project_dir/tests/test_source_boundaries.py"
fi
