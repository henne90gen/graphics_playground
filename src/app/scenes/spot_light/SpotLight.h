#pragma once

#include "scenes/Scene.h"

#include <functional>

class SpotLight : public Scene {
public:
    explicit SpotLight(SceneData &data)
        : Scene(data, "SpotLight"){};
    ~SpotLight() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
};
