#!/usr/bin/env python3
"""Convert Living Canvas source images into native 368x448 RGB565 frames."""

from __future__ import annotations

import argparse
import pathlib
import struct

from PIL import Image


WIDTH = 368
HEIGHT = 448
FRAME_BYTES = WIDTH * HEIGHT * 2
# SPIFFS reserves working space for metadata and garbage collection. Keep the
# generated payload below roughly 70% of the physical partition.
SPIFFS_LIMIT = 0x690000


def cover_crop(image: Image.Image, width: int = WIDTH, height: int = HEIGHT) -> Image.Image:
    image = image.convert("RGB")
    source_ratio = image.width / image.height
    target_ratio = width / height
    if source_ratio > target_ratio:
        crop_width = round(image.height * target_ratio)
        left = (image.width - crop_width) // 2
        image = image.crop((left, 0, left + crop_width, image.height))
    else:
        crop_height = round(image.width / target_ratio)
        top = (image.height - crop_height) // 2
        image = image.crop((0, top, image.width, top + crop_height))
    return image.resize((width, height), Image.Resampling.LANCZOS)


def rgb565_bytes(image: Image.Image) -> bytes:
    pixels = []
    for red, green, blue in image.convert("RGB").get_flattened_data():
        value = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
        pixels.append(value)
    return struct.pack(f"<{len(pixels)}H", *pixels)


def build_static(source: pathlib.Path, output: pathlib.Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(source) as image:
        output.write_bytes(rgb565_bytes(cover_crop(image)))


def build_menu(source: pathlib.Path, data_dir: pathlib.Path) -> None:
    build_static(source, data_dir / "menu.rgb565")


def render_manifest(home_frame_count: int, frame_interval_ms: int) -> str:
    return f"""#pragma once

#include <cstddef>
#include <cstdint>

constexpr std::size_t LIVING_CANVAS_WIDTH = {WIDTH};
constexpr std::size_t LIVING_CANVAS_HEIGHT = {HEIGHT};
constexpr std::size_t LIVING_CANVAS_FRAME_BYTES = {FRAME_BYTES};
constexpr std::size_t LIVING_CANVAS_HOME_FRAME_COUNT = {home_frame_count};
constexpr std::uint32_t LIVING_CANVAS_HOME_FRAME_INTERVAL_MS = {frame_interval_ms};
"""


def build_all(args: argparse.Namespace) -> None:
    data_dir = args.data_dir.resolve()
    data_dir.mkdir(parents=True, exist_ok=True)
    for old_asset in data_dir.glob("*.rgb565"):
        old_asset.unlink()

    frame_paths = sorted(args.frames_dir.glob("*.png"))
    if not frame_paths:
        raise SystemExit(f"no extracted circle frames found in {args.frames_dir}")

    for index, frame_path in enumerate(frame_paths):
        build_static(frame_path, data_dir / f"home_{index:02d}.rgb565")

    build_menu(args.menu_source, data_dir)

    total_bytes = sum(path.stat().st_size for path in data_dir.glob("*.rgb565"))
    if total_bytes > SPIFFS_LIMIT:
        raise SystemExit(f"assets exceed SPIFFS partition: {total_bytes} > {SPIFFS_LIMIT}")

    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(
        render_manifest(len(frame_paths), args.frame_interval_ms), encoding="utf-8"
    )
    print(f"frames={len(frame_paths)} interval_ms={args.frame_interval_ms} bytes={total_bytes}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", type=pathlib.Path, required=True)
    parser.add_argument("--menu-source", type=pathlib.Path, required=True)
    parser.add_argument("--frames-dir", type=pathlib.Path, required=True)
    parser.add_argument("--data-dir", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    parser.add_argument("--frame-interval-ms", type=int, default=180)
    return parser.parse_args()


if __name__ == "__main__":
    build_all(parse_args())
