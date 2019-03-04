#pragma once

#include "scenes/Scene.h"

class Triangle : public Scene {
  public:
    Triangle(GLFWwindow *window, std::function<void()> &backToMainMenu) : Scene(window, backToMainMenu, "Triangle") {}
    virtual ~Triangle() {}

    virtual void tick() override;
};