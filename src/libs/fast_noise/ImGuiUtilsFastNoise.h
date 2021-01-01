#pragma once

#include <FastNoise.h>
#include <array>
#include <imgui.h>

namespace ImGui {
void NoiseTypeSelector(FastNoise::NoiseType *noiseType);
void NoiseTypeSelector(const char *label, FastNoise::NoiseType *noiseType);
} // namespace ImGui
