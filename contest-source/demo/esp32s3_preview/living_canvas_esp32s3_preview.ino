#include <Adafruit_XCA9554.h>
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TouchDrv.hpp>
#include <Wire.h>
#include <esp_heap_caps.h>

#include "HWCDC.h"
#include "board_pins.h"
#include "generated/living_canvas_asset_manifest.h"
#include "src/living_canvas_assets.h"
#include "src/living_canvas_state.h"
#include "src/qrcode/qrcodegen.h"

HWCDC USBSerial;

Arduino_DataBus *displayBus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
Arduino_CO5300 *display = new Arduino_CO5300(
    displayBus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT, 16, 0, 0, 0);
Adafruit_XCA9554 ioExpander;
TouchDrvCST816 touch;
LivingCanvasState appState;

uint16_t *frameBuffer = nullptr;
bool touchReady = false;
bool touchWasDown = false;
bool buttonWasDown = false;
std::size_t lastHomeFrame = LIVING_CANVAS_HOME_FRAME_COUNT;
uint32_t homeStartedAt = 0;

constexpr uint16_t COLOR_INK = 0x2945;
constexpr uint16_t COLOR_CREAM = 0xFF3A;
constexpr uint16_t COLOR_ORANGE = 0xFBA6;
constexpr uint16_t COLOR_GREEN = 0x4D8A;
constexpr uint16_t COLOR_BLUE = 0x3339;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_BLACK = 0x0000;

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

static void drawHome(bool force) {
  std::size_t frame = appState.homeFrame(
      millis() - homeStartedAt, LIVING_CANVAS_HOME_FRAME_COUNT,
      LIVING_CANVAS_HOME_FRAME_INTERVAL_MS);
  if (!force && frame == lastHomeFrame) return;
  char path[28];
  livingCanvasHomePath(frame, path, sizeof(path));
  if (drawFrameFile(path)) {
    lastHomeFrame = frame;
    USBSerial.printf("[HOME] frame=%u\n", static_cast<unsigned>(frame));
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

static void drawChoiceHighlight(LivingCanvasChoice choice) {
  if (choice == LivingCanvasChoice::None) return;
  int16_t x = 88;
  int16_t y = 232;
  int16_t w = 72;
  int16_t h = 98;
  if (choice == LivingCanvasChoice::Mystery) {
    x = 154; y = 248; w = 72; h = 90;
  } else if (choice == LivingCanvasChoice::HomeCooking) {
    x = 218; y = 252; w = 78; h = 88;
  }
  for (int inset = 0; inset < 4; ++inset) {
    display->drawRoundRect(x - inset, y - inset, w + inset * 2, h + inset * 2,
                           10, COLOR_ORANGE);
  }
}

static void drawMenu() {
  if (!drawFrameFile(LIVING_CANVAS_MENU_PATH)) fatalScreen("MENU ASSET ERROR");
  drawBackButton();
  drawChoiceHighlight(appState.selectedChoice());
  display->fillRoundRect(36, 386, 296, 44, 12, COLOR_INK);
  display->setTextColor(COLOR_WHITE);
  display->setTextSize(1);
  display->setCursor(58, 401);
  display->print("TAP TO HIGHLIGHT - TAP AGAIN TO OPEN");
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
  drawTitle("TAKEOUT", "VOICE INPUT DEMO");
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(30, 135);
  display->println("Listening...");
  display->fillRoundRect(28, 190, 312, 128, 16, COLOR_WHITE);
  display->setCursor(48, 220);
  display->println("Light taste");
  display->setCursor(48, 258);
  display->println("No spicy");
  display->setCursor(48, 296);
  display->println("Under 35 RMB");
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
  drawTitle("COOK AT HOME", "FRIDGE VOICE INPUT DEMO");
  display->setTextColor(COLOR_INK);
  display->setTextSize(2);
  display->setCursor(26, 150);
  display->println("In the fridge:");
  display->setTextSize(3);
  display->setCursor(48, 218);
  display->println("Tomato");
  display->setCursor(48, 270);
  display->println("Eggs");
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
  if (y < 215 || y > 355) return LivingCanvasChoice::None;
  if (x >= 70 && x < 150) return LivingCanvasChoice::Takeout;
  if (x >= 150 && x < 228) return LivingCanvasChoice::Mystery;
  if (x >= 228 && x <= 320) return LivingCanvasChoice::HomeCooking;
  return LivingCanvasChoice::None;
}

static void handleTap(int16_t x, int16_t y, uint32_t nowMs) {
  if (appState.screen() == LivingCanvasScreen::Home) {
    appState.wake(nowMs);
    drawCurrentScreen();
    return;
  }
  if (x < 92 && y < 68) {
    appState.back(nowMs);
    drawCurrentScreen();
    return;
  }
  if (appState.screen() == LivingCanvasScreen::Menu) {
    LivingCanvasChoice choice = choiceAt(x, y);
    if (choice != LivingCanvasChoice::None) {
      appState.choose(choice, nowMs);
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
  if (frameBuffer == nullptr) fatalScreen("PSRAM ERROR");
  USBSerial.printf("[BOOT] PSRAM frame buffer=%p bytes=%u\n", frameBuffer,
                   static_cast<unsigned>(LIVING_CANVAS_FRAME_BYTES));

  if (!SPIFFS.begin(false)) fatalScreen("SPIFFS ERROR");
  USBSerial.printf("[BOOT] SPIFFS ok used=%lu total=%lu\n",
                   static_cast<unsigned long>(SPIFFS.usedBytes()),
                   static_cast<unsigned long>(SPIFFS.totalBytes()));

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
      appState.back(nowMs);
      drawCurrentScreen();
    }
  }
  touchWasDown = touchDown;
  buttonWasDown = buttonDown;
  delay(12);
}
