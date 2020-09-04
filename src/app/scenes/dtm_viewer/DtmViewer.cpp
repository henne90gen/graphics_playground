#include "DtmViewer.h"

#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 100000.0F;

void DtmViewer::setup() {
    simpleShader = std::make_shared<Shader>("scenes/dtm_viewer/SimpleVert.glsl", "scenes/dtm_viewer/SimpleFrag.glsl");
    shader = std::make_shared<Shader>("scenes/dtm_viewer/TerrainVert.glsl", "scenes/dtm_viewer/TerrainFrag.glsl");

    GL_Call(glPointSize(5));

    initBoundingBox();

    loadDtm();
    processDtmFuture = std::async(std::launch::async, &DtmViewer::batchProcessor, this);
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

    showSettings(modelScale, cameraPositionWorld, cameraRotation, surfaceToLight, lightColor, lightPower, wireframe,
                 drawTriangles, showBatchIds, terrainSettings);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::vec3 cameraPosition = cameraPositionWorld;
    cameraPosition *= -1.0F;
    cameraPosition.y *= -1.0F;
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    {
        const std::lock_guard<std::mutex> guard(quadTreeMutex);
        std::vector<unsigned int> closestBatches = {};
        if (dtm.quadTree.get(cameraPositionWorld, 100, closestBatches)) {
            for (unsigned int i = 0; i < closestBatches.size(); i++) {
                uploadBatch(closestBatches[i], dtm.batches[closestBatches[i]].indices);
            }
        }

        renderBoundingBoxes(viewMatrix, projectionMatrix, dtm.bb, dtm.batches);
    }

    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, drawTriangles, showBatchIds, terrainSettings);
}

void DtmViewer::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                             glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                             bool &drawTriangles, bool &showBatchIds, DtmSettings &terrainSettings) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 10.0F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPos));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Draw Triangles", &drawTriangles);
    ImGui::Checkbox("Show Batch Ids", &showBatchIds);
    ImGui::DragFloat4("Terrain Levels", reinterpret_cast<float *>(&terrainSettings), dragSpeed);
    if (ImGui::Button("Reset Camera to Center")) {
        cameraPositionWorld = ((dtm.bb.max + dtm.bb.min) / 2.0F) + glm::vec3(0.0F, 500.0F, -2000.0F);
    }
    ImGui::Separator();

    unsigned int occupied = 0;
    for (unsigned int i = 0; i < dtm.gpuMemoryMap.size(); i++) {
        if (dtm.gpuMemoryMap[i].isOccupied) {
            occupied++;
        }
    }
    ImGui::Text("Occupied GPU memory slots: %d / %lu", occupied, dtm.gpuMemoryMap.size());

    auto end = std::chrono::high_resolution_clock::now();
    if (loadedFileCount == totalLoadedFileCount) {
        end = finishLoading;
    }
    auto diff = end - startLoading;
    auto diffNs = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
    auto diffS = diffNs / 1000000000.0F;
    float filesPerSecond = static_cast<float>(loadedFileCount) / diffS;
    ImGui::Text("Loaded files: %4d / %d (%.2f files/s)", loadedFileCount, totalLoadedFileCount, filesPerSecond);

    ImGui::Text("Processed files: %4d / %d (%.2f files/s)", processedFileCount, totalProcessedFileCount,
                filesPerSecond);

    ImGui::Text("DTM memory consumption: ...");
    ImGui::End();
}

void DtmViewer::loadDtm() {
    auto files = getFilesInDirectory("../../../gis_data/dtm");
    auto pointCountEstimate = files.size() * GPU_POINTS_PER_BATCH;
    if (pointCountEstimate < 0) {
        std::cout << "Could not count points in dtm" << std::endl;
        return;
    }

    totalLoadedFileCount = files.size();
    loadedFileCount = 0;

    dtm.vertices = std::vector<glm::vec3>(pointCountEstimate);
    dtm.normals = std::vector<glm::vec3>(pointCountEstimate);
    dtm.indices = std::vector<glm::ivec3>(pointCountEstimate * 2);

    dtm.va = std::make_shared<VertexArray>(shader);
    dtm.va->bind();

    BufferLayout positionLayout = {{ShaderDataType::Float3, "position"}};
    unsigned int vertexSize = GPU_POINT_COUNT * sizeof(glm::vec3);
    dtm.vertexBuffer = std::make_shared<VertexBuffer>(nullptr, vertexSize, positionLayout);
    dtm.va->addVertexBuffer(dtm.vertexBuffer);

    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    unsigned int normalSize = GPU_POINT_COUNT * sizeof(glm::vec3);
    dtm.normalBuffer = std::make_shared<VertexBuffer>(nullptr, normalSize, normalLayout);
    dtm.va->addVertexBuffer(dtm.normalBuffer);

    unsigned int indexSize = GPU_POINT_COUNT * 2 * sizeof(glm::ivec3);
    dtm.indexBuffer = std::make_shared<IndexBuffer>(nullptr, indexSize);
    dtm.va->setIndexBuffer(dtm.indexBuffer);

    std::cout << "Allocated " << vertexSize + normalSize + indexSize << " bytes of GPU memory" << std::endl;

    loadDtmFuture = std::async(std::launch::async, &DtmViewer::loadDtmAsync, this);
}

bool DtmViewer::pointExists(Batch &batch, unsigned int &additionalVerticesCount, const int x, const int z) {
    long index = (static_cast<long>(x) << 32) | z;
    auto localItr = batch.vertexMap.find(index);
    if (localItr != batch.vertexMap.end()) {
        return true;
    }

    auto itr = dtm.vertexMap.find(index);
    if (itr == dtm.vertexMap.end()) {
        return false;
    }

#if COPY_EDGE_POINTS
    // copy point into batch
    unsigned long vertexIndex = dtm.vertexOffset + additionalVerticesCount;
    dtm.vertices[vertexIndex] = dtm.vertices[itr->second];
    batch.vertexMap[index] = vertexIndex - batch.indices.startVertex;
    additionalVerticesCount++;
    return true;
#else
    return false;
#endif
}

// Idea: loadFiles --> raw vertex data --> queue (pointer to data) --> process raw data into mesh --> upload to GPU as
// needed
void DtmViewer::loadDtmAsync() {
#define LOOKUP_INDEX(x, y) (static_cast<long>(x) << 32) | (y)

    RECORD_SCOPE();
    startLoading = std::chrono::high_resolution_clock::now();

    bool success =
          loadXyzDir("../../../gis_data/dtm", [this](unsigned int batchName, const std::vector<glm::vec3> &points) {
              RECORD_SCOPE_NAME("Process Points");

#if 1
              constexpr float stepWidth = 20.0F;
              const unsigned int batchId = dtm.batches.size();
              Batch batch = {batchId, batchName};
              batch.indices.startVertex = dtm.vertexOffset;
              batch.indices.startIndex = dtm.indexOffset;

              ASSERT(points.size() <= GPU_POINTS_PER_BATCH);

#pragma omp parallel for
              for (unsigned int i = 0; i < points.size(); i++) {
                  const auto &point = points[i];
                  const int x = point.x / stepWidth;
                  const float y = point.y / stepWidth;
                  const int z = point.z / stepWidth;

                  const glm::vec3 vertex = {x, y, z};
                  dtm.vertices[dtm.vertexOffset + i] = vertex;
                  dtm.vertexMap[LOOKUP_INDEX(x, z)] = dtm.vertexOffset + i;
                  batch.vertexMap[LOOKUP_INDEX(x, z)] = i;
              }

              unsigned int verticesCount = points.size();
              for (unsigned int i = 0; i < points.size(); i++) {
                  const glm::vec3 &vertex = dtm.vertices[dtm.vertexOffset + i];
                  const int x = vertex.x;
                  const int z = vertex.z;

                  // look up a point in the local vertex map
                  //   point found   -> all good, use it
                  //   point missing -> look up a point in the global vertex map
                  //     point found   -> copy it into the local batch, and use that index
                  //     point missing -> add it to the missing points

                  if (pointExists(batch, verticesCount, x, z + 1) && //
                      pointExists(batch, verticesCount, x + 1, z)) {
                      dtm.indices[dtm.indexOffset++] = glm::ivec3(   //
                            batch.vertexMap[LOOKUP_INDEX(x, z + 1)], //
                            batch.vertexMap[LOOKUP_INDEX(x + 1, z)], //
                            batch.vertexMap[LOOKUP_INDEX(x, z)]      //
                      );
                  }
                  if (pointExists(batch, verticesCount, x, z - 1) && //
                      pointExists(batch, verticesCount, x - 1, z)) {
                      dtm.indices[dtm.indexOffset++] = glm::ivec3(   //
                            batch.vertexMap[LOOKUP_INDEX(x - 1, z)], //
                            batch.vertexMap[LOOKUP_INDEX(x, z)],     //
                            batch.vertexMap[LOOKUP_INDEX(x, z - 1)]  //
                      );
                  }

                  batch.bb.update(vertex);
              }

              dtm.bb.update(batch.bb);

#pragma omp parallel for
              for (unsigned int i = 0; i < verticesCount; i++) {
                  const int x = dtm.vertices[dtm.vertexOffset + i].x;
                  const float y = dtm.vertices[dtm.vertexOffset + i].y;
                  const int z = dtm.vertices[dtm.vertexOffset + i].z;

                  float L = dtm.get(batch, x - 1, z);
                  float R = dtm.get(batch, x + 1, z);
                  float B = dtm.get(batch, x, z - 1);
                  float T = dtm.get(batch, x, z + 1);
                  if (L == 0.0F) {
                      L = y;
                  }
                  if (R == 0.0F) {
                      R = y;
                  }
                  if (B == 0.0F) {
                      B = y;
                  }
                  if (T == 0.0F) {
                      T = y;
                  }
#if 0
                        if (L == 0.0F || R == 0.0F || B == 0.0F || T == 0.0F) {
#pragma omp critical
                            {
                                std::array<std::pair<unsigned long, bool>, 4> missingVertices = {
                                      std::make_pair(GET_INDEX(x - 1, z), L == 0.0F), //
                                      std::make_pair(GET_INDEX(x + 1, z), R == 0.0F), //
                                      std::make_pair(GET_INDEX(x, z - 1), B == 0.0F), //
                                      std::make_pair(GET_INDEX(x, z + 1), T == 0.0F)  //
                                };
                                dtm.missingNormals.push_back({dtm.vertexOffset + i, missingVertices});
                            }
                        }
#endif

                  dtm.normals[dtm.vertexOffset + i] = glm::vec3((L - R) / 2.0F, batchId, (B - T) / 2.0F);
              }

              dtm.vertexOffset += verticesCount;
              batch.indices.endVertex = dtm.vertexOffset;
              batch.indices.endIndex = dtm.indexOffset;

              {
                  const std::lock_guard<std::mutex> guard(quadTreeMutex);
                  dtm.batches.push_back(batch);
                  dtm.quadTree.insert(batch.bb.center(), batchId);
              }
#endif
              std::cout << "Loaded batch of terrain data from disk: " << points.size() << " points\n";
              loadedFileCount++;
          });

    if (!success) {
        std::cout << "Could not load dtm" << std::endl;
        return;
    }

    finishLoading = std::chrono::high_resolution_clock::now();
    // make sure file count is set correctly (prevents off-by-one errors, caused by )
    loadedFileCount = totalLoadedFileCount;
    std::cout << "Finished loading dtm" << std::endl;
}

void DtmViewer::uploadBatch(const unsigned int batchId, const BatchIndices &batchIndices) {
    static unsigned int currentGpuBatchIndex = 0;
    if (batchIndices.startVertex > batchIndices.endVertex || batchIndices.startIndex > batchIndices.endIndex) {
        // TODO find out why this happens
        std::cout << "Failed to upload batch to GPU: vertices " << batchIndices.startVertex << " - "
                  << batchIndices.endVertex << " | " << batchIndices.startIndex << " - " << batchIndices.endIndex
                  << std::endl;
        return;
    }

    for (unsigned int i = 0; i < dtm.gpuMemoryMap.size(); i++) {
        if (!dtm.gpuMemoryMap[i].isOccupied) {
            continue;
        }
        if (dtm.gpuMemoryMap[i].batchId == batchId) {
            return;
        }
    }

    dtm.gpuMemoryMap[currentGpuBatchIndex] = {true, batchId, batchIndices};

    unsigned int pointOffset = currentGpuBatchIndex * GPU_POINTS_PER_BATCH;
    unsigned int indexOffset = pointOffset * 2;

    int offset = pointOffset * sizeof(glm::vec3);
    size_t size = (batchIndices.endVertex - batchIndices.startVertex) * sizeof(glm::vec3);

    dtm.vertexBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, &dtm.vertices[batchIndices.startVertex]));

    dtm.normalBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, &dtm.normals[batchIndices.startVertex]));

    unsigned int indexOffsetBytes = indexOffset * sizeof(glm::ivec3);
    unsigned long triangleCount = batchIndices.endIndex - batchIndices.startIndex;
    size_t indexSize = triangleCount * sizeof(glm::ivec3);
    unsigned int *tmpIndices = reinterpret_cast<unsigned int *>(std::malloc(indexSize));
    for (unsigned int i = 0; i < triangleCount; i++) {
        tmpIndices[i * 3 + 0] = dtm.indices[batchIndices.startIndex + i].x + pointOffset;
        tmpIndices[i * 3 + 1] = dtm.indices[batchIndices.startIndex + i].y + pointOffset;
        tmpIndices[i * 3 + 2] = dtm.indices[batchIndices.startIndex + i].z + pointOffset;
    }
    dtm.indexBuffer->bind();
    GL_Call(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexSize, tmpIndices));
    std::free(tmpIndices);

    std::cout << "Uploaded batch to GPU: " << batchIndices.endVertex - batchIndices.startVertex << " vertices ("
              << batchIndices.startVertex << " - " << batchIndices.endVertex << "), " << triangleCount << " triangles ("
              << batchIndices.startIndex << " - " << batchIndices.endIndex << ")" << std::endl;

    currentGpuBatchIndex++;
    currentGpuBatchIndex %= GPU_BATCH_COUNT;
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

    unsigned int mode = drawTriangles ? GL_TRIANGLES : GL_POINTS;
    std::array<int, GPU_BATCH_COUNT> counts = {};
    std::array<void *, GPU_BATCH_COUNT> indices = {};
    unsigned int drawCount = 0;
    for (unsigned int i = 0; i < dtm.gpuMemoryMap.size(); i++) {
        if (!dtm.gpuMemoryMap[i].isOccupied) {
            continue;
        }

        unsigned long triangleCount = dtm.gpuMemoryMap[i].indices.endIndex - dtm.gpuMemoryMap[i].indices.startIndex;
        counts[drawCount] = triangleCount * 3;
        unsigned long indexOffsetInBytes = i * GPU_POINTS_PER_BATCH * 2 * sizeof(glm::ivec3);
        indices[drawCount] = reinterpret_cast<void *>(indexOffsetInBytes);
        drawCount++;
    }
    GL_Call(glMultiDrawElements(mode, counts.data(), GL_UNSIGNED_INT, indices.data(), drawCount));

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    dtm.va->unbind();
    shader->unbind();
}

void DtmViewer::renderBoundingBoxes(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                                    const BoundingBox3 &bb, const std::vector<Batch> &batches) {
    auto bbParams = std::vector<std::pair<glm::vec3, glm::vec3>>(batches.size() + 1);
    bbParams[0] = std::make_pair(bb.center(), bb.max - bb.min);
#pragma omp parallel for
    for (unsigned int i = 1; i < batches.size() + 1; i++) {
        const auto &batch = batches[i - 1];
        bbParams[i] = std::make_pair(batch.bb.center(), batch.bb.max - batch.bb.min);
    }

    simpleShader->bind();
    simpleShader->setUniform("viewMatrix", viewMatrix);
    simpleShader->setUniform("projectionMatrix", projectionMatrix);

    bbVA->bind();
    static unsigned int instanceVBO = 0;
    if (instanceVBO == 0) {
        GL_Call(glGenBuffers(1, &instanceVBO));
    }
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, instanceVBO));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(std::pair<glm::vec3, glm::vec3>) * bbParams.size(), bbParams.data(),
                         GL_STATIC_DRAW));

    GL_Call(glEnableVertexAttribArray(1));
    GL_Call(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void *)0));
    GL_Call(glVertexAttribDivisor(1, 1));

    GL_Call(glEnableVertexAttribArray(2));
    GL_Call(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void *)(sizeof(glm::vec3))));
    GL_Call(glVertexAttribDivisor(2, 1));

    GL_Call(glDrawElementsInstanced(GL_LINE_STRIP, bbVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
                                    bbParams.size()));

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

void DtmViewer::batchProcessor() {

}
