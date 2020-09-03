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
#include "quad_tree/QuadTree.h"

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
    unsigned int batchId = 0;
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

    // the index stored in the quad tree refers to the batch that the vertex belongs to
    QuadTree<unsigned int, 256> quadTree = {};

    std::vector<MissingNormal> missingNormals = {};

    std::vector<Batch> batches = {};
    std::array<GpuBatch, GPU_BATCH_COUNT> gpuMemoryMap = {};

    unsigned int totalFileCount = 0;
    unsigned int loadedFileCount = 0;

    float get(const Batch &batch, int x, int z) const {
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
    std::vector<std::pair<unsigned int, BatchIndices>> uploads = {};
    std::mutex quadTreeMutex = {};

    std::shared_ptr<VertexArray> bbVA = nullptr;

    std::future<void> loadDtmFuture;

    void renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                       const glm::mat3 &normalMatrix, const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor,
                       float lightPower, bool wireframe, bool drawTriangles, bool showBatchIds,
                       const DtmSettings &levels);

    void showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation, glm::vec3 &lightPos,
                      glm::vec3 &lightColor, float &lightPower, bool &wireframe, bool &drawTriangles,
                      bool &showBatchIds, DtmSettings &terrainLevels, int &updateSpeed);

    void loadDtm();
    void loadDtmAsync();
    void uploadBatch();
    void uploadBatch(unsigned int batchId, const BatchIndices &batchIndices);

    bool pointExists(Batch &batch, unsigned int &additionalVerticesCount, int x, int z);

    void renderBoundingBoxes(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                             const std::vector<BoundingBox3> &bbs);
    void initBoundingBox();
};
