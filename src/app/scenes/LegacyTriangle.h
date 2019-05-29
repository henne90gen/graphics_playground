#pragma once

#include "Scene.h"

class LegacyTriangle : public Scene {
public:
    LegacyTriangle(GLFWwindow *window, std::function<void()> &backToMainMenu)
            : Scene(window, backToMainMenu, "LegacyTriangle") {}

    ~LegacyTriangle() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};