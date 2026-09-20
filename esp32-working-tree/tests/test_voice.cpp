#include "living_canvas_voice.h"

#include <cassert>
#include <cstdint>

int main() {
  int16_t samples[] = {1000, 3000, -2000, 0, 32767, -32768};
  LivingCanvasVoiceMetrics metrics{};
  const std::size_t monoBytes = livingCanvasStereoToMono(
      samples, sizeof(samples), &metrics);
  assert(monoBytes == 3 * sizeof(int16_t));
  assert(samples[0] == 2000);
  assert(samples[1] == -1000);
  assert(samples[2] == 0);
  assert(metrics.frames == 3);
  assert(metrics.leftPeak == 32767);
  assert(metrics.rightPeak == 32768);
  assert(metrics.monoPeak == 2000);

  assert(livingCanvasStereoToMono(nullptr, 0, &metrics) == 0);
  assert(livingCanvasStereoToMono(samples, 3, &metrics) == 0);
  assert(livingCanvasVoiceBufferBytes(16000, 2, 16, 10) == 640000);
  assert(!livingCanvasVoiceCanSubmit(31998, 0));
  assert(!livingCanvasVoiceCanSubmit(32000, 200));
  assert(livingCanvasVoiceCanSubmit(32000, 1200));
  return 0;
}
