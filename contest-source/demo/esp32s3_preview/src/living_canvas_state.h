#pragma once

#include <cstddef>
#include <cstdint>

enum class LivingCanvasScreen : std::uint8_t {
  Home,
  Menu,
  TakeoutListening,
  TakeoutAnalyzing,
  TakeoutResult,
  MysteryBoxes,
  MysteryResult,
  HomeCookingListening,
  HomeCookingResult,
};

enum class LivingCanvasChoice : std::uint8_t {
  None,
  Takeout,
  Mystery,
  HomeCooking,
};

class LivingCanvasState {
 public:
  LivingCanvasScreen screen() const;
  LivingCanvasChoice selectedChoice() const;
  std::uint8_t selectedBox() const;
  void wake(std::uint32_t nowMs);
  bool choose(LivingCanvasChoice choice, std::uint32_t nowMs);
  void chooseMysteryBox(std::uint8_t box, std::uint32_t nowMs);
  void back(std::uint32_t nowMs);
  void update(std::uint32_t nowMs);
  std::size_t homeFrame(std::uint32_t elapsedMs,
                        std::size_t frameCount,
                        std::uint32_t frameIntervalMs) const;

 private:
  void enter(LivingCanvasScreen screen, std::uint32_t nowMs);

  LivingCanvasScreen screen_ = LivingCanvasScreen::Home;
  LivingCanvasChoice selectedChoice_ = LivingCanvasChoice::None;
  std::uint8_t selectedBox_ = 0;
  std::uint32_t enteredAtMs_ = 0;
};
