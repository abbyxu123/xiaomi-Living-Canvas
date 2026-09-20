# Living Canvas ESP32-S3 Preview Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build and flash an independent portrait Living Canvas offline demo for the new ESP32-S3 board while leaving the Gemini-S1 project unchanged.

**Architecture:** A host-tested C++ state machine drives a direct Arduino_GFX renderer. Mac-side tools convert the supplied portrait media to full-screen RGB565 files stored in a 9 MB SPIFFS partition; the ESP32 streams those files into PSRAM and pushes native 368×448 frames to the CO5300 display.

**Tech Stack:** Arduino ESP32 core 3.3.11, Arduino_GFX 1.6.7, SensorLib CST820-compatible touch driver, Adafruit XCA9554, SPIFFS, Python/Pillow, macOS AVFoundation, host C++ tests.

---

### Task 1: Project guardrails and state machine

**Files:**
- Create: `.gitignore`
- Create: `README.md`
- Create: `src/living_canvas_state.h`
- Create: `src/living_canvas_state.cpp`
- Create: `tests/test_state.cpp`
- Create: `tests/run_tests.sh`

**Steps:**
1. Write failing host tests for the home/menu/takeout/mystery/home-cooking flows, selection confirmation, back navigation, timed transitions, and home-frame wraparound.
2. Run `tests/run_tests.sh` and verify it fails because the state API is absent.
3. Implement the smallest dependency-free state machine.
4. Re-run the test and verify it passes.
5. Commit the state-machine slice.

### Task 2: Media conversion pipeline

**Files:**
- Create: `tests/test_build_assets.py`
- Create: `tools/extract_video_frames.m`
- Create: `tools/build_assets.py`
- Create: `tools/build_assets.sh`
- Generate: `data/*.rgb565`
- Generate: `generated/living_canvas_asset_manifest.h`

**Steps:**
1. Write failing tests for 368×448 output, little-endian RGB565 size, deterministic names, total-size ceiling, and Living Canvas naming.
2. Run the Python test and verify the expected missing-tool failure.
3. Implement an AVFoundation frame extractor and Pillow RGB565 converter without modifying source media.
4. Convert `circle.mp4` to a bounded loop and the correct `sitting.png` to the menu screen.
5. Re-run tests and record frame count, interval, and total bytes.
6. Commit the asset pipeline and generated manifest; keep generated binary assets out of Git.

### Task 3: Native portrait board firmware

**Files:**
- Create: `living_canvas_esp32s3_preview.ino`
- Create: `board_pins.h`
- Create: `src/living_canvas_assets.h`
- Create: `src/living_canvas_assets.cpp`
- Create: `partitions.csv`
- Create: `tools/build_firmware.sh`

**Steps:**
1. Add a source-boundary test that requires 368×448 native orientation, SPIFFS mount, Living Canvas naming, and excludes network dependencies.
2. Run the test and verify it fails before the sketch exists.
3. Implement only the validated CO5300/XCA9554/CST820 initialization, PSRAM framebuffer, SPIFFS frame loading, touch edge detection, and state rendering.
4. Build with Arduino CLI using ESP32-S3, 16 MB Flash, OPI PSRAM, HW CDC, and the 3 MB app + 9 MB SPIFFS partition layout.
5. Re-run all host and source-boundary tests.
6. Commit the firmware slice.

### Task 4: SPIFFS image, safe flashing, and device verification

**Files:**
- Generate: `build/spiffs.bin`
- Generate: `build/checksums.sha256`
- Create: `tools/flash_verified_device.sh`

**Steps:**
1. Build the SPIFFS image with `mkspiffs` and verify it is below the partition size.
2. Generate SHA-256 hashes for bootloader, partition table, application, and SPIFFS images.
3. Query only `/dev/cu.usbmodem*` with esptool and stop unless the MAC equals the locally supplied `EXPECTED_ESP32_MAC`.
4. Flash the Arduino images plus SPIFFS at their partition-table offsets.
5. Read the 115200 boot log and verify display, touch, PSRAM, SPIFFS, state machine, and home animation startup messages.
6. Confirm the Gemini-S1 repository status and HEAD are unchanged.
7. Commit build/flash documentation while leaving binaries ignored.
