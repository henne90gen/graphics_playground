#pragma once

#include <FastNoiseLite.h>
#include <array>
#include <imgui.h>

namespace ImGui {
void NoiseTypeSelector(FastNoiseLite::NoiseType *noiseType);
void NoiseTypeSelector(const char *label, FastNoiseLite::NoiseType *noiseType);
} // namespace ImGui
