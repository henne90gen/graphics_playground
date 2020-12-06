#pragma once

#include "util/ImGuiUtils.h"
#include "util/ImGuiUtilsFastNoise.h"

struct Layer {
    bool enabled = true;
    float weight = 1.0F;

    Layer() = default;
    explicit Layer(float weight) : weight(weight) {}
    virtual ~Layer() = default;

    float getWeightedValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) {
        return getValue(width, height, samplePos, currentHeight) * this->weight;
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
    virtual float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos,
                           float currentHeight) = 0;
    virtual void renderSpecialMenu(int i) = 0;
};

struct NoiseLayer : Layer {
    FastNoise *noise = new FastNoise();
    float amplitudeFactor = 1.0F;

    NoiseLayer() = default;
    NoiseLayer(float weight, float frequency) : Layer(weight) { noise->SetFrequency(frequency); }
    ~NoiseLayer() override { delete noise; }

    void renderSpecialMenu(int i) override {
        auto noiseType = noise->GetNoiseType();
        const auto typeLabel = "Noise Type " + std::to_string(i);
        ImGui::NoiseTypeSelector(typeLabel.c_str(), &noiseType);
        noise->SetNoiseType(noiseType);

        auto frequency = noise->GetFrequency();
        const auto frequencyLabel = "Frequency " + std::to_string(i);
        ImGui::SliderFloat(frequencyLabel.c_str(), &frequency, 0.0F, 1.0F);
        noise->SetFrequency(frequency);

        const auto amplitudeLabel = "Amplitude " + std::to_string(i);
        ImGui::DragFloat(amplitudeLabel.c_str(), &amplitudeFactor, 0.001F);
    }

    float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) override {
        return noise->GetNoise(samplePos.x, samplePos.y, samplePos.z) * amplitudeFactor;
    }
};

struct RidgeNoiseLayer : NoiseLayer {
    explicit RidgeNoiseLayer() = default;
    explicit RidgeNoiseLayer(float weight, float frequency) : NoiseLayer(weight, frequency) {}

    float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) override {
        return 2.0F * (0.5F - std::abs(0.5F - NoiseLayer::getValue(width, height, samplePos, currentHeight)));
    }
};

struct BowlLayer : Layer {
    explicit BowlLayer() = default;
    explicit BowlLayer(float weight) : Layer(weight) {}

    void renderSpecialMenu(int i) override {}

    float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) override {
        float adjustedX = samplePos.x - static_cast<float>(width / 2);
        float adjustedY = samplePos.y - static_cast<float>(height / 2);
        return adjustedX * adjustedX + adjustedY * adjustedY;
    }
};
