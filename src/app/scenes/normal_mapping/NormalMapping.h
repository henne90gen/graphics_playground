#pragma once

#include "scenes/Scene.h"

#include <functional>

class NormalMapping : public Scene {
  public:
    NormalMapping(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "NormalMapping"){};

    ~NormalMapping() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
