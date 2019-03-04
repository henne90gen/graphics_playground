#pragma once

#include "Scene.h"

#include <functional>

class TestScene : public Scene {
  public:
    TestScene(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "Test Scene"){};
    virtual ~TestScene(){};

    virtual void tick() override;
};
