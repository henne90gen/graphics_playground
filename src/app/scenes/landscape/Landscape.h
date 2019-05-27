#pragma once

#include "scenes/Scene.h"

#include <functional>

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

    float *heightMap;
    unsigned int heightMapSize;
    VertexBuffer *heightBuffer;

    int pointDensity = 1;

    void updateHeightBuffer(const glm::vec3 &scale, const glm::vec2 &movement, int noiseMode) const;
};
