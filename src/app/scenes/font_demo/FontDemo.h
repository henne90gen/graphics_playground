#pragma once

#include "scenes/Scene.h"

#include <functional>

class FontDemo : public Scene {
public:
    FontDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "FontDemo") {};

    ~FontDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
