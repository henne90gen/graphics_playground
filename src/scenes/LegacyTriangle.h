#pragma once

#include "scenes/Scene.h"

class LegacyTriangle : public Scene {
  public:
    LegacyTriangle(GLFWwindow *window, std::function<void()> &backToMainMenu)
        : Scene(window, backToMainMenu, "LegacyTriangle") {}
    virtual ~LegacyTriangle() {}

    virtual void tick() override;
};