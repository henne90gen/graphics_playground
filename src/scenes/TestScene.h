#pragma once

#include "scenes/Scene.h"

#include <functional>

class TestScene : public Scene {
  public:
    TestScene(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "Test Scene"){};
    virtual ~TestScene(){};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
};
