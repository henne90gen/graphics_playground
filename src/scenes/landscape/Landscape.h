#pragma once

#include "Scene.h"

#include <functional>
#include <memory>

#include "Layers.h"
#include "gl/IndexBuffer.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"

struct TessellationLevels {
    float innerTess = 20.0F;
    glm::vec3 outerTess = glm::vec3(20.0F);
};

struct TerrainLevels {
    float grassLevel = 0.25F;
    float rockLevel = 0.75F;
    float blur = 0.05F;
};

struct ShaderToggles {
    bool showNormals = false;
    bool showTangents = false;
    bool showUVs = false;
    bool drawWireframe = false;
    bool useFiniteDifferences = false;
};

class Landscape : public Scene {
  public:
    explicit Landscape() : Scene("Landscape"){};

    ~Landscape() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;

    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<VertexArray> textureVA;

    std::shared_ptr<Shader> flatShader;
    std::shared_ptr<VertexArray> cubeVA;

    std::shared_ptr<Texture> grassTexture;
    std::shared_ptr<Texture> noiseTexture;
    std::shared_ptr<Texture> normalTexture;

    void generatePoints();
    void renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &modelPosition,
                       const glm::vec3 &modelRotation, const glm::vec3 &modelScale, const glm::vec3 &lightPosition,
                       const glm::vec3 &lightDirection, const glm::vec3 &lightColor, float lightPower,
                       const TerrainLevels &levels, const ShaderToggles &shaderToggles, float uvScaleFactor,
                       float tessellation, const std::vector<NoiseLayer> &vector, float power, float finiteDifference);
    void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition,
                     const glm::vec3 &lightColor);

    void renderNoiseTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                            glm::vec3 &textureScale);
    void renderNormalTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                             glm::vec3 &textureScale);
};
