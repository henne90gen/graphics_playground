#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>
#include <random>

#include <FastNoise.h>

#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

struct Raindrop {
    glm::vec2 startingPosition = {};
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

struct TerrainLevels {
    float waterLevel = 10.0F;
    float grassLevel = 30.0F;
    float rockLevel = 45.0F;
    float blur = 5.0F;
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
    std::vector<glm::vec3> normals = std::vector<glm::vec3>();

    std::shared_ptr<VertexBuffer> heightBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;

    FastNoise *noise1;
    FastNoise *noise2;
    FastNoise *noise3;

    std::mt19937 randomGenerator;
    std::uniform_real_distribution<double> randomDistribution;

    void renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                       const glm::mat3 &normalMatrix, const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor,
                       float &lightPower, bool wireframe, const TerrainLevels &levels);
    void renderPaths(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                     const std::vector<Raindrop> &raindrops);

    void generateTerrainMesh();

    void setHeightMapValue(int x, int z, float value);
    float getHeightMapValue(int x, int z);
    void simulateRaindrop(const SimulationParams &params, Raindrop &raindrop);
    void regenerateTerrain();
    void regenerateRaindrops(std::vector<Raindrop> &raindrops, bool onlyRainAroundCenterPoint,
                             unsigned int raindropCount, const glm::vec2 &centerPoint, float radius);
    void showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation, glm::vec3 &lightPos,
                      glm::vec3 &lightColor, float &lightPower, bool &wireframe, bool &shouldRenderPaths,
                      bool &onlyRainAroundCenterPoint, bool &letItRain, SimulationParams &params, int &raindropCount,
                      glm::vec2 &centerPoint, float &radius, int &simulationSpeed, TerrainLevels &terrainLevels);
    void recalculateNormals();
};
