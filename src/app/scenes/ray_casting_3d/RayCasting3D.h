#pragma once

#include "scenes/Scene.h"

#include <functional>

class RayCasting3D : public Scene {
public:
    RayCasting3D(SceneData &data)
            : Scene(data, "RayCasting3D") {};

    ~RayCasting3D() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
