#pragma once

#include "scenes/Scene.h"

#include <functional>

class SurfaceSmoothing : public Scene {
public:
    SurfaceSmoothing(SceneData &data)
            : Scene(data, "SurfaceSmoothing") {};

    ~SurfaceSmoothing() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
