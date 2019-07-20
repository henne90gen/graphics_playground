#pragma once

#include "scenes/Scene.h"

#include <functional>

class TestScene : public Scene {
public:
    explicit TestScene(SceneData data) : Scene(data, "Test Scene") {};

    ~TestScene() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;
};
