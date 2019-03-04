#pragma once

#include "scenes/Scene.h"

class AdvancedTriangle : public Scene {
  public:
    AdvancedTriangle(GLFWwindow *window, std::function<void()> &backToMainMenu)
        : Scene(window, backToMainMenu, "AdvancedTriangle") {}
    virtual ~AdvancedTriangle() {}

    virtual void tick() override;
};