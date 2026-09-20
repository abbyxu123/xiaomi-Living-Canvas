#include <Adafruit_XCA9554.h>
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TouchDrv.hpp>
#include <Wire.h>
#include <esp_heap_caps.h>
#include <jpeg_decoder.h>
#include "ESP_I2S.h"

#include "HWCDC.h"
#include "board_pins.h"
#include "generated/living_canvas_asset_manifest.h"
#include "src/living_canvas_assets.h"
#include "src/living_canvas_state.h"
#include "src/living_canvas_voice.h"
#include "src/qrcode/qrcodegen.h"

HWCDC USBSerial;

Arduino_DataBus *displayBus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
Arduino_CO5300 *display = new Arduino_CO5300(
    displayBus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT, 16, 0, 0, 0);
Adafruit_XCA9554 ioExpander;
TouchDrvCST816 touch;
LivingCanvasState appState;
I2SClass i2s;

uint16_t *frameBuffer = nullptr;
uint8_t *jpegBuffer = nullptr;
uint8_t *voiceBuffer = nullptr;
bool touchReady = false;
bool audioReady = false;
bool recording = false;
bool touchWasDown = false;
bool buttonWasDown = false;
std::size_t lastHomeFrame = LIVING_CANVAS_HOME_FRAME_COUNT;
uint32_t homeStartedAt = 0;
uint32_t recordingStartedAt = 0;
std::size_t voiceBytes = 0;

constexpr uint16_t COLOR_INK = 0x2945;
constexpr uint16_t COLOR_CREAM = 0xFF3A;
constexpr uint16_t COLOR_ORANGE = 0xFBA6;
constexpr uint16_t COLOR_GREEN = 0x4D8A;
constexpr uint16_t COLOR_BLUE = 0x3339;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_BLACK = 0x0000;
constexpr std::size_t JPEG_BUFFER_BYTES = 65536;
constexpr uint32_t VOICE_SAMPLE_RATE = 16000;
constexpr uint8_t VOICE_MAX_SECONDS = 10;
constexpr std::size_t VOICE_BUFFER_BYTES =
    VOICE_SAMPLE_RATE * 2 * sizeof(int16_t) * VOICE_MAX_SECONDS;

static bool startRecording();
static void finishRecording(bool submit);

static void fatalScreen(const char *message) {
  display->fillScreen(0x4000);
  display->setTextColor(0xFFFF);
  display->setTextSize(2);
  display->setCursor(22, 190);
  display->println("Living Canvas");
  display->setCursor(22, 225);
  display->println(message);
  while (true) delay(1000);
}

static bool drawFrameFile(const char *path) {
  File file = SPIFFS.open(path, FILE_READ);
  if (!file || file.size() != LIVING_CANVAS_FRAME_BYTES) {
    USBSerial.printf("[ASSET] invalid %s size=%lu\n", path,
                     file ? static_cast<unsigned long>(file.size()) : 0UL);
    return false;
  }
  std::size_t read = file.readBytes(reinterpret_cast<char *>(frameBuffer),
                                    LIVING_CANVAS_FRAME_BYTES);
  file.close();
  if (read != LIVING_CANVAS_FRAME_BYTES) return false;
  display->draw16bitRGBBitmap(0, 0, frameBuffer, LCD_WIDTH, LCD_HEIGHT);
  return true;
}

static bool drawJpegFrame(const char *path, uint32_t *decodeMs = nullptr,
                          uint32_t *drawMs = nullptr) {
  File file = SPIFFS.open(path, FILE_READ);
  std::size_t size = file ? static_cast<std::size_t>(file.size()) : 0;
  if (!file || size == 0 || size > JPEG_BUFFER_BYTES) {
    USBSerial.printf("[JPEG] invalid %s size=%u\n", path,
                     static_cast<unsigned>(size));
    return false;
  }
  std::size_t read = file.readBytes(reinterpret_cast<char *>(jpegBuffer), size);
  file.close();
  if (read != size) return false;

  esp_jpeg_image_cfg_t cfg = {};
  cfg.indata = jpegBuffer;
  cfg.indata_size = size;
  cfg.outbuf = reinterpret_cast<uint8_t *>(frameBuffer);
  cfg.outbuf_size = LIVING_CANVAS_FRAME_BYTES;
  cfg.out_format = JPEG_IMAGE_FORMAT_RGB565;
  cfg.out_scale = JPEG_IMAGE_SCALE_0;
  cfg.flags.swap_color_bytes = 0;
  esp_jpeg_image_output_t output = {};
  uint32_t decodeStarted = millis();
  if (esp_jpeg_decode(&cfg, &output) != ESP_OK ||
      output.width != LCD_WIDTH || output.height != LCD_HEIGHT ||
      output.output_len != LIVING_CANVAS_FRAME_BYTES) {
    USBSerial.printf("[JPEG] decode failed %s %ux%u bytes=%u\n", path,
                     output.width, output.height,
                     static_cast<unsigned>(output.output_len));
    return false;
  }
  uint32_t decodedAt = millis();
  display->draw16bitRGBBitmap(0, 0, frameBuffer, LCD_WIDTH, LCD_HEIGHT);
  uint32_t drawnAt = millis();
  if (decodeMs) *decodeMs = decodedAt - decodeStarted;
  if (drawMs) *drawMs = drawnAt - decodedAt;
  return true;
}

static void drawHome(bool force) {
  std::size_t frame = appState.homeFrame(
      millis() - homeStartedAt, LIVING_CANVAS_HOME_FRAME_COUNT,
      LIVING_CANVAS_HOME_FRAME_INTERVAL_MS);
  if (!force && frame == lastHomeFrame) return;
  char path[28];
  livingCanvasHomePath(frame, path, sizeof(path));
  uint32_t decodeMs = 0;
  uint32_t drawMs = 0;
  if (drawJpegFrame(path, &decodeMs, &drawMs)) {
    lastHomeFrame = frame;
    if (frame % 10 == 0) {
      USBSerial.printf("[HOME] frame=%u decode=%ums draw=%ums\n",
                       static_cast<unsigned>(frame),
                       static_cast<unsigned>(decodeMs),
                       static_cast<unsigned>(drawMs));
    }
  }
}

static void drawBackButton() {
  display->fillRoundRect(8, 8, 80, 38, 10, COLOR_INK);
  display->setTextColor(COLOR_WHITE);
  display->setTextSize(1);
  display->setCursor(28, 22);
  display->print("< BACK");
}

static void drawTitle(const char *title, const char *subtitle) {
  display->fillScreen(COLOR_CREAM);
  display->fillRect(0, 0, LCD_WIDTH, 86, COLOR_INK);
  display->setTextColor(COLOR_WHITE);
  display->setTextSize(2);
  display->setCursor(104, 20);
  display->println(title);
  display->setTextSize(1);
  display->setCursor(104, 54);
  display->println(subtitle);
  drawBackButton();
}

static void drawMenu() {
  const char *path = LIVING_CANVAS_MENU_PATH;
  if (appState.selectedChoice() == LivingCanvasChoice::Takeout) {
    path = LIVING_CANVAS_MENU_TAKEOUT_PATH;
  } else if (appState.selectedChoice() == LivingCanvasChoice::Mystery) {
    path = LIVING_CANVAS_MENU_MIXBOX_PATH;
  } else if (appState.selectedChoice() == LivingCanvasChoice::HomeCooking) {
    path = LIVING_CANVAS_MENU_EATATHOME_PATH;
  }
  if (!drawFrameFile(path)) fatalScreen("MENU ASSET ERROR");
  drawBackButton();
}

static void drawAgentCard(int16_t y, const char *label, const char *value) {
  display->fillRoundRect(24, y, 320, 64, 12, COLOR_WHITE);
  display->drawRoundRect(24, y, 320, 64, 12, COLOR_ORANGE);
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(40, y + 12);
  display->print(label);
  display->setTextSize(1);
  display->setCursor(40, y + 42);
  display->print(value);
}

static void drawTakeoutListening() {
  drawTitle("TAKEOUT", "REAL VOICE INPUT");
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(30, 135);
  display->println(recording ? "Listening..." : "Tap to retry");
  display->fillRoundRect(28, 190, 312, 128, 16, COLOR_WHITE);
  display->setCursor(48, 220);
  display->println("Say cuisine");
  display->setCursor(48, 258);
  display->println("taste / budget");
  display->setCursor(48, 296);
  display->println("Tap when done");
}

static void drawTakeoutAnalyzing() {
  drawTitle("ANALYZING", "MULTI-AGENT DEMO");
  drawAgentCard(110, "TASTE AGENT", "light / no spicy        OK");
  drawAgentCard(190, "MEMORY AGENT", "liked rice bowls      OK");
  drawAgentCard(270, "BUDGET AGENT", "under 35 RMB          OK");
  drawAgentCard(350, "DECISION AGENT", "ranking candidates...");
}

static void drawQr(const char *text, int16_t top) {
  static uint8_t temp[qrcodegen_BUFFER_LEN_FOR_VERSION(8)];
  static uint8_t qr[qrcodegen_BUFFER_LEN_FOR_VERSION(8)];
  if (!qrcodegen_encodeText(text, temp, qr, qrcodegen_Ecc_LOW, 1, 8,
                            qrcodegen_Mask_AUTO, true)) return;
  int size = qrcodegen_getSize(qr);
  int scale = 4;
  int quiet = 4;
  int pixels = (size + quiet * 2) * scale;
  int left = (LCD_WIDTH - pixels) / 2;
  display->fillRect(left, top, pixels, pixels, COLOR_WHITE);
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (qrcodegen_getModule(qr, x, y)) {
        display->fillRect(left + (x + quiet) * scale,
                          top + (y + quiet) * scale, scale, scale, COLOR_BLACK);
      }
    }
  }
}

static void drawTakeoutResult() {
  drawTitle("TAKEOUT READY", "SCAN THE QR CODE");
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(42, 106);
  display->println("Tomato beef rice");
  display->setTextSize(1);
  display->setCursor(96, 138);
  display->println("32 RMB - 28 min");
  drawQr("https://github.com/abbyxu123/xiaomi-Living-Canvas", 168);
  display->setCursor(76, 388);
  display->println("Returns home automatically");
}

static void drawMysteryBoxes() {
  drawTitle("MYSTERY BOX", "PICK ONE OF FOUR TAKEOUTS");
  const char *labels[] = {"A", "B", "C", "D"};
  for (int i = 0; i < 4; ++i) {
    int x = 38 + (i % 2) * 158;
    int y = 120 + (i / 2) * 140;
    display->fillRoundRect(x, y, 134, 110, 14, COLOR_ORANGE);
    display->drawRoundRect(x, y, 134, 110, 14, COLOR_INK);
    display->setTextColor(COLOR_INK);
    display->setTextSize(4);
    display->setCursor(x + 54, y + 38);
    display->print(labels[i]);
  }
}

static void drawMysteryResult() {
  drawTitle("BOX OPENED", "MEMORY-SAFE RANDOM PICK");
  display->setTextColor(COLOR_INK);
  display->setTextSize(5);
  display->setCursor(142, 150);
  display->print(static_cast<char>('A' + appState.selectedBox()));
  display->setTextSize(2);
  display->setCursor(58, 245);
  display->println("Chicken rice bowl");
  display->setCursor(76, 285);
  display->println("No spicy - 29 RMB");
}

static void drawHomeCookingListening() {
  drawTitle("COOK AT HOME", "REAL FRIDGE VOICE INPUT");
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(26, 150);
  display->println(recording ? "Listening..." : "Tap to retry");
  display->setTextSize(2);
  display->setCursor(48, 218);
  display->println("Say what is in");
  display->setCursor(48, 270);
  display->println("your fridge");
  display->setCursor(48, 322);
  display->println("Tap when done");
}

static void drawHomeCookingResult() {
  drawTitle("DINNER RECIPE", "PRESET DEMO RESULT");
  display->setTextColor(COLOR_INK);
  display->setTextSize(3);
  display->setCursor(34, 135);
  display->println("Tomato & Egg");
  display->setTextSize(2);
  display->setCursor(34, 205);
  display->println("1. Scramble eggs");
  display->setCursor(34, 250);
  display->println("2. Cook tomato");
  display->setCursor(34, 295);
  display->println("3. Mix and serve");
  display->setCursor(34, 355);
  display->println("Ready in 12 min");
}

static void drawCurrentScreen() {
  USBSerial.printf("[STATE] screen=%u choice=%u\n",
                   static_cast<unsigned>(appState.screen()),
                   static_cast<unsigned>(appState.selectedChoice()));
  switch (appState.screen()) {
    case LivingCanvasScreen::Home:
      homeStartedAt = millis();
      lastHomeFrame = LIVING_CANVAS_HOME_FRAME_COUNT;
      drawHome(true);
      break;
    case LivingCanvasScreen::Menu:
      drawMenu();
      break;
    case LivingCanvasScreen::TakeoutListening:
      drawTakeoutListening();
      break;
    case LivingCanvasScreen::TakeoutAnalyzing:
      drawTakeoutAnalyzing();
      break;
    case LivingCanvasScreen::TakeoutResult:
      drawTakeoutResult();
      break;
    case LivingCanvasScreen::MysteryBoxes:
      drawMysteryBoxes();
      break;
    case LivingCanvasScreen::MysteryResult:
      drawMysteryResult();
      break;
    case LivingCanvasScreen::HomeCookingListening:
      drawHomeCookingListening();
      break;
    case LivingCanvasScreen::HomeCookingResult:
      drawHomeCookingResult();
      break;
  }
}

static LivingCanvasChoice choiceAt(int16_t x, int16_t y) {
  if (y < 255 || y > 365) return LivingCanvasChoice::None;
  if (x >= 65 && x < 155) return LivingCanvasChoice::Takeout;
  if (x >= 145 && x < 230) return LivingCanvasChoice::Mystery;
  if (x >= 215 && x <= 325) return LivingCanvasChoice::HomeCooking;
  return LivingCanvasChoice::None;
}

static void handleTap(int16_t x, int16_t y, uint32_t nowMs) {
  if (appState.screen() == LivingCanvasScreen::Home) {
    appState.wake(nowMs);
    drawCurrentScreen();
    return;
  }
  if (x < 92 && y < 68) {
    finishRecording(false);
    appState.back(nowMs);
    drawCurrentScreen();
    return;
  }
  if (appState.screen() == LivingCanvasScreen::Menu) {
    LivingCanvasChoice choice = choiceAt(x, y);
    if (choice != LivingCanvasChoice::None) {
      bool confirmed = appState.choose(choice, nowMs);
      if (confirmed && (appState.screen() == LivingCanvasScreen::TakeoutListening ||
                        appState.screen() == LivingCanvasScreen::HomeCookingListening)) {
        startRecording();
      }
      drawCurrentScreen();
    }
    return;
  }
  if (appState.screen() == LivingCanvasScreen::MysteryBoxes) {
    if (x >= 38 && x <= 330 && y >= 120 && y <= 370) {
      uint8_t column = x >= 196 ? 1 : 0;
      uint8_t row = y >= 260 ? 1 : 0;
      appState.chooseMysteryBox(row * 2 + column, nowMs);
      drawCurrentScreen();
    }
  }
  if (appState.screen() == LivingCanvasScreen::TakeoutListening ||
      appState.screen() == LivingCanvasScreen::HomeCookingListening) {
    if (recording) {
      finishRecording(true);
    } else {
      startRecording();
      drawCurrentScreen();
    }
    return;
  }
}

static bool codecWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x18);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

static bool codecRead(uint8_t reg, uint8_t *value) {
  Wire.beginTransmission(0x18);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(0x18, 1) != 1) return false;
  *value = Wire.read();
  return true;
}

static bool initAudio() {
  pinMode(PA, OUTPUT);
  digitalWrite(PA, LOW);

  const uint8_t registers[][2] = {
      {0x00, 0x1F}, {0x00, 0x00}, {0x00, 0x80}, {0x01, 0x3F},
      {0x02, 0x00}, {0x03, 0x10}, {0x04, 0x10}, {0x05, 0x00},
      {0x06, 0x03}, {0x07, 0x00}, {0x08, 0xFF}, {0x09, 0x0C},
      {0x0A, 0x0C}, {0x0D, 0x01}, {0x0E, 0x02}, {0x12, 0x00},
      {0x13, 0x10}, {0x1C, 0x6A}, {0x37, 0x08}, {0x17, 0xC8},
      {0x14, 0x1A}, {0x16, 0x03},
  };
  for (const auto &entry : registers) {
    if (!codecWrite(entry[0], entry[1])) {
      USBSerial.printf("[MIC] ES8311 write failed reg=0x%02x\n", entry[0]);
      return false;
    }
  }
  uint8_t chipId1 = 0;
  uint8_t chipId2 = 0;
  codecRead(0xFD, &chipId1);
  codecRead(0xFE, &chipId2);
  USBSerial.printf("[MIC] ES8311 id=%02x:%02x\n", chipId1, chipId2);

  i2s.setPins(I2S_BCK_IO, I2S_WS_IO, I2S_DO_IO, I2S_DI_IO, I2S_MCK_IO);
  if (!i2s.begin(I2S_MODE_STD, VOICE_SAMPLE_RATE,
                 I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO,
                 I2S_STD_SLOT_BOTH)) {
    USBSerial.println("[MIC] I2S init failed");
    return false;
  }
  USBSerial.println("[MIC] real capture ready 16000Hz stereo");
  return true;
}

static bool startRecording() {
  if (!audioReady || voiceBuffer == nullptr) {
    USBSerial.println("[REC] unavailable");
    return false;
  }
  voiceBytes = 0;
  recordingStartedAt = millis();
  recording = true;
  USBSerial.println("[REC] start");
  return true;
}

static void captureRecording() {
  if (!recording) return;
  const std::size_t remaining = VOICE_BUFFER_BYTES - voiceBytes;
  if (remaining == 0 || millis() - recordingStartedAt >= VOICE_MAX_SECONDS * 1000UL) {
    finishRecording(true);
    return;
  }
  const std::size_t chunk = remaining < 2048 ? remaining : 2048;
  voiceBytes += i2s.readBytes(reinterpret_cast<char *>(voiceBuffer) + voiceBytes,
                              chunk);
}

static void finishRecording(bool submit) {
  if (!recording) return;
  recording = false;
  USBSerial.printf("[REC] stop stereo_bytes=%u submit=%u\n",
                   static_cast<unsigned>(voiceBytes), submit ? 1U : 0U);
  if (!submit) return;

  LivingCanvasVoiceMetrics metrics{};
  const std::size_t monoBytes = livingCanvasStereoToMono(
      reinterpret_cast<int16_t *>(voiceBuffer), voiceBytes, &metrics);
  USBSerial.printf("[REC] mono_bytes=%u frames=%u peak=%u/%u/%u\n",
                   static_cast<unsigned>(monoBytes),
                   static_cast<unsigned>(metrics.frames),
                   static_cast<unsigned>(metrics.leftPeak),
                   static_cast<unsigned>(metrics.rightPeak),
                   static_cast<unsigned>(metrics.monoPeak));
  if (livingCanvasVoiceCanSubmit(monoBytes, metrics.monoPeak)) {
    appState.submitVoice(millis());
  }
  drawCurrentScreen();
}

static bool initBoardPower() {
  Wire.begin(IIC_SDA, IIC_SCL);
  if (!ioExpander.begin(0x20)) return false;
  const uint8_t enabledPins[] = {0, 1, 2, 6};
  for (uint8_t pin : enabledPins) ioExpander.pinMode(pin, OUTPUT);
  for (uint8_t pin : enabledPins) ioExpander.digitalWrite(pin, LOW);
  delay(20);
  for (uint8_t pin : enabledPins) ioExpander.digitalWrite(pin, HIGH);
  return true;
}

void setup() {
  USBSerial.begin(115200);
  USBSerial.setTxTimeoutMs(0);
  delay(200);
  USBSerial.println("[BOOT] Living Canvas ESP32-S3 preview");

  pinMode(0, INPUT_PULLUP);
  if (!initBoardPower()) {
    USBSerial.println("[BOOT] fatal: display power IO unavailable");
    while (true) delay(1000);
  }
  USBSerial.println("[BOOT] display power ok");

  display->begin();
  display->setBrightness(205);
  display->fillScreen(0x20C4);
  display->setTextColor(0xFFFF);
  display->setTextSize(2);
  display->setCursor(55, 200);
  display->println("Living Canvas");

  touchReady = touch.begin(Wire, CST816_SLAVE_ADDRESS, IIC_SDA, IIC_SCL);
  if (touchReady) {
    touch.setMaxCoordinates(LCD_WIDTH, LCD_HEIGHT);
    USBSerial.println("[BOOT] CST820 touch ok");
  } else {
    USBSerial.println("[BOOT] CST820 touch unavailable; BOOT button enabled");
  }

  frameBuffer = static_cast<uint16_t *>(
      heap_caps_malloc(LIVING_CANVAS_FRAME_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  jpegBuffer = static_cast<uint8_t *>(
      heap_caps_malloc(JPEG_BUFFER_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  voiceBuffer = static_cast<uint8_t *>(
      heap_caps_malloc(VOICE_BUFFER_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (frameBuffer == nullptr || jpegBuffer == nullptr || voiceBuffer == nullptr) {
    fatalScreen("PSRAM ERROR");
  }
  USBSerial.printf("[BOOT] PSRAM frame buffer=%p bytes=%u\n", frameBuffer,
                   static_cast<unsigned>(LIVING_CANVAS_FRAME_BYTES));

  if (!SPIFFS.begin(false)) fatalScreen("SPIFFS ERROR");
  USBSerial.printf("[BOOT] SPIFFS ok used=%lu total=%lu\n",
                   static_cast<unsigned long>(SPIFFS.usedBytes()),
                   static_cast<unsigned long>(SPIFFS.totalBytes()));

  audioReady = initAudio();

  homeStartedAt = millis();
  drawHome(true);
  USBSerial.println("[BOOT] home animation started");
}

void loop() {
  uint32_t nowMs = millis();
  LivingCanvasScreen beforeUpdate = appState.screen();
  appState.update(nowMs);
  if (appState.screen() != beforeUpdate) drawCurrentScreen();

  if (appState.screen() == LivingCanvasScreen::Home) drawHome(false);
  captureRecording();

  bool touchDown = false;
  int16_t touchX = 0;
  int16_t touchY = 0;
  if (touchReady) {
    int16_t x[1];
    int16_t y[1];
    touchDown = touch.getPoint(x, y, 1) > 0;
    if (touchDown) {
      touchX = x[0];
      touchY = y[0];
    }
  }
  bool buttonDown = digitalRead(0) == LOW;

  if (touchDown && !touchWasDown) {
    USBSerial.printf("[TOUCH] x=%d y=%d\n", touchX, touchY);
    handleTap(touchX, touchY, nowMs);
  }
  if (buttonDown && !buttonWasDown) {
    if (appState.screen() == LivingCanvasScreen::Home) {
      handleTap(180, 220, nowMs);
    } else if (appState.screen() == LivingCanvasScreen::Menu) {
      handleTap(110, 280, nowMs);
    } else {
      finishRecording(false);
      appState.back(nowMs);
      drawCurrentScreen();
    }
  }
  touchWasDown = touchDown;
  buttonWasDown = buttonDown;
  delay(12);
}
