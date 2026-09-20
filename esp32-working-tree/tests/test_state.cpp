#include <cassert>
#include <cstdint>

#include "living_canvas_state.h"

int main() {
  LivingCanvasState state;
  assert(state.screen() == LivingCanvasScreen::Home);

  state.wake(100);
  assert(state.screen() == LivingCanvasScreen::Menu);
  assert(state.selectedChoice() == LivingCanvasChoice::None);

  assert(!state.choose(LivingCanvasChoice::Takeout, 200));
  assert(state.screen() == LivingCanvasScreen::Menu);
  assert(state.selectedChoice() == LivingCanvasChoice::Takeout);
  assert(state.choose(LivingCanvasChoice::Takeout, 300));
  assert(state.screen() == LivingCanvasScreen::TakeoutListening);

  state.update(2299);
  assert(state.screen() == LivingCanvasScreen::TakeoutListening);
  state.update(12300);
  assert(state.screen() == LivingCanvasScreen::TakeoutListening);
  assert(state.submitVoice(12300));
  assert(state.screen() == LivingCanvasScreen::TakeoutAnalyzing);
  state.update(15300);
  assert(state.screen() == LivingCanvasScreen::TakeoutResult);
  state.update(23300);
  assert(state.screen() == LivingCanvasScreen::Home);

  state.wake(14000);
  state.choose(LivingCanvasChoice::HomeCooking, 14100);
  state.choose(LivingCanvasChoice::HomeCooking, 14200);
  assert(state.screen() == LivingCanvasScreen::HomeCookingListening);
  assert(state.submitVoice(14250));
  assert(state.screen() == LivingCanvasScreen::HomeCookingResult);
  state.back(14300);
  assert(state.screen() == LivingCanvasScreen::Menu);
  state.back(14400);
  assert(state.screen() == LivingCanvasScreen::Home);

  state.wake(15000);
  state.choose(LivingCanvasChoice::Mystery, 15100);
  state.choose(LivingCanvasChoice::Mystery, 15200);
  assert(state.screen() == LivingCanvasScreen::MysteryBoxes);
  state.chooseMysteryBox(2, 15300);
  assert(state.screen() == LivingCanvasScreen::MysteryResult);
  assert(state.selectedBox() == 2);

  assert(state.homeFrame(0, 8, 125) == 0);
  assert(state.homeFrame(124, 8, 125) == 0);
  assert(state.homeFrame(125, 8, 125) == 1);
  assert(state.homeFrame(1000, 8, 125) == 0);
  assert(state.homeFrame(999999, 0, 125) == 0);
  assert(state.homeFrame(999999, 8, 0) == 0);
  return 0;
}
