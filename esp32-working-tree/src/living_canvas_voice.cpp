#include "living_canvas_voice.h"

namespace {
std::uint32_t magnitude(std::int16_t sample) {
  return sample < 0 ? static_cast<std::uint32_t>(-static_cast<std::int32_t>(sample))
                    : static_cast<std::uint32_t>(sample);
}
}  // namespace

std::size_t livingCanvasStereoToMono(
    std::int16_t *samples,
    std::size_t stereoBytes,
    LivingCanvasVoiceMetrics *metrics) {
  if (metrics != nullptr) *metrics = {};
  if (samples == nullptr || stereoBytes < 4 || stereoBytes % 4 != 0) return 0;

  LivingCanvasVoiceMetrics result{};
  result.frames = stereoBytes / 4;
  for (std::size_t frame = 0; frame < result.frames; ++frame) {
    const std::int16_t left = samples[frame * 2];
    const std::int16_t right = samples[frame * 2 + 1];
    const std::int16_t mono = static_cast<std::int16_t>(
        (static_cast<std::int32_t>(left) + static_cast<std::int32_t>(right)) / 2);
    samples[frame] = mono;
    const std::uint32_t leftMagnitude = magnitude(left);
    const std::uint32_t rightMagnitude = magnitude(right);
    const std::uint32_t monoMagnitude = magnitude(mono);
    if (leftMagnitude > result.leftPeak) result.leftPeak = leftMagnitude;
    if (rightMagnitude > result.rightPeak) result.rightPeak = rightMagnitude;
    if (monoMagnitude > result.monoPeak) result.monoPeak = monoMagnitude;
  }
  if (metrics != nullptr) *metrics = result;
  return result.frames * sizeof(std::int16_t);
}

std::size_t livingCanvasVoiceBufferBytes(
    std::uint32_t sampleRate,
    std::uint8_t channels,
    std::uint8_t bitsPerSample,
    std::uint8_t seconds) {
  if (bitsPerSample % 8 != 0) return 0;
  return static_cast<std::size_t>(sampleRate) * channels *
         (bitsPerSample / 8) * seconds;
}

bool livingCanvasVoiceCanSubmit(std::size_t monoBytes,
                                std::uint32_t monoPeak) {
  return monoBytes >= 32000 && monoPeak >= 1000;
}
