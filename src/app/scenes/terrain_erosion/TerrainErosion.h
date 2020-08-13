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
    HeightMap heightMap = {HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, std::vector<float>()};
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

    void regenerateTerrain();
    void regenerateRaindrops(std::vector<Raindrop> &raindrops, bool onlyRainAroundCenterPoint,
                             unsigned int raindropCount, const glm::vec2 &centerPoint, float radius);
    void showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation, glm::vec3 &lightPos,
                      glm::vec3 &lightColor, float &lightPower, bool &wireframe, bool &shouldRenderPaths,
                      bool &onlyRainAroundCenterPoint, bool &letItRain, SimulationParams &params, int &raindropCount,
                      glm::vec2 &centerPoint, float &radius, int &simulationSpeed, TerrainLevels &terrainLevels);
    void recalculateNormals();
};
