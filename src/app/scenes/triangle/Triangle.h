#pragma once

#include "scenes/Scene.h"

#include <glad/glad.h>

#include "opengl/Shader.h"
#include "opengl/VertexBuffer.h"

class Triangle : public Scene {
public:
    Triangle(GLFWwindow *window, std::function<void()> &backToMainMenu) : Scene(window, backToMainMenu, "Triangle") {}

    ~Triangle() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    Shader *shader;

    VertexBuffer *positionBuffer;
    VertexBuffer *colorBuffer;

    GLuint positionLocation;
    GLuint colorLocation;
};