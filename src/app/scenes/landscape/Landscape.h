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
    VertexArray *vertexArray;

    Shader *shader;
    IndexBuffer *indexBuffer;
    std::vector<float> heightMap = std::vector<float>();

    VertexBuffer *heightBuffer;
    FastNoise *noise;

    void generatePoints(unsigned int pointDensity);

    void updateHeightBuffer(const unsigned int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                            FastNoise::NoiseType &noiseType);
};
