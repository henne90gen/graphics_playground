#pragma once

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <memory>

#include "Layers.h"
#include "ShaderToggles.h"
#include "TerrainParams.h"

class Trees {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> cubeVA;

    int treeCount = 10000;

  public:
    void init();

    void showGui();

    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                const TerrainParams &terrainParams);
};
