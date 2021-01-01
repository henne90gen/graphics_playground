#include "ImGuiUtilsFastNoise.h"

void ImGui::NoiseTypeSelector(FastNoise::NoiseType *noiseType) {
    ImGui::NoiseTypeSelector("Noise Algorithm", noiseType);
}

void ImGui::NoiseTypeSelector(const char *label, FastNoise::NoiseType *noiseType) {
    static const std::array<const char *, 10> items = {"Value",   "ValueFractal",   "Perlin",   "PerlinFractal",
                                                       "Simplex", "SimplexFractal", "Cellular", "WhiteNoise",
                                                       "Cubic",   "CubicFractal"};
    ImGui::Combo(label, reinterpret_cast<int *>(noiseType), items.data(), items.size());
}
