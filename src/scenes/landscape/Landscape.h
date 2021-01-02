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
    float grassLevel = 0.4F;
    float rockLevel = 1.0F;
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
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexArray> textureVA;
    std::shared_ptr<VertexArray> cubeVA;

    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<Shader> flatShader;
    std::shared_ptr<Shader> skyShader;

    std::shared_ptr<Texture> grassTexture;
    std::shared_ptr<Texture> dirtTexture;
    std::shared_ptr<Texture> rockTexture;

    std::shared_ptr<VertexArray> generatePoints(const std::shared_ptr<Shader> &shader);
    void renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &modelPosition,
                       const glm::vec3 &modelRotation, const glm::vec3 &modelScale, const glm::vec3 &lightPosition,
                       const glm::vec3 &lightDirection, const glm::vec3 &lightColor, float lightPower,
                       const TerrainLevels &levels, const ShaderToggles &shaderToggles, float uvScaleFactor,
                       float tessellation, const std::vector<NoiseLayer> &vector, float power, float finiteDifference);
    void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition,
                     const glm::vec3 &lightColor);
    void renderSky(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &skyScale,
                   const glm::vec3 &skyColor, float cloudAnimationSpeed);

    void renderTexture(const glm::vec3 &texturePosition, float zoom, const std::shared_ptr<Texture> &texture);
    void initTextures();
};
