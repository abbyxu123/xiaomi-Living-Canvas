#pragma once

#include <cstddef>
#include <cstdint>

struct LivingCanvasVoiceMetrics {
  std::size_t frames = 0;
  std::uint32_t leftPeak = 0;
  std::uint32_t rightPeak = 0;
  std::uint32_t monoPeak = 0;
};

std::size_t livingCanvasStereoToMono(
    std::int16_t *samples,
    std::size_t stereoBytes,
    LivingCanvasVoiceMetrics *metrics);

std::size_t livingCanvasVoiceBufferBytes(
    std::uint32_t sampleRate,
    std::uint8_t channels,
    std::uint8_t bitsPerSample,
    std::uint8_t seconds);

bool livingCanvasVoiceCanSubmit(std::size_t monoBytes,
                                std::uint32_t monoPeak);
