#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <random>

#include "gis/XyzLoader.h"
#include "opengl/IndexBuffer.h"
#include "opengl/VertexArray.h"

struct DtmSettings {
    float waterLevel = 0.0F;
    float grassLevel = 25.0F;
    float rockLevel = 45.0F;
    float blur = 6.0F;
};

struct UploadSlice {
    unsigned long startVertex;
    unsigned long endVertex; // exclusive
    unsigned long startIndex;
    unsigned long endIndex; // exclusive
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

    glm::vec3 cameraPositionWorld;

    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::ivec3> indices = {};
    std::unordered_map<long, unsigned int> vertexMap = {};

    unsigned long vertexOffset = 0;
    unsigned long indexOffset = 0;
    unsigned long gpuPointCount = 0;

    BoundingBox3 bb = {};

    std::vector<MissingNormal> missingNormals = {};

    // TODO add tracking information to find out which patch is currently present on the GPU
    // TODO find a way to store points in patches without having to reorganize points all the time
    // TODO maybe use a QuadTree data structure to dynamically adjust partitioning
    // TODO remove normal.y and replace it with a unique id for the file (normals need to be re-normalized anyway and
    //  they always point up)

    float get(int x, int z) const {
        long index = (static_cast<long>(x) << 32) | z;
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

    Dtm dtm;
    std::vector<UploadSlice> uploads = {};
    std::mutex uploadsMutex = {};

    std::shared_ptr<VertexArray> bbVA = nullptr;

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
    void uploadSlices();

    void renderBoundingBox(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const BoundingBox3 &bb);
    void initBoundingBox();
};
