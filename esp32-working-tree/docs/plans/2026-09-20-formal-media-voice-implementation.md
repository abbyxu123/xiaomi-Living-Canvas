# Formal Media And Voice Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the ESP32-S3 preview media with the approved Living Canvas visual system, play the 10.08-second idle loop smoothly, and capture real user speech through the onboard microphone for the existing Living Canvas gateway contract.

**Architecture:** Mac-side tools extract and JPEG-compress a 10 FPS portrait frame sequence and precompose four RGB565 menu states. The ESP32 decodes each JPEG into its existing PSRAM framebuffer, renders the menu states directly, captures ES8311 I2S PCM after a confirmed choice, and sends mono audio to the configurable local HTTP gateway.

**Tech Stack:** Arduino ESP32 core 3.3.11, Arduino_GFX 1.6.7, Espressif `esp_jpeg`, ESP_I2S, ES8311, WiFi/HTTPClient, ArduinoJson, SPIFFS, Python/Pillow, macOS AVFoundation.

---

### Task 1: New media pipeline

**Files:**
- Modify: `tools/extract_video_frames.m`
- Modify: `tools/build_assets.py`
- Modify: `tools/build_assets.sh`
- Modify: `generated/living_canvas_asset_manifest.h`
- Test: `tests/test_build_assets.py`

**Steps:**
1. Add failing tests for JPEG animation files, 10 FPS metadata, four menu-state files, approved source paths, and total SPIFFS payload.
2. Run `tests/run_tests.sh` and verify the media tests fail.
3. Allow at least 120 extracted frames and encode 101 frames from the 10.08-second video as quality-70 JPEG.
4. Build `menu.rgb565` plus three selected menu frames by fitting each transparent icon to its matching table object and adding a warm glow.
5. Run the asset tests and inspect generated file count and byte total.
6. Commit the media pipeline.

### Task 2: JPEG playback and approved menu states

**Files:**
- Modify: `src/living_canvas_assets.h`
- Modify: `src/living_canvas_assets.cpp`
- Modify: `living_canvas_esp32s3_preview.ino`
- Modify: `tests/test_source_boundaries.py`

**Steps:**
1. Add failing source tests for `esp_jpeg_decode`, `.jpg` home paths, and selected menu paths.
2. Run tests and verify the new assertions fail.
3. Read each JPEG into a bounded PSRAM input buffer, decode to RGB565, and draw from the existing framebuffer.
4. Replace outline-only selection with the precomposed selected menu frame.
5. Update touch regions for the new object positions and retain double-tap confirmation.
6. Build the firmware and run all tests.
7. Commit the playback/menu slice.

### Task 3: Real microphone capture

**Files:**
- Create: `src/es8311.c`
- Create: `src/es8311.h`
- Create: `src/es8311_reg.h`
- Create: `src/living_canvas_voice.h`
- Create: `src/living_canvas_voice.cpp`
- Modify: `board_pins.h`
- Modify: `living_canvas_esp32s3_preview.ino`
- Test: `tests/test_voice.cpp`
- Modify: `tests/run_tests.sh`

**Steps:**
1. Add host tests for stereo-to-mono conversion, peak tracking, duration bounds, and empty-recording rejection.
2. Run tests and verify the voice module is absent.
3. Add only the generic ES8311 driver and V2 audio pins: MCLK 16, BCLK 9, WS 45, DOUT 8, DIN 10, PA 46.
4. Implement a PSRAM-backed 16 kHz stereo recorder and deterministic stereo-to-mono conversion.
5. Start recording after second-click confirmation; stop on tap or at 10 seconds; back cancels.
6. Log bytes, frames, and L/R/mono peaks for physical verification.
7. Build, run tests, and commit.

### Task 4: Living Canvas gateway client

**Files:**
- Create: `living_canvas_local_config.h.example`
- Create locally/ignored: `living_canvas_local_config.h`
- Create: `src/living_canvas_gateway.h`
- Create: `src/living_canvas_gateway.cpp`
- Modify: `.gitignore`
- Modify: `living_canvas_esp32s3_preview.ino`
- Test: `tests/test_gateway_contract.cpp`

**Steps:**
1. Add failing host tests for session, voice URL, session-status parsing, and safe QR URL construction.
2. Run tests and verify the gateway module is absent.
3. Add a neutral Living Canvas configuration header and keep local credentials ignored.
4. Connect Wi-Fi, create `/v1/session`, upload mono PCM to `/v1/voice?session_id=...&rate=16000`, and poll `/v1/session/{id}`.
5. Map listening, analyzing, recommendation, error, and retry states to the existing simple renderer.
6. Preserve real gateway failures instead of substituting fake recognized text.
7. Run tests, build, and commit.

### Task 5: Device verification and safe flash

**Files:**
- Modify: `docs/FLASH_VERIFICATION.md`

**Steps:**
1. Generate the new SPIFFS image and SHA-256 checksums.
2. Flash only after the script reads the MAC supplied locally through `EXPECTED_MAC`.
3. Monitor boot logs for display, touch, PSRAM, SPIFFS, JPEG decode, audio codec, and microphone allocation.
4. Observe three animation loops and record actual frame timings.
5. Record a short physical microphone sample and verify nonzero PCM and peak values; do not claim ASR success unless the local gateway is running.
6. Re-run host tests and checksum verification.
7. Update the verification record and commit.
