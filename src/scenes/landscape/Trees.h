#pragma once

#define USE_TREE_MODELS 0

#include <Model.h>
#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <memory>

#include "Layers.h"
#include "ShaderToggles.h"
#include "TerrainParams.h"

class Trees {
    std::shared_ptr<Shader> shader;

    std::shared_ptr<Shader> compShader;
    unsigned int treePositionTextureWidth = 64;
    unsigned int treePositionTextureHeight = 64;
    unsigned int treePositionTextureId = 0;

#if USE_TREE_MODELS
    Model treeModel = {};
    int vertexCount = 0;
#else
    std::shared_ptr<VertexArray> cubeVA;
#endif

    int treeCount = 1000;

  public:
    [[nodiscard]] unsigned int getTreePositionTextureId() const { return treePositionTextureId; }

    void init();
    void initComputeShaderStuff();

    void showGui();

    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                const TerrainParams &terrainParams);
};
