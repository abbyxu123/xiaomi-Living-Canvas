import pathlib
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
        self.assertIn("LivingCanvasState", sketch)
        self.assertIn("CST820", sketch)
        for forbidden in ("WiFi", "WebSockets"):
            self.assertNotIn(forbidden, combined)

    def test_partition_has_dedicated_nine_megabyte_spiffs(self):
        partitions = (ROOT / "partitions.csv").read_text(encoding="utf-8")
        self.assertIn("spiffs,   data, spiffs,   0x610000, 0x960000", partitions)

    def test_flash_script_is_locked_to_the_new_board(self):
        script = (ROOT / "tools" / "flash_verified_device.sh").read_text(encoding="utf-8")
        self.assertIn("EXPECTED_ESP32_MAC", script)
        self.assertIn("/dev/cu.usbmodem", script)
        self.assertNotIn("usbserial-", script)

    def test_build_script_regenerates_spiffs_and_checksums(self):
        script = (ROOT / "tools" / "build_firmware.sh").read_text(encoding="utf-8")
        self.assertIn("mkspiffs", script)
        self.assertIn("checksums.sha256", script)
        self.assertIn("shasum -a 256", script)

    def test_scripts_are_portable_and_do_not_expose_local_home_paths(self):
        script_paths = sorted((ROOT / "tools").glob("*.sh")) + [
            ROOT / "tests" / "run_tests.sh"
        ]
        combined = "\n".join(path.read_text(encoding="utf-8") for path in script_paths)
        self.assertNotIn("/Users/", combined)
        self.assertIn("LC_ASSET_SOURCE_DIR", combined)
        self.assertIn("ARDUINO_CLI", combined)
        self.assertIn("ESPTOOL_BIN", combined)


if __name__ == "__main__":
    unittest.main()
