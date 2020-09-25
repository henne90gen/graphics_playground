#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

class Landscape : public Scene {
  public:
    explicit Landscape(SceneData data) : Scene(data, "Landscape"){};

    ~Landscape() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> heightBuffer;
    std::vector<float> heightMap = std::vector<float>();

    FastNoise *noise;

    void generatePoints(unsigned int pointDensity);

    void updateHeightBuffer(unsigned int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                            FastNoise::NoiseType &noiseType);
};
