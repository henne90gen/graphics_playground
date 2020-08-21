#include "DtmViewer.h"

#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100000.0F;

void DtmViewer::setup() {
    simpleShader = std::make_shared<Shader>("scenes/dtm_viewer/SimpleVert.glsl", "scenes/dtm_viewer/SimpleFrag.glsl");
    shader = std::make_shared<Shader>("scenes/dtm_viewer/TerrainVert.glsl", "scenes/dtm_viewer/TerrainFrag.glsl");

    GL_Call(glPointSize(5));

    initBoundingBox();
    loadDtm();
}

void DtmViewer::destroy() {}

void DtmViewer::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(3.5F, -0.05F, 0.0F);
    static glm::vec3 surfaceToLight = {-4.5F, 7.0F, 0.0F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static float lightPower = 13.0F;
    static bool wireframe = false;
    static bool drawTriangles = true;
    static bool showBatchIds = false;
    static auto terrainSettings = DtmSettings();

    showSettings(modelScale, dtm.cameraPositionWorld, cameraRotation, surfaceToLight, lightColor, lightPower, wireframe,
                 drawTriangles, showBatchIds, terrainSettings);

    uploadSlices();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::vec3 cameraPosition = dtm.cameraPositionWorld;
    cameraPosition *= -1.0F;
    cameraPosition.y *= -1.0F;
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, drawTriangles, showBatchIds, terrainSettings);
    renderBoundingBox(viewMatrix, projectionMatrix, dtm.bb);
}

void DtmViewer::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                             glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                             bool &drawTriangles, bool &showBatchIds, DtmSettings &terrainSettings) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition));
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPos));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Draw Triangles", &drawTriangles);
    ImGui::Checkbox("Show Batch Ids", &showBatchIds);
    ImGui::DragFloat4("Terrain Levels", reinterpret_cast<float *>(&terrainSettings), dragSpeed);
    ImGui::End();
}

void DtmViewer::renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                              const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                              const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, const float lightPower,
                              const bool wireframe, const bool drawTriangles, const bool showBatchIds,
                              const DtmSettings &levels) {
    RECORD_SCOPE_NAME("Render Terrain");

    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("normalMatrix", normalMatrix);

    shader->setUniform("waterLevel", levels.waterLevel);
    shader->setUniform("grassLevel", levels.grassLevel);
    shader->setUniform("rockLevel", levels.rockLevel);
    shader->setUniform("blur", levels.blur);

    shader->setUniform("surfaceToLight", surfaceToLight);
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightPower", lightPower / 100.0F);

    shader->setUniform("showBatchIds", showBatchIds);

    dtm.va->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    if (drawTriangles) {
        GL_Call(glDrawElements(GL_TRIANGLES, dtm.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    } else {
        GL_Call(glDrawElements(GL_POINTS, dtm.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    dtm.va->unbind();
    shader->unbind();
}

void DtmViewer::loadDtm() {
    auto files = getFilesInDirectory("../../../gis_data/dtm");
    auto pointCountEstimate = files.size() * 10000;
    if (pointCountEstimate < 0) {
        std::cout << "Could not count points in dtm" << std::endl;
        return;
    }

    dtm.vertices = std::vector<glm::vec3>(pointCountEstimate);
    dtm.normals = std::vector<glm::vec3>(pointCountEstimate);
    dtm.indices = std::vector<glm::ivec3>(pointCountEstimate * 2);

    dtm.va = std::make_shared<VertexArray>(shader);
    dtm.va->bind();

    dtm.gpuPointCount = 1000000;
    BufferLayout positionLayout = {{ShaderDataType::Float3, "position"}};
    unsigned int vertexSize = dtm.gpuPointCount * sizeof(glm::vec3);
    dtm.vertexBuffer = std::make_shared<VertexBuffer>(nullptr, vertexSize, positionLayout);
    dtm.va->addVertexBuffer(dtm.vertexBuffer);

    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    unsigned int normalSize = dtm.gpuPointCount * sizeof(glm::vec3);
    dtm.normalBuffer = std::make_shared<VertexBuffer>(nullptr, normalSize, normalLayout);
    dtm.va->addVertexBuffer(dtm.normalBuffer);

    unsigned int indexSize = dtm.gpuPointCount * 2 * sizeof(glm::ivec3);
    dtm.indexBuffer = std::make_shared<IndexBuffer>(nullptr, indexSize);
    dtm.va->setIndexBuffer(dtm.indexBuffer);

    std::cout << "Allocated " << vertexSize + normalSize + indexSize << " bytes of GPU memory" << std::endl;

    loadDtmFuture = std::async(std::launch::async, &DtmViewer::loadDtmAsync, this);
}

void DtmViewer::loadDtmAsync() {
    bool success = loadXyzDir("../../../gis_data/dtm", [this](const std::vector<glm::vec3> &points) {
#define GET_INDEX(x, y) dtm.vertexMap[(static_cast<long>(x) << 32) | (y)]
        constexpr float stepWidth = 20.0F;
        const auto vertexOffsetBefore = dtm.vertexOffset;
        const auto indexOffsetBefore = dtm.indexOffset;
        const float batchId = dtm.vertexOffset;

#pragma omp parallel for
        for (unsigned int i = 0; i < points.size(); i++) {
            const auto &vertex = points[i];

            const int x = vertex.x / stepWidth;
            const float y = vertex.y / stepWidth;
            const int z = vertex.z / stepWidth;
            dtm.vertices[dtm.vertexOffset + i] = {x, y, z};
            GET_INDEX(x, z) = dtm.vertexOffset + i;
        }

#pragma omp parallel for
        for (unsigned int i = 0; i < points.size(); i++) {
            const int x = dtm.vertices[dtm.vertexOffset + i].x;
            const int z = dtm.vertices[dtm.vertexOffset + i].z;

            const float L = dtm.get(x - 1, z);
            const float R = dtm.get(x + 1, z);
            const float B = dtm.get(x, z - 1);
            const float T = dtm.get(x, z + 1);
            // TODO record which points have inaccurate normals, because they are at the edge of the patch
            dtm.normals[dtm.vertexOffset + i] = glm::vec3((L - R) / 2.0F, batchId, (B - T) / 2.0F);
        }

        for (unsigned int i = 0; i < points.size(); i++) {
            const int x = dtm.vertices[dtm.vertexOffset + i].x;
            const int z = dtm.vertices[dtm.vertexOffset + i].z;

            const float topH = dtm.get(x, z + 1);
            const float rightH = dtm.get(x + 1, z);
            if (topH != 0.0F && rightH != 0.0F) {
                dtm.indices[dtm.indexOffset++] = glm::ivec3( //
                      GET_INDEX(x, z + 1),                   //
                      GET_INDEX(x + 1, z),                   //
                      GET_INDEX(x, z)                        //
                );
            }
            const float bottomH = dtm.get(x, z - 1);
            const float leftH = dtm.get(x - 1, z);
            if (bottomH != 0.0F && leftH != 0.0F) {
                dtm.indices[dtm.indexOffset++] = glm::ivec3( //
                      GET_INDEX(x - 1, z),                   //
                      GET_INDEX(x, z),                       //
                      GET_INDEX(x, z - 1)                    //
                );
            }
        }

        dtm.vertexOffset += points.size();

        {
            const std::lock_guard<std::mutex> guard(uploadsMutex);
            UploadSlice slice = {vertexOffsetBefore, dtm.vertexOffset, indexOffsetBefore, dtm.indexOffset};
            uploads.push_back(slice);
        }
        std::cout << "Loaded batch of terrain data from disk: " << points.size() << " points" << std::endl;
    });

    if (!success) {
        std::cout << "Could not load dtm" << std::endl;
        return;
    }

    std::cout << "Finished loading dtm" << std::endl;
}

void DtmViewer::uploadSlices() {
    if (!uploadsMutex.try_lock()) {
        return;
    }
    if (uploads.empty()) {
        uploadsMutex.unlock();
        return;
    }

    auto slice = uploads.back();

    if ((slice.endVertex - slice.startVertex) + dtm.vertexOffset > dtm.gpuPointCount) {
        uploadsMutex.unlock();
        return;
    }

    int offset = slice.startVertex * sizeof(glm::vec3);
    size_t size = (slice.endVertex - slice.startVertex) * sizeof(glm::vec3);

    dtm.vertexBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, dtm.vertices.data() + slice.startVertex));

    dtm.normalBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, dtm.normals.data() + slice.startVertex));

    int indexOffset = slice.startIndex * sizeof(glm::ivec3);
    size_t indexSize = (slice.endIndex - slice.startIndex) * sizeof(glm::ivec3);
    dtm.indexBuffer->bind();
    GL_Call(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset, indexSize, dtm.indices.data() + slice.startIndex));

    std::cout << "Uploaded slice to GPU: " << slice.endVertex - slice.startVertex << " vertices, "
              << slice.endIndex - slice.startIndex << " indices" << std::endl;

    std::cout << "Vertices: " << slice.startVertex << " - " << slice.endVertex << " | Indices: " << slice.startIndex
              << " - " << slice.endIndex << std::endl;

#define UPDATE(left, op, right)                                                                                        \
    if (left op right) {                                                                                               \
        right = left;                                                                                                  \
    }
    for (unsigned int i = slice.startVertex; i < slice.endVertex; i++) {
        UPDATE(dtm.vertices[i].x, <, dtm.bb.min.x)
        UPDATE(dtm.vertices[i].y, <, dtm.bb.min.y)
        UPDATE(dtm.vertices[i].z, <, dtm.bb.min.z)

        UPDATE(dtm.vertices[i].x, >, dtm.bb.max.x)
        UPDATE(dtm.vertices[i].y, >, dtm.bb.max.y)
        UPDATE(dtm.vertices[i].z, >, dtm.bb.max.z)
    }

    dtm.cameraPositionWorld = ((dtm.bb.max + dtm.bb.min) / 2.0F) + glm::vec3(0.0F, 50.0F, -200.0F);

    uploads.pop_back();
    uploadsMutex.unlock();
}

void DtmViewer::renderBoundingBox(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                                  const BoundingBox3 &bb) {
    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, (bb.min + bb.max) / 2.0F);
    modelMatrix = glm::scale(modelMatrix, bb.max - bb.min);

    simpleShader->bind();
    simpleShader->setUniform("modelMatrix", modelMatrix);
    simpleShader->setUniform("viewMatrix", viewMatrix);
    simpleShader->setUniform("projectionMatrix", projectionMatrix);

    bbVA->bind();

    GL_Call(glDrawElements(GL_LINE_STRIP, bbVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    bbVA->unbind();
    simpleShader->unbind();
}

void DtmViewer::initBoundingBox() {
    bbVA = std::make_shared<VertexArray>(simpleShader);
    std::vector<glm::vec3> vertices = {
          {-0.5F, -0.5F, -0.5F}, // 0 - - -
          {0.5F, -0.5F, -0.5F},  // 1 + - -
          {-0.5F, 0.5F, -0.5F},  // 2 - + -
          {0.5F, 0.5F, -0.5F},   // 3 + + -
          {-0.5F, -0.5F, 0.5F},  // 4 - - +
          {0.5F, -0.5F, 0.5F},   // 5 + - +
          {-0.5F, 0.5F, 0.5F},   // 6 - + +
          {0.5F, 0.5F, 0.5F},    // 7 + + +
    };

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    bbVA->addVertexBuffer(vb);

    std::vector<unsigned int> indices = {
          0, 1, 5, 4, 0,    // bottom layer
          2, 3, 7, 6, 2,    // top layer
          3, 1, 5, 7, 6, 4, // remaining edges
    };

    auto ib = std::make_shared<IndexBuffer>(indices);
    bbVA->setIndexBuffer(ib);
}
