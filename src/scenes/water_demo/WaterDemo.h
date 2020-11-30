#pragma once

#include "scenes/Scene.h"

#include <functional>

class WaterDemo : public Scene {
public:
    WaterDemo(SceneData &data)
            : Scene(data, "WaterDemo") {};

    ~WaterDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
