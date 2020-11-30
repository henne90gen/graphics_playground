#pragma once

#include "scenes/Scene.h"

class LegacyTriangle : public Scene {
public:
    explicit LegacyTriangle(SceneData data) : Scene(data, "LegacyTriangle") {}

    ~LegacyTriangle() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};