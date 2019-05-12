#pragma once

#include "scenes/Scene.h"

#include <glad/glad.h>

#include "opengl/Shader.h"
#include "opengl/VertexBuffer.h"

class Triangle : public Scene {
  public:
    Triangle(GLFWwindow *window, std::function<void()> &backToMainMenu) : Scene(window, backToMainMenu, "Triangle") {}
    virtual ~Triangle() {}

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;

  private:
    Shader *shader;
    VertexBuffer *positionBuffer;
    VertexBuffer *colorBuffer;
    GLuint positionLocation;
    GLuint colorLocation;
};