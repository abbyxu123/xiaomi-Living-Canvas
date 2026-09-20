#include "living_canvas_state.h"

LivingCanvasScreen LivingCanvasState::screen() const { return screen_; }

LivingCanvasChoice LivingCanvasState::selectedChoice() const {
  return selectedChoice_;
}

std::uint8_t LivingCanvasState::selectedBox() const { return selectedBox_; }

void LivingCanvasState::enter(LivingCanvasScreen screen, std::uint32_t nowMs) {
  screen_ = screen;
  enteredAtMs_ = nowMs;
}

void LivingCanvasState::wake(std::uint32_t nowMs) {
  if (screen_ != LivingCanvasScreen::Home) return;
  selectedChoice_ = LivingCanvasChoice::None;
  enter(LivingCanvasScreen::Menu, nowMs);
}

bool LivingCanvasState::choose(LivingCanvasChoice choice, std::uint32_t nowMs) {
  if (screen_ != LivingCanvasScreen::Menu || choice == LivingCanvasChoice::None) {
    return false;
  }
  if (selectedChoice_ != choice) {
    selectedChoice_ = choice;
    return false;
  }

  selectedChoice_ = LivingCanvasChoice::None;
  switch (choice) {
    case LivingCanvasChoice::Takeout:
      enter(LivingCanvasScreen::TakeoutListening, nowMs);
      break;
    case LivingCanvasChoice::Mystery:
      enter(LivingCanvasScreen::MysteryBoxes, nowMs);
      break;
    case LivingCanvasChoice::HomeCooking:
      enter(LivingCanvasScreen::HomeCookingListening, nowMs);
      break;
    case LivingCanvasChoice::None:
      return false;
  }
  return true;
}

void LivingCanvasState::chooseMysteryBox(std::uint8_t box, std::uint32_t nowMs) {
  if (screen_ != LivingCanvasScreen::MysteryBoxes || box > 3) return;
  selectedBox_ = box;
  enter(LivingCanvasScreen::MysteryResult, nowMs);
}

void LivingCanvasState::back(std::uint32_t nowMs) {
  selectedChoice_ = LivingCanvasChoice::None;
  if (screen_ == LivingCanvasScreen::Menu) {
    enter(LivingCanvasScreen::Home, nowMs);
  } else if (screen_ != LivingCanvasScreen::Home) {
    enter(LivingCanvasScreen::Menu, nowMs);
  }
}

void LivingCanvasState::update(std::uint32_t nowMs) {
  const std::uint32_t elapsed = nowMs - enteredAtMs_;
  switch (screen_) {
    case LivingCanvasScreen::TakeoutListening:
      if (elapsed >= 2000) enter(LivingCanvasScreen::TakeoutAnalyzing, nowMs);
      break;
    case LivingCanvasScreen::TakeoutAnalyzing:
      if (elapsed >= 3000) enter(LivingCanvasScreen::TakeoutResult, nowMs);
      break;
    case LivingCanvasScreen::HomeCookingListening:
      if (elapsed >= 2000) enter(LivingCanvasScreen::HomeCookingResult, nowMs);
      break;
    case LivingCanvasScreen::TakeoutResult:
    case LivingCanvasScreen::MysteryResult:
    case LivingCanvasScreen::HomeCookingResult:
      if (elapsed >= 8000) enter(LivingCanvasScreen::Home, nowMs);
      break;
    case LivingCanvasScreen::Home:
    case LivingCanvasScreen::Menu:
    case LivingCanvasScreen::MysteryBoxes:
      break;
  }
}

std::size_t LivingCanvasState::homeFrame(std::uint32_t elapsedMs,
                                         std::size_t frameCount,
                                         std::uint32_t frameIntervalMs) const {
  if (frameCount == 0 || frameIntervalMs == 0) return 0;
  return (elapsedMs / frameIntervalMs) % frameCount;
}
