#pragma once

struct NoiseLayer {
    enum NoiseType {
        NORMAL = 0,
        RIDGE = 1,
    };

    bool enabled = true;
    float frequency = 1.0F;
    float amplitude = 1.0F;
    NoiseType noiseType = NORMAL;

    NoiseLayer() = default;
    NoiseLayer(float frequency, float amplitude) : frequency(frequency), amplitude(amplitude) {}
};

#if 0
struct RidgeNoiseLayer : NoiseLayer {
    explicit RidgeNoiseLayer() = default;
    explicit RidgeNoiseLayer(float weight, float frequency) : NoiseLayer(weight, frequency) {}

    float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) override {
        return 2.0F * (0.5F - std::abs(0.5F - NoiseLayer::getValue(width, height, samplePos, currentHeight)));
    }
};

struct BowlLayer : NoisLayer {
    explicit BowlLayer() = default;
    explicit BowlLayer(float weight) : NoisLayer(weight) {}

    void renderSpecialMenu(int i) override {}

    float getValue(unsigned int width, unsigned int height, const glm::vec3 &samplePos, float currentHeight) override {
        float adjustedX = samplePos.x - static_cast<float>(width / 2);
        float adjustedY = samplePos.y - static_cast<float>(height / 2);
        return adjustedX * adjustedX + adjustedY * adjustedY;
    }
};
#endif
