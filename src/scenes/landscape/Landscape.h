#pragma once

#include "Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "Layers.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"

struct TerrainLevels {
    float grassLevel = 0.25F;
    float rockLevel = 0.75F;
    float blur = 0.05F;
};

struct ShaderToggles {
    bool useNormalMap = true;
    bool showUVs;
    bool drawWireframe;
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

    void generatePoints(unsigned int pointDensity);
    void updateHeightBuffer(unsigned int pointDensity, const glm::vec3 &movement, const std::vector<Layer *> &layers,
                            float power, float platformHeight);
    void renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &modelPosition,
                       const glm::vec3 &modelRotation, const glm::vec3 &modelScale, const glm::vec3 &surfaceToLight,
                       const glm::vec3 &lightColor, float lightPower, const TerrainLevels &levels,
                       const ShaderToggles &shaderToggles, float uvScaleFactor);
    void renderNoiseTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                            glm::vec3 &textureScale);
    void updateNormalTexture(unsigned int pointDensity, const glm::vec3 &movement, const std::vector<Layer *> &layers,
                             float power, float normalScale);
    void renderNormalTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                             glm::vec3 &textureScale);
};
