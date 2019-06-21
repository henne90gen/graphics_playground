#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"

class Landscape : public Scene {
public:
    Landscape(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "Landscape") {};

    ~Landscape() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::vector<float> heightMap = std::vector<float>();

    std::shared_ptr<VertexBuffer> heightBuffer;
    FastNoise *noise;

    void generatePoints(unsigned int pointDensity);

    void updateHeightBuffer(unsigned int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                            FastNoise::NoiseType &noiseType);
};
