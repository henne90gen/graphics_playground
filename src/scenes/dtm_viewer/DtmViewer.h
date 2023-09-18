#pragma once

#include "Scene.h"

#include <array>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <random>

#include "DtmDownloader.h"
#include "XyzLoader.h"
#include "gl/IndexBuffer.h"
#include "gl/VertexArray.h"
#include "quad_tree/QuadTree.h"

#define GPU_BATCH_COUNT_CONFIGURABLE 1

constexpr unsigned long GPU_POINTS_PER_BATCH = 10000;

enum class DtmDataSource {
    LOCAL = 0,
    SAXONY = 1,
};

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
    std::string batchName;
    BatchIndices indices = {};
    BoundingBox3 bb = {};
    std::unordered_map<long, unsigned int> vertexMap = {};
};

struct GpuBatch {
    bool isOccupied = false;
    unsigned int batchId = 0;
    BatchIndices indices = {};
};

struct RawBatch {
    std::string batchName;
    std::vector<glm::vec3> points;
};

struct Dtm {
    std::shared_ptr<VertexArray> va = nullptr;
    std::shared_ptr<VertexBuffer> vertexBuffer = nullptr;
    std::shared_ptr<VertexBuffer> normalBuffer = nullptr;
    std::shared_ptr<IndexBuffer> indexBuffer = nullptr;

    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::ivec3> indices = {};

    unsigned long vertexOffset = 0;
    unsigned long indexOffset = 0;

    std::vector<Batch> batches = {};

    BoundingBox3 bb = {};

    // the index stored in the quad tree refers to the batch that the vertex belongs to
    QuadTree<unsigned int> quadTree = {};

    std::vector<GpuBatch> gpuMemoryMap = {};

    float getHeightAt(const Batch &batch, int x, int z) const {
        long index = (static_cast<long>(x) << 32) | z;
        auto localItr = batch.vertexMap.find(index);
        if (localItr != batch.vertexMap.end()) {
            return vertices[localItr->second + batch.indices.startVertex].y;
        }
        return 0.0F;
    }

    void reset(std::shared_ptr<Shader> shader, int pointCountEstimate);
    void initGpuMemory(std::shared_ptr<Shader> shader, const unsigned int gpuBatchCount);
};

class DtmViewer : public Scene {
  public:
    explicit DtmViewer() : Scene("DtmViewer"){};
    ~DtmViewer() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> simpleShader;
    std::unique_ptr<DtmDownloader> downloader;

    DtmDataSource dataSource = DtmDataSource::LOCAL;

    Dtm dtm = {};

    std::mutex dtmMutex = {};
    std::mutex rawBatchMutex = {};
    std::vector<RawBatch> rawBatches = {};

    std::shared_ptr<VertexArray> bbVA = nullptr;

    std::future<void> loadDtmFuture;
    std::future<void> processDtmFuture;

    std::chrono::time_point<std::chrono::high_resolution_clock> startLoading;
    std::chrono::time_point<std::chrono::high_resolution_clock> finishLoading;
    unsigned int loadedFileCount = 0;
    unsigned int totalLoadedFileCount = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> startProcessing;
    std::chrono::time_point<std::chrono::high_resolution_clock> finishProcessing;
    unsigned int processedFileCount = 0;
    unsigned int totalProcessedFileCount = 0;

    void renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                       const glm::mat3 &normalMatrix, const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor,
                       float lightPower, bool wireframe, bool drawTriangles, bool showBatchIds,
                       const DtmSettings &levels, unsigned int gpuBatchCount);

    void showSettings(glm::vec3 &modelScale, glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower,
                      bool &wireframe, bool &drawTriangles, bool &drawBoundingBoxes, bool &showBatchIds,
                      DtmSettings &terrainLevels, int &gpuBatchCount);

    void loadDtm();
    void loadLocalDtm(const std::string &directory);
    void loadSaxonDtm();
    void initGpuMemory(unsigned int gpuBatchCount);
    void loadDtmAsync(const std::string &path);

    void batchProcessor();
    void processBatch(const RawBatch &rawBatch);

    void uploadBatch(unsigned int gpuBatchCount, unsigned int batchId, const BatchIndices &batchIndices);

    static bool pointExists(Batch &batch, int x, int z);

    void initBoundingBox();
    void renderBoundingBoxes(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                             const glm::mat4 &projectionMatrix, const BoundingBox3 &bb,
                             const std::vector<Batch> &batches);

    bool allFilesLoaded() const { return loadedFileCount == totalLoadedFileCount; }
    bool allFilesProcessed() const { return processedFileCount == totalProcessedFileCount; }
};
