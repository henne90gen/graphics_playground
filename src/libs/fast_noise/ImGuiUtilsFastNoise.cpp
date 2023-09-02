#include "ImGuiUtilsFastNoise.h"

void ImGui::NoiseTypeSelector(FastNoiseLite::NoiseType *noiseType) {
    ImGui::NoiseTypeSelector("Noise Algorithm", noiseType);
}

void ImGui::NoiseTypeSelector(const char *label, FastNoiseLite::NoiseType *noiseType) {
    static const std::array<const char *, 6> items = {"OpenSimplex2",   "OpenSimplex2S",   "Cellular",   "Perlin",
                                                       "ValueCubic", "Value"};
    ImGui::Combo(label, reinterpret_cast<int *>(noiseType), items.data(), items.size());
}
