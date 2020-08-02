#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include <FastNoise.h>

#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

struct Path {
    std::vector<glm::vec3> vertices = {};
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
    void renderPaths(const std::vector<Path> &paths);

    void generatePoints();

    void updateHeightBuffer();
    void simulateRainDrop(std::vector<glm::vec3> &path, const glm::vec3 &start);
};
