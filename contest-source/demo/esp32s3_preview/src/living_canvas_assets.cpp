#include "living_canvas_assets.h"

#include <cstdio>

void livingCanvasHomePath(std::size_t frame, char *buffer, std::size_t bufferSize) {
  if (buffer == nullptr || bufferSize == 0) return;
  std::snprintf(buffer, bufferSize, "/home_%02zu.rgb565", frame);
}
