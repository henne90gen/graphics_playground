#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <glad/glad.h>

class Texture : public Scene {
  public:
    Texture(GLFWwindow *window, std::function<void(void)> &backToMainMenu) : Scene(window, backToMainMenu, "Texture"){};
    virtual ~Texture(){};

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
