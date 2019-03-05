#pragma once

#include "scenes/Scene.h"

class LegacyTriangle : public Scene {
  public:
    LegacyTriangle(GLFWwindow *window, std::function<void()> &backToMainMenu)
        : Scene(window, backToMainMenu, "LegacyTriangle") {}
    virtual ~LegacyTriangle() {}

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
};