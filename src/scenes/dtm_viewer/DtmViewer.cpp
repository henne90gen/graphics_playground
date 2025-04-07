#include "DtmViewer.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <zip.h>

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/RenderUtils.h"
#include "util/TimeUtils.h"

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 100000.0F;

constexpr uint32_t DEFAULT_GPU_BATCH_COUNT = 200;

constexpr const char *DTM_DIRECTORY_LOCAL = "dtm_viewer_resources/local";
constexpr const char *DTM_DIRECTORY_SAXONY = "dtm_viewer_resources/saxony";

std::array<std::string, 1> SAXONY_DOWNLOAD_URLS = {
      "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33390_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33390_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33390_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33392_5636_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33392_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33392_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33392_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5630_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5632_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5634_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5636_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33394_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5630_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5632_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5634_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5636_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33396_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5632_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5634_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5636_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33398_5644_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5632_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5634_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5636_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5644_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33400_5646_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33402_5638_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33402_5640_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33402_5642_2_sn_xyz.zip",
      // "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33402_5644_2_sn_xyz.zip",
};

DEFINE_SCENE_MAIN(DtmViewer);
DEFINE_DEFAULT_SHADERS(dtm_viewer_Terrain)
DEFINE_DEFAULT_SHADERS(dtm_viewer_Simple)

void DtmViewer::setup() {
    getCamera().setDistance(1000);
    getCamera().setRotation(0.75, 0);

    simpleShader = CREATE_DEFAULT_SHADER(dtm_viewer_Simple);
    shader = CREATE_DEFAULT_SHADER(dtm_viewer_Terrain);
    downloader = std::make_unique<DtmDownloader>();

    GL_Call(glPointSize(5));

    bbVA = createBoundingBoxVA(simpleShader);

    loadDtm();
}

void DtmViewer::destroy() {}

void DtmViewer::tick() {
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
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

    showSettings(modelScale, surfaceToLight, lightColor, lightPower, wireframe, drawTriangles, drawBoundingBoxes,
                 showBatchIds, terrainSettings, gpuBatchCount);

    if (gpuBatchCount != previousGpuBatchCount) {
        previousGpuBatchCount = gpuBatchCount;
        dtm.initGpuMemory(shader, gpuBatchCount);
    }

    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = getCamera().getViewMatrix();
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    {
        const std::lock_guard<std::mutex> guard(dtmMutex);
        std::vector<uint64_t> closestBatches = {};
        if (dtm.quadTree.get(getCamera().getFocalPoint(), gpuBatchCount, closestBatches)) {
            for (const auto closestBatch : closestBatches) {
                uploadBatch(gpuBatchCount, closestBatch, dtm.batches[closestBatch].indices);
            }
        }

        if (drawBoundingBoxes) {
            renderBoundingBoxes(modelMatrix, viewMatrix, projectionMatrix, dtm.bb, dtm.batches);
        }
    }

    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, drawTriangles, showBatchIds, terrainSettings, gpuBatchCount);
}

void DtmViewer::showSettings(glm::vec3 &modelScale, glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower,
                             bool &wireframe, bool &drawTriangles, bool &drawBoundingBoxes, bool &showBatchIds,
                             DtmSettings &terrainSettings, int32_t &gpuBatchCount) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");

    auto previousDataSource = dataSource;
    static const std::array<const char *, 2> items = {"Local", "Saxony"};
    ImGui::Combo("Data Source", reinterpret_cast<int *>(&dataSource), items.data(), items.size());
    if (previousDataSource != dataSource) {
        loadDtm();
    }

    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
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
        getCamera().setFocalPoint(dtm.bb.center());
    }
    ImGui::Separator();

    uint32_t occupied = 0;
    for (auto &batch : dtm.gpuMemoryMap) {
        if (batch.isOccupied) {
            occupied++;
        }
    }
    ImGui::Text("Occupied GPU memory slots: %d / %lu", occupied, dtm.gpuMemoryMap.size());

    const auto gpuPointCount = gpuBatchCount * GPU_POINTS_PER_BATCH;
    const auto vertexSize = gpuPointCount * sizeof(glm::vec3);
    const auto normalSize = gpuPointCount * sizeof(glm::vec3);
    const auto indexSize = gpuPointCount * 2 * sizeof(glm::ivec3);
    const auto gpuMemorySize = vertexSize + normalSize + indexSize;
    float gpuMemorySizeMB = static_cast<float>(gpuMemorySize) / 1024.0F / 1024.0F;
    ImGui::Text("GPU memory usage: %.2fMB", gpuMemorySizeMB);

    {
        auto end = std::chrono::high_resolution_clock::now();
        if (allFilesLoaded()) {
            end = finishLoading;
        }
        const auto diff = end - startLoading;
        const auto diffNs = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
        const auto diffS = diffNs / 1000000000.0F;
        const auto filesPerSecond = static_cast<float>(loadedFileCount) / diffS;
        ImGui::Text("Loaded:    %4lu / %lu (%.2f files/s, %.2fs)", loadedFileCount, totalLoadedFileCount, filesPerSecond,
                    diffS);
    }
    {
        auto end = std::chrono::high_resolution_clock::now();
        if (allFilesProcessed()) {
            end = finishProcessing;
        }
        const auto diff = end - startProcessing;
        const auto diffNs = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
        const auto diffS = diffNs / 1000000000.0F;
        const auto filesPerSecond = static_cast<float>(processedFileCount) / diffS;
        ImGui::Text("Processed: %4lu / %lu (%.2f files/s, %.2fs)", processedFileCount, totalProcessedFileCount,
                    filesPerSecond, diffS);
    }

    ImGui::Text("DTM memory consumption: ...");
    ImGui::End();
}

void DtmViewer::loadDtm() {
    switch (dataSource) {
    case DtmDataSource::LOCAL:
        loadLocalDtm(DTM_DIRECTORY_LOCAL, true);
        break;
    case DtmDataSource::SAXONY:
        loadSaxonyDtm();
        break;
    }

    processDtmFuture = std::async(std::launch::async, &DtmViewer::batchProcessor, this);
}

void DtmViewer::loadSaxonyDtm() {
    totalLoadedFileCount = 1;
    totalProcessedFileCount = 1;
    loadedFileCount = 0;
    processedFileCount = 0;

    dtm.reset(shader, SAXONY_DOWNLOAD_URLS.size() * GPU_POINTS_PER_BATCH);

    loadSaxonyDtmFuture = std::async(std::launch::async, &DtmViewer::loadSaxonyDtmAsync, this);
}

void DtmViewer::loadSaxonyDtmAsync() {
    std::filesystem::create_directories(DTM_DIRECTORY_SAXONY);

    for (const auto &url : SAXONY_DOWNLOAD_URLS) {
        // download zip file
        // TODO parse the url and extract the file name
        auto destinationFilename = std::string("dgm1_33390_5638_2_sn_xyz.zip");
        auto destinationFilepath = std::string(DTM_DIRECTORY_SAXONY) + "/" + destinationFilename;
        if (!std::filesystem::exists(destinationFilepath)) {
            downloader->download(url, destinationFilepath);
        }

        // extract zip file
        auto zipContainerOpt = zip::Container::open_from_file(destinationFilepath);
        if (!zipContainerOpt) {
            std::cerr << "Failed to open downloaded zip file: " << destinationFilepath << std::endl;
            continue;
        }

        auto zipContainer = zipContainerOpt.value();
        zipContainer.extract_to_directory(DTM_DIRECTORY_SAXONY);
    }

    loadLocalDtm(DTM_DIRECTORY_SAXONY, false);
}

void DtmViewer::loadLocalDtm(const std::string &directory, bool shouldResetDtm) {
    auto files = getFilesInDirectory(directory, &isXyzFile);
    auto pointCountEstimate = files.size() * GPU_POINTS_PER_BATCH;
    if (pointCountEstimate < 0) {
        std::cout << "Could not count points in dtm" << std::endl;
        return;
    }

    totalLoadedFileCount = files.size();
    totalProcessedFileCount = files.size();
    loadedFileCount = 0;
    processedFileCount = 0;

    if (shouldResetDtm) {
        dtm.reset(shader, pointCountEstimate);
    }

    loadLocalDtmFuture = std::async(std::launch::async, &DtmViewer::loadLocalDtmAsync, this, directory);
}

void DtmViewer::loadLocalDtmAsync(const std::string &directory) {
    RECORD_SCOPE();
    startLoading = std::chrono::high_resolution_clock::now();

    bool success = loadXyzDir(directory, [this](const std::string &batchName, const std::vector<glm::vec3> &points) {
        RECORD_SCOPE_NAME("Process Points");
        {
            const std::lock_guard<std::mutex> guard(rawBatchMutex);
            RawBatch rawBatch = {batchName, points};
            rawBatches.push_back(rawBatch);
        }

        std::cout << "Loaded batch of terrain data from disk: " << batchName << " with " << points.size()
                  << " points\n";
        loadedFileCount++;
    });

    if (!success) {
        std::cout << "Could not load DTM" << std::endl;
        return;
    }

    finishLoading = std::chrono::high_resolution_clock::now();
    std::cout << "Finished loading DTM" << std::endl;
}

void DtmViewer::uploadBatch(const unsigned int gpuBatchCount, const uint64_t batchId,
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

    const auto pointOffset = currentGpuBatchIndex * GPU_POINTS_PER_BATCH;
    const auto indexOffset = pointOffset * 2;

    const auto offset = pointOffset * sizeof(glm::vec3);
    const auto size = (batchIndices.endVertex - batchIndices.startVertex) * sizeof(glm::vec3);

    dtm.vertexBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, &dtm.vertices[batchIndices.startVertex]));

    dtm.normalBuffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, &dtm.normals[batchIndices.startVertex]));

    const auto indexOffsetBytes = indexOffset * sizeof(glm::ivec3);
    const auto triangleCount = batchIndices.endIndex - batchIndices.startIndex;
    const auto indexSize = triangleCount * sizeof(glm::ivec3);
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
    auto counts = std::vector<int32_t>(gpuBatchCount);
    auto indices = std::vector<void *>(gpuBatchCount);
    unsigned int drawCount = 0;
    for (unsigned int i = 0; i < dtm.gpuMemoryMap.size(); i++) {
        if (!dtm.gpuMemoryMap[i].isOccupied) {
            continue;
        }

        const auto triangleCount = dtm.gpuMemoryMap[i].indices.endIndex - dtm.gpuMemoryMap[i].indices.startIndex;
        counts[drawCount] = (int32_t)(triangleCount * 3);
        const auto indexOffsetInBytes = i * GPU_POINTS_PER_BATCH * 2 * sizeof(glm::ivec3);
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

void DtmViewer::renderBoundingBoxes(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                                    const glm::mat4 &projectionMatrix, const BoundingBox3 &bb,
                                    const std::vector<Batch> &batches) {
    auto bbParams = std::vector<std::pair<glm::vec3, glm::vec3>>(batches.size() + 1);
    bbParams[0] = std::make_pair(bb.center(), bb.max - bb.min);
    for (unsigned int i = 1; i < batches.size() + 1; i++) {
        const auto &batch = batches[i - 1];
        bbParams[i] = std::make_pair(batch.bb.center(), batch.bb.max - batch.bb.min);
    }

    simpleShader->bind();
    simpleShader->setUniform("modelMatrix", modelMatrix);
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
                                    (uint32_t)bbParams.size()));

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
    const auto index = (static_cast<int64_t>(x) << 32) | z;
    const auto localItr = batch.vertexMap.find(index);
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

#if OpenMP_FOUND
#pragma omp task
#endif
            {
                processBatch(rawBatch);
                std::cout << "Processed batch of terrain data: " << rawBatch.batchName << " with "
                          << rawBatch.points.size() << " points" << std::endl;
            }
        }
#if OpenMP_FOUND
#pragma omp taskwait
#endif
    }

    finishProcessing = std::chrono::high_resolution_clock::now();
    std::cout << "Finished processing DTM" << std::endl;
    getCamera().setFocalPoint(dtm.bb.center());
}

#define LOOKUP_INDEX(x, y) (static_cast<int64_t>(x) << 32) | (y)

void DtmViewer::processBatch(const RawBatch &rawBatch) {
    constexpr float stepWidth = 20.0F;
    Batch batch = {};
    uint64_t vertexOffset = 0;
    uint64_t indexOffset = 0;
    const auto verticesCount = rawBatch.points.size();
    {
        const std::lock_guard<std::mutex> guard(dtmMutex);

        vertexOffset = dtm.vertexOffset;
        indexOffset = dtm.indexOffset;

        dtm.vertexOffset += verticesCount;
        dtm.indexOffset += verticesCount * 2;

        const auto batchId = dtm.batches.size();
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
        const auto x = (int)vertex.x;
        const auto z = (int)vertex.z;

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
        const auto x = (int)vertex.x;
        const auto y = vertex.y;
        const auto z = (int)vertex.z;

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

void Dtm::initGpuMemory(std::shared_ptr<Shader> shader, const size_t gpuBatchCount) {
    gpuMemoryMap = std::vector<GpuBatch>(gpuBatchCount);

    const auto gpuPointCount = gpuBatchCount * GPU_POINTS_PER_BATCH;
    const auto vertexSize = gpuPointCount * sizeof(glm::vec3);
    const auto normalSize = gpuPointCount * sizeof(glm::vec3);
    const auto indexSize = gpuPointCount * 2 * sizeof(glm::ivec3);

    va = std::make_shared<VertexArray>(shader);
    va->bind();

    BufferLayout positionLayout = {{ShaderDataType::Float3, "position"}};
    vertexBuffer = std::make_shared<VertexBuffer>(nullptr, vertexSize, positionLayout);
    va->addVertexBuffer(vertexBuffer);

    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    normalBuffer = std::make_shared<VertexBuffer>(nullptr, normalSize, normalLayout);
    va->addVertexBuffer(normalBuffer);

    indexBuffer = std::make_shared<IndexBuffer>(nullptr, indexSize);
    va->setIndexBuffer(indexBuffer);
}

void Dtm::reset(std::shared_ptr<Shader> shader, const size_t pointCountEstimate) {
    batches = {};
    bb = {};
    quadTree = {};
    gpuMemoryMap = {};

    vertexOffset = 0;
    indexOffset = 0;

    vertices = std::vector<glm::vec3>(pointCountEstimate);
    normals = std::vector<glm::vec3>(pointCountEstimate);
    indices = std::vector<glm::ivec3>(pointCountEstimate * 2);

    initGpuMemory(shader, DEFAULT_GPU_BATCH_COUNT);
}
