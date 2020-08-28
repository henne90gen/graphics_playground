#pragma once

#include "scenes/Scene.h"

#include <array>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <random>

#include "gis/XyzLoader.h"
#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

// These toggles are for partially implemented features
#define COPY_EDGE_POINTS 0

constexpr unsigned long GPU_BATCH_COUNT = 100;
#if COPY_EDGE_POINTS
constexpr unsigned long GPU_POINTS_PER_BATCH = 10404;
#else
constexpr unsigned long GPU_POINTS_PER_BATCH = 10000;
#endif
constexpr unsigned long GPU_POINT_COUNT = GPU_BATCH_COUNT * GPU_POINTS_PER_BATCH;

struct DtmSettings {
    float waterLevel = 0.0F;
    float grassLevel = 25.0F;
    float rockLevel = 45.0F;
    float blur = 6.0F;
};

struct BatchIndices {
    unsigned long startVertex = 0;
    unsigned long endVertex = 0; // exclusive
    unsigned long startIndex = 0;
    unsigned long endIndex = 0; // exclusive
};

struct Batch {
    unsigned int batchId;
    BatchIndices indices = {};
    BoundingBox3 bb = {};
    std::unordered_map<long, unsigned int> vertexMap = {};
};

struct GpuBatch {
    bool isOccupied = false;
    BatchIndices indices = {};
};

struct MissingNormal {
    unsigned long vertexIndex;
    // true means that the vertex is missing
    std::array<std::pair<unsigned long, bool>, 4> missingVertices = {};
};

struct Dtm {
    std::shared_ptr<VertexArray> va = nullptr;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;

    glm::vec3 cameraPositionWorld = {};

    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::ivec3> indices = {};
    std::unordered_map<long, unsigned int> vertexMap = {};

    unsigned long vertexOffset = 0;
    unsigned long indexOffset = 0;

    BoundingBox3 bb = {};

    std::vector<MissingNormal> missingNormals = {};

    std::vector<Batch> batches = {};
    std::array<GpuBatch, GPU_BATCH_COUNT> gpuMemoryMap = {};

    // TODO maybe use a QuadTree data structure to dynamically adjust partitioning

    float get(int batchId, int x, int z) const {
        const Batch &batch = batches[batchId];
        long index = (static_cast<long>(x) << 32) | z;
        auto localItr = batch.vertexMap.find(index);
        if (localItr != batch.vertexMap.end()) {
            return vertices[localItr->second + batch.indices.startVertex].y;
        }
        auto itr = vertexMap.find(index);
        if (itr == vertexMap.end()) {
            return 0.0F;
        }
        return vertices[itr->second].y;
    }
};

class DtmViewer : public Scene {
  public:
    explicit DtmViewer(SceneData &data) : Scene(data, "DtmViewer"){};
    ~DtmViewer() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> simpleShader;

    Dtm dtm = {};
    std::vector<BatchIndices> uploads = {};
    std::mutex uploadsMutex = {};

    std::shared_ptr<VertexArray> bbVA = nullptr;
    std::array<std::shared_ptr<VertexArray>, GPU_BATCH_COUNT> batchBBVA = {};

    std::future<void> loadDtmFuture;

    void renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                       const glm::mat3 &normalMatrix, const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor,
                       float lightPower, bool wireframe, bool drawTriangles, bool showBatchIds,
                       const DtmSettings &levels);

    void showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation, glm::vec3 &lightPos,
                      glm::vec3 &lightColor, float &lightPower, bool &wireframe, bool &drawTriangles,
                      bool &showBatchIds, DtmSettings &terrainLevels);

    void loadDtm();
    void loadDtmAsync();
    void uploadBatch();
    void uploadBatch(const BatchIndices &batchIndices);

    bool pointExists(Batch &batch, unsigned int &additionalVerticesCount, int x, int z);

    void renderBoundingBox(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const BoundingBox3 &bb);
    void initBoundingBox();
};
