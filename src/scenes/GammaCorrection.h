#pragma once

#include "scenes/Scene.h"

#include <functional>

class GammaCorrection : public Scene {
  public:
    GammaCorrection(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "GammaCorrection"){};
    virtual ~GammaCorrection(){};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
};
