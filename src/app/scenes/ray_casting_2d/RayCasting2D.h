#pragma once

#include "scenes/Scene.h"

#include <functional>

class RayCasting2D : public Scene {
public:
    RayCasting2D(SceneData &data)
            : Scene(data, "RayCasting2D") {};

    ~RayCasting2D() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
