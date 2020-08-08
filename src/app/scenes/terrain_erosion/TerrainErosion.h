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
    float water = 0.0F;
    float sediment = 0.0F;
    std::vector<glm::vec3> path = {};
};

struct SimulationParams {
    float Kq = 10;          // constant parameter for soil carry capacity formula
    float Kw = 0.001f;      // water evaporation speed
    float Kr = 0.9f;        // erosion speed
    float Kd = 0.02f;       // deposition speed
    float Ki = 0.1F;        // direction inertia
    float minSlope = 0.05f; // minimum slope for soil carry capacity formula
    float g = 20;           // gravity
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

    void generateTerrainMesh();

    void setHeightMapValue(int x, int z, float value);
    float getHeightMapValue(int x, int z);
    void simulateRaindrop(const SimulationParams &params, Raindrop &raindrop);
    void regenerateTerrain();
    void regenerateRaindrops(std::vector<Raindrop> &raindrops, unsigned int raindropCount) const;
    void adjustRaindropsToTerrain(std::vector<Raindrop> &raindrops);
};
