#pragma once

#include "scenes/Scene.h"

#include <glad/glad.h>

#include <functional>

class Cube : public Scene {
  public:
    Cube(GLFWwindow *window, std::function<void(void)> &backToMainMenu) : Scene(window, backToMainMenu, "Cube"){};
    virtual ~Cube(){};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;

  private:
    GLuint programId;
    GLuint vertexbuffer;
    GLuint uvBuffer;
    GLuint textureId;
    GLuint positionLocation;
    GLuint uvLocation;
};
