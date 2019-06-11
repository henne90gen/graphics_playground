#pragma once

#include "scenes/Scene.h"

#include <functional>

class ModelLoading : public Scene {
  public:
    ModelLoading(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "ModelLoading"){};

    ~ModelLoading() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
