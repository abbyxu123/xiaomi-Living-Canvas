#pragma once

#include <cstddef>

constexpr const char *LIVING_CANVAS_MENU_PATH = "/menu.rgb565";
constexpr const char *LIVING_CANVAS_MENU_TAKEOUT_PATH = "/menu_takeout.rgb565";
constexpr const char *LIVING_CANVAS_MENU_MIXBOX_PATH = "/menu_mixbox.rgb565";
constexpr const char *LIVING_CANVAS_MENU_EATATHOME_PATH = "/menu_eatathome.rgb565";
void livingCanvasHomePath(std::size_t frame, char *buffer, std::size_t bufferSize);
