#pragma once

#include "Scene.h"

class LegacyTriangle : public Scene {
  public:
    explicit LegacyTriangle() : Scene("LegacyTriangle") {}

    ~LegacyTriangle() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;
};