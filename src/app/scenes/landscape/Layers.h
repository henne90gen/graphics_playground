#pragma once

#include "util/ImGuiUtils.h"

struct Layer {
    bool enabled = true;
    float weight = 1.0F;

    Layer() = default;
    explicit Layer(float weight) : weight(weight) {}
    virtual ~Layer() = default;

    float getWeightedValue(float width, float height, float x, float y, float z) {
        return getValue(width, height, x, y, z) * this->weight;
    }

    bool renderMenu(int i) {
        ImGui::Separator();

        const auto btnLabel = "Remove Layer " + std::to_string(i);
        bool shouldBeRemoved = ImGui::Button(btnLabel.c_str());

        ImGui::SameLine();
        const auto enabledLabel = "Enabled " + std::to_string(i);
        ImGui::Checkbox(enabledLabel.c_str(), &enabled);

        const auto weightLabel = "Weight " + std::to_string(i);
        ImGui::SliderFloat(weightLabel.c_str(), &weight, 0.0F, 1.0F);

        renderSpecialMenu(i);

        return shouldBeRemoved;
    }

  protected:
    virtual float getValue(float width, float height, float x, float y, float z) = 0;
    virtual void renderSpecialMenu(int i) = 0;
};

struct NoiseLayer : Layer {
    FastNoise *noise = new FastNoise();

    NoiseLayer() = default;
    NoiseLayer(float weight, float frequency) : Layer(weight) { noise->SetFrequency(frequency); }
    ~NoiseLayer() { delete noise; }

    void renderSpecialMenu(int i) override {
        auto noiseType = noise->GetNoiseType();
        const auto typeLabel = "Noise Type " + std::to_string(i);
        ImGui::NoiseTypeSelector(typeLabel.c_str(), &noiseType);
        noise->SetNoiseType(noiseType);

        auto frequency = noise->GetFrequency();
        const auto frequencyLabel = "Frequency " + std::to_string(i);
        ImGui::SliderFloat(frequencyLabel.c_str(), &frequency, 0.0F, 1.0F);
        noise->SetFrequency(frequency);
    }

    float getValue(float width, float height, float x, float y, float z) override { return noise->GetNoise(x, y, z); }
};

struct BowlLayer : Layer {
    explicit BowlLayer(float weight) : Layer(weight) {}

    void renderSpecialMenu(int i) override {}

    float getValue(float width, float height, float x, float y, float z) override {
        float adjustedX = x - (width / 2);
        float adjustedY = y - (height / 2);
        return adjustedX * adjustedX + adjustedY * adjustedY;
    }
};
