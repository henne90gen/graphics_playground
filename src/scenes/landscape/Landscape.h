#pragma once

#include "Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "Layers.h"
#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"

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
    std::vector<float> heightMap = std::vector<float>();

    std::shared_ptr<Shader> noiseTextureShader;
    std::shared_ptr<VertexArray> noiseTextureVA;
    std::shared_ptr<Texture> noiseTexture;

    void generatePoints(unsigned int pointDensity);
    void updateHeightBuffer(unsigned int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                            const std::vector<Layer *> &layers);
    void renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &modelRotation,
                       const glm::vec3 &modelScale, int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                       const std::vector<Layer *> &layers, bool drawWireframe);
    void Landscape::renderNoiseTexture(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                                       const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                                       const glm::vec3 &textureScale);
};
