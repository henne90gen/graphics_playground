#pragma once

#include <gl/Shader.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <glm/glm.hpp>

#include "ShaderToggles.h"
#include "TerrainParams.h"

struct TerrainLevels {
    float grassLevel = 0.4F;
    float rockLevel = 1.0F;
    float blur = 0.05F;
};

class Terrain {
    std::shared_ptr<Shader> terrainShader;

    std::shared_ptr<VertexArray> terrainVA;

    std::shared_ptr<Texture> grassTexture;
    std::shared_ptr<Texture> dirtTexture;
    std::shared_ptr<Texture> rockTexture;

    glm::vec3 modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    glm::vec3 modelPosition = glm::vec3(0.0F);
    glm::vec3 modelRotation = glm::vec3(0.0F);

    TerrainLevels levels = {};
    float tessellation = 60.0F;
    float uvScaleFactor = 20.0F;

    void initTextures();
    static std::shared_ptr<VertexArray> generatePoints(const std::shared_ptr<Shader> &shader);

  public:
    TerrainParams terrainParams = {};

    [[nodiscard]] const std::shared_ptr<Texture> &getGrassTexture() const { return grassTexture; }
    [[nodiscard]] const std::shared_ptr<Texture> &getDirtTexture() const { return dirtTexture; }
    [[nodiscard]] const std::shared_ptr<Texture> &getRockTexture() const { return rockTexture; }

    void init();
    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles);
    void showGui();
    void showLayersGui();
};
