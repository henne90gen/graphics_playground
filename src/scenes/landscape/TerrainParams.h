#pragma once

#include <gl/Shader.h>
#include <memory>
#include <vector>

#include "Layers.h"

struct TerrainParams {
  private:
    std::vector<NoiseLayer> noiseLayers;
    float finiteDifference = 0.01F;
    bool useFiniteDifferences = false;
    float power = 1.1F;
    float bowlStrength = 20.0F;
    float platformHeight = 0.15F;
    int seed = 1337;

  public:
    TerrainParams();

    void showGui();
    void showLayersGui();

    void setShaderUniforms(const std::shared_ptr<Shader> &shader) const;
};
