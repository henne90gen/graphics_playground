#pragma once

#include "scenes/Scene.h"

#include <functional>

class Texture : public Scene {
  public:
    Texture(GLFWwindow *window, std::function<void(void)> &backToMainMenu) : Scene(window, backToMainMenu, "Texture"){};
    virtual ~Texture(){};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
};
