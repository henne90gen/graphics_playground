#include "DtmViewer.h"

#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 100000.0F;

constexpr unsigned int DEFAULT_GPU_BATCH_COUNT = 200;

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
    static auto cameraRotation = glm::vec3(4.5F, 0.0F, 0.0F);
    static glm::vec3 surfaceToLight = {-4.5F, 7.0F, 0.0F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static float lightPower = 13.0F;
    static bool wireframe = false;
    static bool drawTriangles = true;
    static bool drawBoundingBoxes = true;
    static bool showBatchIds = false;
    static auto terrainSettings = DtmSettings();
    static int gpuBatchCount = DEFAULT_GPU_BATCH_COUNT;
    static int previousGpuBatchCount = 0;

    showSettings(modelScale, cameraPositionWorld, cameraRotation, surfaceToLight, lightColor, lightPower, wireframe,
                 drawTriangles, drawBoundingBoxes, showBatchIds, terrainSettings, gpuBatchCount);

    if (gpuBatchCount != previousGpuBatchCount) {
        previousGpuBatchCount = gpuBatchCount;
        dtm.gpuMemoryMap = std::vector<GpuBatch>(gpuBatchCount);
        initGpuMemory(gpuBatchCount);
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::vec3 cameraPosition = cameraPositionWorld;
    cameraPosition *= -1.0F;
    cameraPosition.y *= -1.0F;
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    {
        const std::lock_guard<std::mutex> guard(dtmMutex);
        std::vector<unsigned int> closestBatches = {};
        if (dtm.quadTree.get(cameraPositionWorld, gpuBatchCount, closestBatches)) {
            for (unsigned int closestBatch : closestBatches) {
                uploadBatch(gpuBatchCount, closestBatch, dtm.batches[closestBatch].indices);
            }
        }

        if (drawBoundingBoxes) {
            renderBoundingBoxes(viewMatrix, projectionMatrix, dtm.bb, dtm.batches);
        }
    }

    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, drawTriangles, showBatchIds, terrainSettings, gpuBatchCount);
}

void DtmViewer::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                             glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                             bool &drawTriangles, bool &drawBoundingBoxes, bool &showBatchIds,
                             DtmSettings &terrainSettings, int &gpuBatchCount) {
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
    ImGui::Checkbox("Draw Bounding Boxes", &drawBoundingBoxes);
    ImGui::Checkbox("Show Batch Ids", &showBatchIds);
    ImGui::DragFloat4("Terrain Levels", reinterpret_cast<float *>(&terrainSettings), dragSpeed);
    ImGui::SliderInt("GPU Batch Count", &gpuBatchCount, 10, 1000);
    if (ImGui::Button("Reset Camera to Center")) {
        cameraPositionWorld = dtm.bb.center() + glm::vec3(0.0F, 5000.0F, -2000.0F);
    }
    ImGui::Separator();

    unsigned int occupied = 0;
    for (auto &batch : dtm.gpuMemoryMap) {
        if (batch.isOccupied) {
            occupied++;
        }
    }
    ImGui::Text("Occupied GPU memory slots: %d / %lu", occupied, dtm.gpuMemoryMap.size());

    const unsigned int gpuPointCount = gpuBatchCount * GPU_POINTS_PER_BATCH;
    const unsigned int vertexSize = gpuPointCount * sizeof(glm::vec3);
    const unsigned int normalSize = gpuPointCount * sizeof(glm::vec3);
    const unsigned int indexSize = gpuPointCount * 2 * sizeof(glm::ivec3);
    unsigned int gpuMemorySize = vertexSize + normalSize + indexSize;
    float gpuMemorySizeMB = static_cast<float>(gpuMemorySize) / 1024.0F / 1024.0F;
    ImGui::Text("GPU memory usage: %.2fMB", gpuMemorySizeMB);

    {
        auto end = std::chrono::high_resolution_clock::now();
        if (allFilesLoaded()) {
            end = finishLoading;
        }
        auto diff = end - startLoading;
        auto diffNs = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
        auto diffS = diffNs / 1000000000.0F;
        float filesPerSecond = static_cast<float>(loadedFileCount) / diffS;
        ImGui::Text("Loaded:    %4d / %d (%.2f files/s, %.2fs)", loadedFileCount, totalLoadedFileCount, filesPerSecond,
                    diffS);
    }
    {
        auto end = std::chrono::high_resolution_clock::now();
        if (allFilesProcessed()) {
            end = finishProcessing;
        }
        auto diff = end - startProcessing;
        auto diffNs = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
        auto diffS = diffNs / 1000000000.0F;
        float filesPerSecond = static_cast<float>(processedFileCount) / diffS;
        ImGui::Text("Processed: %4d / %d (%.2f files/s, %.2fs)", processedFileCount, totalProcessedFileCount,
                    filesPerSecond, diffS);
    }

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

    // one file is not a .xyz
    totalLoadedFileCount = files.size() - 1;
    totalProcessedFileCount = files.size() - 1;
    loadedFileCount = 0;
    processedFileCount = 0;

    dtm.vertices = std::vector<glm::vec3>(pointCountEstimate);
    dtm.normals = std::vector<glm::vec3>(pointCountEstimate);
    dtm.indices = std::vector<glm::ivec3>(pointCountEstimate * 2);

    initGpuMemory(DEFAULT_GPU_BATCH_COUNT);

    loadDtmFuture = std::async(std::launch::async, &DtmViewer::loadDtmAsync, this);
}

void DtmViewer::loadDtmAsync() {
#define LOOKUP_INDEX(x, y) (static_cast<long>(x) << 32) | (y)

    RECORD_SCOPE();
    startLoading = std::chrono::high_resolution_clock::now();

    bool success =
          loadXyzDir("../../../gis_data/dtm", [this](unsigned int batchName, const std::vector<glm::vec3> &points) {
              RECORD_SCOPE_NAME("Process Points");
              {
                  const std::lock_guard<std::mutex> guard(rawBatchMutex);
                  RawBatch rawBatch = {batchName, points};
                  rawBatches.push_back(rawBatch);
              }

              std::cout << "Loaded batch of terrain data from disk: " << points.size() << " points\n";
              loadedFileCount++;
          });

    if (!success) {
        std::cout << "Could not load DTM" << std::endl;
        return;
    }

    finishLoading = std::chrono::high_resolution_clock::now();
    std::cout << "Finished loading DTM" << std::endl;
}

void DtmViewer::uploadBatch(const unsigned int gpuBatchCount, const unsigned int batchId,
                            const BatchIndices &batchIndices) {
    static unsigned int currentGpuBatchIndex = 0;
    if (batchIndices.startVertex > batchIndices.endVertex || batchIndices.startIndex > batchIndices.endIndex) {
        // TODO find out why this happens
        std::cout << "Failed to upload batch to GPU: vertices " << batchIndices.startVertex << " - "
                  << batchIndices.endVertex << " | " << batchIndices.startIndex << " - " << batchIndices.endIndex
                  << std::endl;
        return;
    }

    for (auto &batch : dtm.gpuMemoryMap) {
        if (!batch.isOccupied) {
            continue;
        }
        if (batch.batchId == batchId) {
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
    auto *tmpIndices = reinterpret_cast<unsigned int *>(std::malloc(indexSize));
    for (unsigned int i = 0; i < triangleCount; i++) {
        tmpIndices[i * 3 + 0] = dtm.indices[batchIndices.startIndex + i].x + pointOffset;
        tmpIndices[i * 3 + 1] = dtm.indices[batchIndices.startIndex + i].y + pointOffset;
        tmpIndices[i * 3 + 2] = dtm.indices[batchIndices.startIndex + i].z + pointOffset;
    }
    dtm.indexBuffer->bind();
    GL_Call(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffsetBytes, indexSize, tmpIndices));
    std::free(tmpIndices);

#if 0
    std::cout << "Uploaded batch to GPU: " << batchIndices.endVertex - batchIndices.startVertex << " vertices ("
              << batchIndices.startVertex << " - " << batchIndices.endVertex << "), " << triangleCount << " triangles ("
              << batchIndices.startIndex << " - " << batchIndices.endIndex << ")" << std::endl;
#endif

    currentGpuBatchIndex++;
    currentGpuBatchIndex %= gpuBatchCount;
}

void DtmViewer::renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                              const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                              const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, const float lightPower,
                              const bool wireframe, const bool drawTriangles, const bool showBatchIds,
                              const DtmSettings &levels, const unsigned int gpuBatchCount) {
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
    auto counts = std::vector<int>(gpuBatchCount);
    auto indices = std::vector<void *>(gpuBatchCount);
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

bool DtmViewer::pointExists(Batch &batch, const int x, const int z) {
    long index = (static_cast<long>(x) << 32) | z;
    auto localItr = batch.vertexMap.find(index);
    return localItr != batch.vertexMap.end();
}

void DtmViewer::batchProcessor() {
    startProcessing = std::chrono::high_resolution_clock::now();

#pragma omp parallel
#pragma omp single
    {
        while (!allFilesProcessed()) {
            if (!rawBatchMutex.try_lock()) {
                continue;
            }
            if (rawBatches.empty()) {
                rawBatchMutex.unlock();
                continue;
            }

            RawBatch rawBatch = rawBatches.back();
            rawBatches.pop_back();

            rawBatchMutex.unlock();

#pragma omp task
            { processBatch(rawBatch); }
        }
#pragma omp taskwait
    }

    finishProcessing = std::chrono::high_resolution_clock::now();
}

void DtmViewer::processBatch(const RawBatch &rawBatch) {
    constexpr float stepWidth = 20.0F;
    Batch batch = {};
    unsigned int vertexOffset = 0;
    unsigned int indexOffset = 0;
    const unsigned int verticesCount = rawBatch.points.size();
    {
        const std::lock_guard<std::mutex> guard(dtmMutex);

        vertexOffset = dtm.vertexOffset;
        indexOffset = dtm.indexOffset;

        dtm.vertexOffset += verticesCount;
        dtm.indexOffset += verticesCount * 2;

        const unsigned int batchId = dtm.batches.size();
        batch.batchId = batchId;
        batch.batchName = rawBatch.batchName;
        batch.indices.startVertex = vertexOffset;
        batch.indices.startIndex = indexOffset;
        batch.indices.endVertex = dtm.vertexOffset;
        batch.indices.endIndex = dtm.indexOffset;
        dtm.batches.push_back(batch);
    }

    ASSERT(verticesCount <= GPU_POINTS_PER_BATCH);

    for (unsigned int i = 0; i < verticesCount; i++) {
        const auto &point = rawBatch.points[i];
        const int x = static_cast<int>(point.x / stepWidth);
        const float y = point.y / stepWidth;
        const int z = static_cast<int>(point.z / stepWidth);

        const glm::vec3 vertex = {x, y, z};
        dtm.vertices[vertexOffset + i] = vertex;

        // could probably be moved to index loop (to enable 'parallel for' for vertices)
        batch.vertexMap[LOOKUP_INDEX(x, z)] = i;
    }

    for (unsigned int i = 0; i < verticesCount; i++) {
        const glm::vec3 &vertex = dtm.vertices[vertexOffset + i];
        const int x = vertex.x;
        const int z = vertex.z;

        auto topRight = glm::ivec3(0);
        if (pointExists(batch, x, z + 1) && //
            pointExists(batch, x + 1, z)) {
            topRight = glm::ivec3(                         //
                  batch.vertexMap[LOOKUP_INDEX(x, z + 1)], //
                  batch.vertexMap[LOOKUP_INDEX(x + 1, z)], //
                  batch.vertexMap[LOOKUP_INDEX(x, z)]      //
            );
        }
        dtm.indices[indexOffset + i * 2] = topRight;

        auto bottomLeft = glm::ivec3(0);
        if (pointExists(batch, x, z - 1) && //
            pointExists(batch, x - 1, z)) {
            bottomLeft = glm::ivec3(                       //
                  batch.vertexMap[LOOKUP_INDEX(x - 1, z)], //
                  batch.vertexMap[LOOKUP_INDEX(x, z)],     //
                  batch.vertexMap[LOOKUP_INDEX(x, z - 1)]  //
            );
        }
        dtm.indices[indexOffset + i * 2 + 1] = bottomLeft;

        batch.bb.update(vertex);
    }

    for (unsigned int i = 0; i < verticesCount; i++) {
        glm::vec3 &vertex = dtm.vertices[vertexOffset + i];
        const int x = vertex.x;
        const float y = vertex.y;
        const int z = vertex.z;

        float L = dtm.getHeightAt(batch, x - 1, z);
        float R = dtm.getHeightAt(batch, x + 1, z);
        float B = dtm.getHeightAt(batch, x, z - 1);
        float T = dtm.getHeightAt(batch, x, z + 1);
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

        dtm.normals[vertexOffset + i] = glm::vec3((L - R) / 2.0F, batch.batchId, (B - T) / 2.0F);
    }

    {
        const std::lock_guard<std::mutex> guard(dtmMutex);
        dtm.bb.update(batch.bb);
        dtm.batches[batch.batchId] = batch;
        dtm.quadTree.insert(batch.bb.center(), batch.batchId);
        processedFileCount++;
    }
}
void DtmViewer::initGpuMemory(const unsigned int gpuBatchCount) {
    const unsigned int gpuPointCount = gpuBatchCount * GPU_POINTS_PER_BATCH;
    const unsigned int vertexSize = gpuPointCount * sizeof(glm::vec3);
    const unsigned int normalSize = gpuPointCount * sizeof(glm::vec3);
    const unsigned int indexSize = gpuPointCount * 2 * sizeof(glm::ivec3);

    dtm.va = std::make_shared<VertexArray>(shader);
    dtm.va->bind();

    BufferLayout positionLayout = {{ShaderDataType::Float3, "position"}};
    dtm.vertexBuffer = std::make_shared<VertexBuffer>(nullptr, vertexSize, positionLayout);
    dtm.va->addVertexBuffer(dtm.vertexBuffer);

    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    dtm.normalBuffer = std::make_shared<VertexBuffer>(nullptr, normalSize, normalLayout);
    dtm.va->addVertexBuffer(dtm.normalBuffer);

    dtm.indexBuffer = std::make_shared<IndexBuffer>(nullptr, indexSize);
    dtm.va->setIndexBuffer(dtm.indexBuffer);
}
