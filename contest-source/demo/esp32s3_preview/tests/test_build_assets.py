import importlib.util
import pathlib
import tempfile
import unittest

from PIL import Image


ROOT = pathlib.Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "build_assets.py"
SPEC = importlib.util.spec_from_file_location("build_assets", MODULE_PATH)
build_assets = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(build_assets)


class BuildAssetsTest(unittest.TestCase):
    def test_cover_crop_is_native_portrait_size(self):
        source = Image.new("RGB", (1086, 1448), (200, 100, 50))
        result = build_assets.cover_crop(source, 368, 448)
        self.assertEqual(result.size, (368, 448))

    def test_rgb565_is_little_endian_and_exact_size(self):
        image = Image.new("RGB", (2, 1))
        image.putdata([(255, 0, 0), (0, 255, 0)])
        self.assertEqual(build_assets.rgb565_bytes(image), b"\x00\xf8\xe0\x07")

    def test_build_static_writes_full_screen_frame(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source = pathlib.Path(temp_dir) / "source.png"
            output = pathlib.Path(temp_dir) / "screen.rgb565"
            Image.new("RGB", (1086, 1448), (1, 2, 3)).save(source)
            build_assets.build_static(source, output)
            self.assertEqual(output.stat().st_size, 368 * 448 * 2)

    def test_build_menu_writes_only_the_correct_sitting_frame(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source = pathlib.Path(temp_dir) / "sitting.png"
            output_dir = pathlib.Path(temp_dir) / "data"
            Image.new("RGB", (1086, 1448), (8, 9, 10)).save(source)
            build_assets.build_menu(source, output_dir)
            self.assertEqual(
                sorted(path.name for path in output_dir.glob("*.rgb565")),
                ["menu.rgb565"],
            )

    def test_manifest_contains_only_living_canvas_names(self):
        manifest = build_assets.render_manifest(12, 180)
        self.assertIn("LIVING_CANVAS_HOME_FRAME_COUNT = 12", manifest)
        self.assertIn("LIVING_CANVAS_HOME_FRAME_INTERVAL_MS = 180", manifest)
        self.assertIn("LIVING_CANVAS_FRAME_BYTES", manifest)

    def test_video_extractor_uses_avfoundation_and_bounded_frame_count(self):
        source = (ROOT / "tools" / "extract_video_frames.m").read_text(encoding="utf-8")
        self.assertIn("AVAssetImageGenerator", source)
        self.assertIn("frameCount", source)


if __name__ == "__main__":
    unittest.main()
