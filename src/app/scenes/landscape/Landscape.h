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
    void generatePoints();

    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;

    std::vector<float> heightMap = std::vector<float>();
    VertexBuffer *heightBuffer;

    int pointDensity = 1;
    FastNoise *noise;

    void updateHeightBuffer(const glm::vec3 &scale, const glm::vec2 &movement, FastNoise::NoiseType &noiseType);
};
