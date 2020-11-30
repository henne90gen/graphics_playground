#pragma once

#include "Scene.h"

#include <functional>

class TestScene : public Scene {
public:
    explicit TestScene() : Scene( "Test Scene") {};

    ~TestScene() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};
