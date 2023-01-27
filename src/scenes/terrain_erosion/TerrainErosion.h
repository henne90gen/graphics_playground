#pragma once

#include "Scene.h"

#include <functional>
#include <memory>
#include <random>

#include <FastNoiseLite.h>

#include "RainSimulation.h"
#include "gl/IndexBuffer.h"
#include "gl/VertexArray.h"

struct TerrainLevels {
    float waterLevel = 0.0F;
    float grassLevel = 25.0F;
    float rockLevel = 45.0F;
    float blur = 6.0F;
};

struct TerrainData {
    HeightMap heightMap = {};
    std::unordered_map<int64_t, float> originalHeightData = {};

    std::shared_ptr<VertexArray> va;
    std::shared_ptr<VertexBuffer> heightBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;

    glm::vec3 pointToLookAt;
};

class TerrainErosion : public Scene {
  public:
    explicit TerrainErosion() : Scene("TerrainErosion"){};
    ~TerrainErosion() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> pathShader;

    TerrainData noiseTerrain;

    FastNoiseLite *noise1;
    FastNoiseLite *noise2;
    FastNoiseLite *noise3;
    std::mt19937 randomGenerator;
    std::uniform_real_distribution<double> randomDistribution;

    void renderTerrain(const TerrainData &terrainData, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                       const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                       const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, float lightPower, bool wireframe,
                       bool drawTriangles, int verticesPerFrame, const TerrainLevels &levels);
    void renderPaths(const std::vector<Raindrop> &raindrops, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                     const glm::mat4 &projectionMatrix);

    void initTerrainMesh(TerrainData &terrainData, const std::vector<glm::vec2> &vertices,
                         const std::vector<glm::ivec3> &indices);

    void regenerateNoiseTerrain();
    void regenerateRaindrops(const glm::ivec2 &terrainSize, std::vector<Raindrop> &raindrops,
                             bool onlyRainAroundCenterPoint, unsigned int raindropCount, const glm::vec2 &centerPoint,
                             float radius);
    void showSettings(glm::vec3 &modelScale, glm::vec3 &modelRotation, glm::vec3 &cameraPosition,
                      glm::vec3 &cameraRotation, glm::vec3 &surfaceToLight, glm::vec3 &lightColor, float &lightPower,
                      bool &wireframe, bool &drawTriangles, int &verticesPerFrame, bool &shouldRenderPaths,
                      bool &onlyRainAroundCenterPoint, bool &letItRain, SimulationParams &params, int &raindropCount,
                      glm::vec2 &centerPoint, float &radius, TerrainLevels &terrainLevels, glm::ivec2 &terrainSize);
    void recalculateNormals(TerrainData &terrainData, int verticesPerFrame);
    void runSimulation(const glm::ivec2 &terrainSize, TerrainData &terrainData, std::vector<Raindrop> &raindrops,
                       bool onlyRainAroundCenterPoint, const glm::vec2 &centerPoint, unsigned int raindropCount,
                       float radius, const SimulationParams &params);
    void generateNoiseTerrainData(const glm::ivec2 &terrainSize);
};
