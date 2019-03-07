#pragma once

#include "scenes/Scene.h"

#include <functional>

class Cube : public Scene {
  public:
    Cube(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "Cube"){};
    virtual ~Cube(){};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
};
