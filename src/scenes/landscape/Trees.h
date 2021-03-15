#pragma once

#define USE_TREE_MODELS 1

#include <Model.h>
#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <memory>

#include "Layers.h"
#include "ShaderToggles.h"
#include "TerrainParams.h"

class Trees {
    std::shared_ptr<Shader> shader = nullptr;
    std::shared_ptr<Shader> flatColorShader = nullptr;

    std::shared_ptr<Shader> compShader = nullptr;
    unsigned int treePositionTextureWidth = 32;
    unsigned int treePositionTextureHeight = 32;
    unsigned int treePositionTextureId = 0;

#if USE_TREE_MODELS
    Model treeModel = {};
    int vertexCount = 0;
#else
    std::shared_ptr<VertexArray> cubeVA = nullptr;
#endif

    int treeCount = 1024;
    float lodSize = 1000.0F;
    float lodInnerSize = 100.0F;

    std::shared_ptr<VertexArray> gridVA = nullptr;
    float gridHeight = 120.0F;

  public:
    [[nodiscard]] unsigned int getTreePositionTextureId() const { return treePositionTextureId; }

    void init();
    void initComputeShader();
    void initModel();
    void initGrid();

    void showGui();

    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                const TerrainParams &terrainParams);
    void renderComputeShader(const TerrainParams &terrainParams);
    void renderTreeModels(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                          const ShaderToggles &shaderToggles);
    void renderCubes(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                     const TerrainParams &terrainParams);
    void renderGrid(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);
};
