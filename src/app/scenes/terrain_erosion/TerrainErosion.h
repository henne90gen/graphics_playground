#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>
#include <random>

#include <FastNoise.h>

#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"
#include "terrain_erosion/RainSimulation.h"

#define HEIGHTMAP_WIDTH 300
#define HEIGHTMAP_HEIGHT 300

struct TerrainLevels {
    float waterLevel = 10.0F;
    float grassLevel = 30.0F;
    float rockLevel = 45.0F;
    float blur = 5.0F;
};

struct TerrainData {
    std::shared_ptr<VertexArray> va = nullptr;
    HeightMap heightMap = {};
    std::vector<glm::vec3> normals = {};

    std::shared_ptr<VertexBuffer> heightBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;

    glm::vec3 pointToLookAt;
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

    TerrainData noiseTerrain;
    TerrainData realTerrain;
    TerrainData *currentTerrain = nullptr;

    FastNoise *noise1;
    FastNoise *noise2;
    FastNoise *noise3;

    std::mt19937 randomGenerator;
    std::uniform_real_distribution<double> randomDistribution;

    void renderTerrain(const TerrainData *terrainData, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                       const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                       const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, float lightPower, bool wireframe,
                       bool drawTriangles, const TerrainLevels &levels);
    void renderPaths(const std::vector<Raindrop> &raindrops, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                     const glm::mat4 &projectionMatrix);

    void generateTerrainMesh(TerrainData &terrainData, const std::vector<glm::vec2> &vertices, HeightMap &heightMap,
                             const std::vector<glm::ivec3> &indices);

    void regenerateNoiseTerrain();
    void regenerateRaindrops(std::vector<Raindrop> &raindrops, bool onlyRainAroundCenterPoint,
                             unsigned int raindropCount, const glm::vec2 &centerPoint, float radius);
    void showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation, glm::vec3 &lightPos,
                      glm::vec3 &lightColor, float &lightPower, bool &wireframe, bool &drawTriangles,
                      bool &shouldRenderPaths, bool &onlyRainAroundCenterPoint, bool &letItRain,
                      SimulationParams &params, int &raindropCount, glm::vec2 &centerPoint, float &radius,
                      int &simulationSpeed, TerrainLevels &terrainLevels);
    void recalculateNormals(TerrainData *terrainData);
    void runSimulation(TerrainData *terrainData, std::vector<Raindrop> &raindrops, bool letItRain,
                       unsigned int simulationSpeed, bool onlyRainAroundCenterPoint, const glm::vec2 &centerPoint,
                       unsigned int raindropCount, float radius, const SimulationParams &params);
    void generateNoiseTerrainData();
    void loadRealTerrain();
};
