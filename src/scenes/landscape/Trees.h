#pragma once

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <Model.h>
#include <memory>

#include "Layers.h"
#include "ShaderToggles.h"
#include "TerrainParams.h"

class Trees {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> cubeVA;
    Model treeModel;

    int treeCount = 1000;

    int vertexCount = 0;

  public:
    void init();

    void showGui();

    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                const TerrainParams &terrainParams);
};
