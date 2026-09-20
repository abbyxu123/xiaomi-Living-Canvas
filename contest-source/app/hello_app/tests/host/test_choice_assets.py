#!/usr/bin/env python3

import hashlib
import importlib.util
import os
import tempfile
from pathlib import Path

from PIL import Image


APP_DIR = Path(__file__).resolve().parents[2]
SOURCE_DIR = Path(
    os.environ.get("LC_CHOICE_ASSET_SOURCE_DIR", APP_DIR / "assets" / "source")
)
CONVERTER_PATH = APP_DIR / "tools" / "convert_choice_assets.py"

EXPECTED = {
    "sitting.png": {
        "sha256": "b662dc623695ebf1f8044e9b1d1101056a504a6820cdafea9f21cb2dbb8577f9",
        "size": (1086, 1448),
        "alpha": False,
    },
    "homedinner.png": {
        "sha256": "0e033242db21a2d8e974c6b012ace82201e5678cbad94cf47f4dc098767dfbe3",
        "size": (1254, 1254),
        "alpha": True,
    },
    "mystery_box_ui.png": {
        "sha256": "e73f0226adc3f30e289317d5315a66e42ff74ecd0e3ff9800e4453174c2204dd",
        "size": (1024, 1024),
        "alpha": True,
    },
    "takeout_bag_ui.png": {
        "sha256": "23519c2f7a565d5252a706c10a82df359e36126e93ffd1bfbb45d3309a51aff9",
        "size": (1024, 1024),
        "alpha": True,
    },
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_converter():
    spec = importlib.util.spec_from_file_location("convert_choice_assets", CONVERTER_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("could not load asset converter")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_sources() -> None:
    for name, expected in EXPECTED.items():
        path = SOURCE_DIR / name
        assert path.is_file(), f"missing approved source: {path}"
        assert digest(path) == expected["sha256"], f"unexpected digest: {name}"
        with Image.open(path) as image:
            assert image.size == expected["size"], f"unexpected size: {name}"
            has_alpha = image.mode in ("RGBA", "LA") or "transparency" in image.info
            assert has_alpha is expected["alpha"], f"unexpected alpha mode: {name}"


def test_conversion() -> None:
    converter = load_converter()
    with tempfile.TemporaryDirectory(prefix="lc-assets-") as temp_dir:
        temp = Path(temp_dir)
        first_c = temp / "first.c"
        first_h = temp / "first.h"
        second_c = temp / "second.c"
        second_h = temp / "second.h"

        first = converter.generate_files(SOURCE_DIR, first_c, first_h)
        second = converter.generate_files(SOURCE_DIR, second_c, second_h)

        assert first == second
        assert first["portrait_background_bytes"] == 240 * 320 * 2
        assert first["landscape_background_bytes"] == 320 * 240 * 2
        assert first["card_bytes"] == 64 * 64 * 2
        assert first_c.read_bytes() == second_c.read_bytes()
        assert first_h.read_bytes() == second_h.read_bytes()
        assert "lc_bg_portrait" in first_c.read_text(encoding="utf-8")
        assert "lc_card_takeout" in first_c.read_text(encoding="utf-8")


def main() -> None:
    test_sources()
    test_conversion()
    print("PASS: choice_assets")


if __name__ == "__main__":
    main()
