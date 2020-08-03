#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

struct Raindrop {
    glm::vec3 startingPosition = {};
    float acceleration = 0.0F;
    float velocity = 0.0F;
    float sediment = 0.0F;
    std::vector<glm::vec3> path = {};
};

class TerrainErosion : public Scene {
  public:
    explicit TerrainErosion(SceneData &data) : Scene(data, "TerrainErosion"){};
    ~TerrainErosion() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> pathShader;

    std::shared_ptr<VertexArray> terrainVA;
    std::vector<float> heightMap = std::vector<float>();

    std::shared_ptr<VertexBuffer> heightBuffer;

    FastNoise *noise1;
    FastNoise *noise2;
    FastNoise *noise3;

    void renderTerrain(bool wireframe);
    void renderPaths(const std::vector<Raindrop> &paths);

    void generatePoints();

    void updateHeightBuffer();
    void simulateRaindrop(Raindrop &path);
    void regenerateTerrain();
    void regenerateRaindrops(std::vector<Raindrop> &paths) const;
    void adjustRaindropsToTerrain(std::vector<Raindrop> &raindrops);
};
