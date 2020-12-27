#pragma once

#include "Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

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
    bool useNormalMap = true;
    bool showUVs = false;
    bool drawWireframe = true;
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
    std::shared_ptr<VertexBuffer> heightBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;
    std::shared_ptr<VertexBuffer> tangentBuffer;
    std::shared_ptr<VertexBuffer> biTangentBuffer;

    std::vector<float> heightMap = {};
    std::vector<glm::vec2> vertices = {};
    std::vector<glm::vec2> uvs = {};
    std::vector<glm::ivec3> indices = {};

    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<VertexArray> textureVA;

    std::shared_ptr<Texture> noiseTexture;
    std::shared_ptr<Texture> normalTexture;

    void generatePoints();
    void updateHeightBuffer(unsigned int pointDensity, const glm::vec3 &movement,
                            const std::vector<NoiseLayer *> &layers, float power, float platformHeight);
    void renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &modelPosition,
                       const glm::vec3 &modelRotation, const glm::vec3 &modelScale, const glm::vec3 &surfaceToLight,
                       const glm::vec3 &lightColor, float lightPower, const TerrainLevels &levels,
                       const ShaderToggles &shaderToggles, float uvScaleFactor, const TessellationLevels &tessLevels,
                       const std::vector<NoiseLayer> &vector);
    void renderNoiseTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                            glm::vec3 &textureScale);
    void updateNormalTexture(unsigned int pointDensity, const glm::vec3 &movement,
                             const std::vector<NoiseLayer *> &layers, float power, float normalScale);
    void renderNormalTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                             glm::vec3 &textureScale);
};
