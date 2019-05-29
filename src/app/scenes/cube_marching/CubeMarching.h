#pragma once

#include "scenes/Scene.h"

#include <functional>

class CubeMarching : public Scene {
  public:
    CubeMarching(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "CubeMarching"){};
    ~CubeMarching() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
};
