#pragma once

#include "scenes/Scene.h"

#include <functional>

class TestScene : public Scene {
public:
    TestScene(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "Test Scene") {};

    ~TestScene() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;
};
