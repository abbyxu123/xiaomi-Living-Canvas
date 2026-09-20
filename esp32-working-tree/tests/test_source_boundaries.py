import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class SourceBoundaryTest(unittest.TestCase):
    def test_sketch_is_native_portrait_offline_living_canvas(self):
        sketch = (ROOT / "living_canvas_esp32s3_preview.ino").read_text(encoding="utf-8")
        pins = (ROOT / "board_pins.h").read_text(encoding="utf-8")
        combined = sketch + pins
        self.assertIn("LCD_WIDTH 368", pins)
        self.assertIn("LCD_HEIGHT 448", pins)
        self.assertIn("SPIFFS.begin", sketch)
        self.assertIn("draw16bitRGBBitmap", sketch)
        self.assertIn("esp_jpeg_decode", sketch)
        self.assertIn("LivingCanvasState", sketch)
        self.assertIn("CST820", sketch)
        for forbidden in ("WiFi", "WebSockets"):
            self.assertNotIn(forbidden, combined)

    def test_voice_uses_v2_es8311_i2s_pins_and_real_capture(self):
        sketch = (ROOT / "living_canvas_esp32s3_preview.ino").read_text(encoding="utf-8")
        pins = (ROOT / "board_pins.h").read_text(encoding="utf-8")
        self.assertIn("I2S_MCK_IO  16", pins)
        self.assertIn("I2S_BCK_IO  9", pins)
        self.assertIn("I2S_WS_IO   45", pins)
        self.assertIn("I2S_DI_IO   10", pins)
        self.assertIn("I2SClass", sketch)
        self.assertIn("readBytes", sketch)
        self.assertIn("[REC]", sketch)

    def test_home_uses_jpeg_and_menu_uses_only_approved_selection_frames(self):
        assets_h = (ROOT / "src" / "living_canvas_assets.h").read_text(encoding="utf-8")
        assets_cpp = (ROOT / "src" / "living_canvas_assets.cpp").read_text(encoding="utf-8")
        combined = assets_h + assets_cpp
        self.assertIn(".jpg", combined)
        self.assertIn("/menu.rgb565", combined)
        self.assertIn("/menu_takeout.rgb565", combined)
        self.assertIn("/menu_mixbox.rgb565", combined)
        self.assertIn("/menu_eatathome.rgb565", combined)

    def test_partition_has_dedicated_nine_megabyte_spiffs(self):
        partitions = (ROOT / "partitions.csv").read_text(encoding="utf-8")
        self.assertIn("spiffs,   data, spiffs,   0x610000, 0x960000", partitions)

    def test_flash_script_is_locked_to_the_new_board(self):
        script = (ROOT / "tools" / "flash_verified_device.sh").read_text(encoding="utf-8")
        self.assertIn("EXPECTED_MAC", script)
        self.assertIsNone(re.search(r"(?i)(?:[0-9a-f]{2}:){5}[0-9a-f]{2}", script))
        self.assertIn("/dev/cu.usbmodem", script)
        self.assertNotIn("usbserial-", script)
        self.assertNotIn("/Users/", script)

    def test_build_script_regenerates_spiffs_and_checksums(self):
        script = (ROOT / "tools" / "build_firmware.sh").read_text(encoding="utf-8")
        self.assertIn("mkspiffs", script)
        self.assertIn("checksums.sha256", script)
        self.assertIn("shasum -a 256", script)
        self.assertIn("ARDUINO_CLI", script)
        self.assertIn("MKSPIFFS", script)
        self.assertNotIn("/Users/", script)

    def test_test_runner_does_not_contain_private_macos_paths(self):
        script = (ROOT / "tests" / "run_tests.sh").read_text(encoding="utf-8")
        self.assertIn("${PYTHON_BIN:-python3}", script)
        self.assertNotIn("/Users/", script)

    def test_public_source_has_no_embedded_device_mac(self):
        mac_pattern = re.compile(r"(?i)(?:[0-9a-f]{2}:){5}[0-9a-f]{2}")
        text_suffixes = {".cpp", ".csv", ".h", ".ino", ".md", ".py", ".sh"}
        for path in ROOT.rglob("*"):
            if path.is_file() and path.suffix in text_suffixes and "build" not in path.parts:
                source = path.read_text(encoding="utf-8")
                self.assertIsNone(mac_pattern.search(source), str(path.relative_to(ROOT)))


if __name__ == "__main__":
    unittest.main()
