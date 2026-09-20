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

    def test_build_home_jpeg_is_native_portrait_and_compressed(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source = pathlib.Path(temp_dir) / "frame.png"
            output = pathlib.Path(temp_dir) / "home_000.jpg"
            Image.new("RGB", (1086, 1448), (12, 34, 56)).save(source)
            build_assets.build_home_jpeg(source, output, quality=70)
            with Image.open(output) as result:
                self.assertEqual(result.size, (368, 448))
                self.assertEqual(result.format, "JPEG")
            self.assertLess(output.stat().st_size, 368 * 448 * 2)

    def test_build_menu_states_writes_only_four_approved_frames(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = pathlib.Path(temp_dir)
            base = root / "newnew-sitting.png"
            Image.new("RGB", (1086, 1448), (40, 30, 20)).save(base)
            icons = {}
            for name, color in (
                ("takeout", (220, 170, 90, 255)),
                ("mixbox", (220, 30, 30, 255)),
                ("eatathome", (250, 245, 220, 255)),
            ):
                path = root / f"{name}.png"
                Image.new("RGBA", (256, 256), color).save(path)
                icons[name] = path
            output_dir = root / "data"
            build_assets.build_menu_states(base, icons, output_dir)
            self.assertEqual(
                sorted(path.name for path in output_dir.glob("*.rgb565")),
                [
                    "menu.rgb565",
                    "menu_eatathome.rgb565",
                    "menu_mixbox.rgb565",
                    "menu_takeout.rgb565",
                ],
            )

    def test_manifest_contains_only_living_canvas_names(self):
        manifest = build_assets.render_manifest(12, 180)
        self.assertIn("LIVING_CANVAS_HOME_FRAME_COUNT = 12", manifest)
        self.assertIn("LIVING_CANVAS_HOME_FRAME_INTERVAL_MS = 180", manifest)
        self.assertIn("LIVING_CANVAS_FRAME_BYTES", manifest)
        self.assertIn('LIVING_CANVAS_HOME_EXTENSION = ".jpg"', manifest)

    def test_video_extractor_uses_avfoundation_and_bounded_frame_count(self):
        source = (ROOT / "tools" / "extract_video_frames.m").read_text(encoding="utf-8")
        self.assertIn("AVAssetImageGenerator", source)
        self.assertIn("frameCount", source)
        self.assertIn("frameCount > 180", source)

    def test_build_script_uses_portable_approved_media_inputs(self):
        source = (ROOT / "tools" / "build_assets.sh").read_text(encoding="utf-8")
        self.assertNotIn("/Users/", source)
        self.assertIn("${SOURCE_DIR:-", source)
        self.assertIn("${CIRCLE_VIDEO:-", source)
        self.assertIn("Circle video.mp4", source)
        self.assertIn("newnew-sitting.png", source)
        self.assertIn("takeout.png", source)
        self.assertIn("mixbox.png", source)
        self.assertIn("eatathome.png", source)
        self.assertIn("${PYTHON_BIN:-python3}", source)
        self.assertIn('frame_count=101', source)


if __name__ == "__main__":
    unittest.main()
