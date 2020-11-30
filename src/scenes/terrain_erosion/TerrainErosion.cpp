#include "TerrainErosion.h"

#include <chrono>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100000.0F;

DEFINE_SHADER(terrain_erosion_Terrain)
DEFINE_SHADER(terrain_erosion_Path)

void TerrainErosion::setup() {
    std::random_device global_random_device;
    randomGenerator = std::mt19937(global_random_device());
    randomDistribution = std::uniform_real_distribution<double>(0.0, 1.0);

    GL_Call(glEnable(GL_PRIMITIVE_RESTART));
    GL_Call(glPrimitiveRestartIndex(~0));

    pathShader = SHADER(terrain_erosion_Path);

    shader = SHADER(terrain_erosion_Terrain);
    shader->bind();

    noise1 = new FastNoise();
    noise2 = new FastNoise();
    noise3 = new FastNoise();

    noise1->SetFrequency(0.1F);
    noise2->SetFrequency(0.3F);
    noise3->SetFrequency(0.7F);

    FastNoise::NoiseType noiseType = FastNoise::Simplex;
    noise1->SetNoiseType(noiseType);
    noise2->SetNoiseType(noiseType);
    noise3->SetNoiseType(noiseType);

    generateNoiseTerrainData();
}

void TerrainErosion::destroy() {}

void TerrainErosion::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraPosition = glm::vec3(-120.0F, -155.0F, -375.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.6F, -0.05F, 0.0F);
    static glm::vec3 surfaceToLight = {-4.5F, 7.0F, 0.0F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static float lightPower = 13.0F;
    static bool wireframe = false;
    static bool drawTriangles = true;
    static int verticesPerFrame = 15000;
    static bool shouldRenderPaths = false;
    static auto terrainLevels = TerrainLevels();
    static bool onlyRainAroundCenterPoint = false;
    static bool letItRain = false;
    static auto params = SimulationParams();
    static int raindropCount = 100;
    static auto centerPoint = glm::vec2(HEIGHTMAP_WIDTH / 2, HEIGHTMAP_HEIGHT / 2);
    static auto radius = 30.0F;
    static int simulationSpeed = 2;
    static auto raindrops = std::vector<Raindrop>();
    static bool pathsInitialized = false;
    if (!pathsInitialized) {
        pathsInitialized = true;
        regenerateRaindrops(raindrops, onlyRainAroundCenterPoint, raindropCount, centerPoint, radius);
    }

    showSettings(modelScale, cameraPosition, cameraRotation, surfaceToLight, lightColor, lightPower, wireframe,
                 drawTriangles, verticesPerFrame, shouldRenderPaths, onlyRainAroundCenterPoint, letItRain, params,
                 raindropCount, centerPoint, radius, simulationSpeed, terrainLevels);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    runSimulation(noiseTerrain, raindrops, letItRain, simulationSpeed, onlyRainAroundCenterPoint, centerPoint,
                  raindropCount, radius, params);

    if (letItRain && shouldRenderPaths) {
        renderPaths(raindrops, modelMatrix, viewMatrix, projectionMatrix);
    }

    recalculateNormals(noiseTerrain, verticesPerFrame);
    renderTerrain(noiseTerrain, modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor,
                  lightPower, wireframe, drawTriangles, verticesPerFrame, terrainLevels);
}

void TerrainErosion::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                                  glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                                  bool &drawTriangles, int &verticesPerFrame, bool &shouldRenderPaths,
                                  bool &onlyRainAroundCenterPoint, bool &letItRain, SimulationParams &params,
                                  int &raindropCount, glm::vec2 &centerPoint, float &radius, int &simulationSpeed,
                                  TerrainLevels &terrainLevels) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);

    cameraPosition.x *= -1.0F;
    cameraPosition.z *= -1.0F;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition));
    cameraPosition.x *= -1.0F;
    cameraPosition.z *= -1.0F;

    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPos));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Draw Triangles", &drawTriangles);
    ImGui::SliderInt("Vertices Per Frame", &verticesPerFrame, 10, 100000);
    ImGui::Checkbox("Raindrop Paths", &shouldRenderPaths);
    if (ImGui::Button("Regenerate Terrain")) {
        regenerateNoiseTerrain();
    }
    ImGui::DragFloat4("Terrain Levels", reinterpret_cast<float *>(&terrainLevels), dragSpeed);

    ImGui::Separator();

    ImGui::Text("Simulation Parameters");
    if (letItRain) {
        if (ImGui::Button("Stop Simulation")) {
            letItRain = false;
        }
    } else {
        if (ImGui::Button("Start Simulation")) {
            letItRain = true;
        }
    }
    ImGui::SliderInt("Simulation Speed", &simulationSpeed, 1, 50);
    ImGui::DragInt("Raindrop Count", &raindropCount);
    bool before = onlyRainAroundCenterPoint;
    ImGui::Checkbox("Only Simulate around Point", &onlyRainAroundCenterPoint);
    if (before != onlyRainAroundCenterPoint) {
        centerPoint = glm::vec2(-cameraPosition.x, -cameraPosition.z);
    }
    if (onlyRainAroundCenterPoint) {
        ImGui::DragFloat2("Center Point", reinterpret_cast<float *>(&centerPoint), dragSpeed);
        ImGui::DragFloat("Radius", &radius, dragSpeed);
    }
    ImGui::DragFloat("Kq", &params.Kq, dragSpeed);
    ImGui::DragFloat("Kw", &params.Kw, dragSpeed);
    ImGui::DragFloat("Kr", &params.Kr, dragSpeed);
    ImGui::DragFloat("Kd", &params.Kd, dragSpeed);
    ImGui::DragFloat("Ki", &params.Ki, dragSpeed);
    ImGui::DragFloat("minSlope", &params.minSlope, dragSpeed);
    ImGui::DragFloat("g", &params.g, dragSpeed);
    if (ImGui::Button("Reset Parameters")) {
        params = SimulationParams();
    }

    ImGui::End();
}

void TerrainErosion::generateNoiseTerrainData() {
    const unsigned int verticesCount = HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT;
    auto vertices = std::vector<glm::vec2>(verticesCount);
    auto grid = std::vector<glm::ivec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        int x = i % HEIGHTMAP_WIDTH;
        int z = floor(static_cast<float>(i) / static_cast<float>(HEIGHTMAP_HEIGHT));
        vertices[i] = glm::vec2(x, z);
        grid[i] = glm::ivec2(x, z);
    }

    const unsigned int indicesPerQuad = 6;
    unsigned int indicesCount = HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT * indicesPerQuad;
    auto indices = std::vector<glm::ivec3>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < HEIGHTMAP_HEIGHT - 1; y++) {
        for (unsigned int x = 0; x < HEIGHTMAP_WIDTH - 1; x++) {
            indices[counter++] = {
                  (y + 1) * HEIGHTMAP_WIDTH + x, //
                  y * HEIGHTMAP_WIDTH + (x + 1), //
                  y * HEIGHTMAP_WIDTH + x        //
            };
            indices[counter++] = {
                  (y + 1) * HEIGHTMAP_WIDTH + x,       //
                  (y + 1) * HEIGHTMAP_WIDTH + (x + 1), //
                  y * HEIGHTMAP_WIDTH + (x + 1)        //
            };
        }
    }
    HeightMap heightMap = {grid};
    initTerrainMesh(noiseTerrain, vertices, heightMap, indices);

    regenerateNoiseTerrain();
}

void TerrainErosion::regenerateRaindrops(std::vector<Raindrop> &raindrops, bool onlyRainAroundCenterPoint,
                                         const unsigned int raindropCount, const glm::vec2 &centerPoint,
                                         const float radius) {
    raindrops.clear();
    raindrops.reserve(raindropCount);
    if (onlyRainAroundCenterPoint) {
        for (unsigned int j = 0; j < raindropCount; j++) {
            double x = randomDistribution(randomGenerator);
            double y = randomDistribution(randomGenerator);
            auto p = glm::vec2(x, y);
            p *= radius;
            Raindrop raindrop = {};
            raindrop.startingPosition = centerPoint + p;
            raindrops.push_back(raindrop);
        }
    } else {
        for (unsigned int j = 0; j < raindropCount; j++) {
            double num = randomDistribution(randomGenerator);
            auto i = static_cast<unsigned int>(num * (HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT));
            auto x = static_cast<float>(i % HEIGHTMAP_WIDTH);
            auto z = std::floor(static_cast<float>(i) / static_cast<float>(HEIGHTMAP_WIDTH));
            Raindrop raindrop = {};
            raindrop.startingPosition = glm::vec2(x, z);
            raindrops.push_back(raindrop);
        }
    }
}

float generateHeight(const FastNoise *noise, const float x, const float y, const float z) {
    float generatedHeight = noise->GetNoise(x, y, z);
    const float offset = 1.0F;
    generatedHeight += offset;
    const float scaleFactor = 2.0F;
    generatedHeight /= scaleFactor;
    return generatedHeight;
}

void TerrainErosion::regenerateNoiseTerrain() {
    auto now = std::chrono::high_resolution_clock::now();
    int seed = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
    noise1->SetSeed(seed);
    noise2->SetSeed(seed);
    noise3->SetSeed(seed);

    const glm::vec3 scale = {10.0F, 10.0F, 1.0F};
    const glm::vec3 frequencyScale = {15.0F, 5.0F, 1.0F};

    for (unsigned int i = 0; i < noiseTerrain.heightMap.grid.size(); i++) {
        int x = noiseTerrain.heightMap.grid[i].x;
        int y = noiseTerrain.heightMap.grid[i].y;
        float realX = static_cast<float>(x) / scale.x;
        float realY = static_cast<float>(y) / scale.y;

        float generatedHeight = 0.0F;
        generatedHeight += generateHeight(noise1, realX, realY, scale.z) * frequencyScale.x;
        generatedHeight += generateHeight(noise2, realX, realY, scale.z) * frequencyScale.y;
        generatedHeight += generateHeight(noise3, realX, realY, scale.z) * frequencyScale.z;

        noiseTerrain.heightMap.set(x, y, generatedHeight * 3.0F);
    }

    auto heights = std::vector<float>(noiseTerrain.heightMap.grid.size());
    for (unsigned int i = 0; i < noiseTerrain.heightMap.grid.size(); i++) {
        auto &vec = noiseTerrain.heightMap.grid[i];
        heights[i] = noiseTerrain.heightMap.get(vec.x, vec.y);
    }
    noiseTerrain.heightBuffer->update(heights);
}

template <typename T>
void uploadBufferDataStriped(const TerrainData &terrainData, const std::shared_ptr<VertexBuffer> &buffer,
                             const int counter, int verticesPerFrame,
                             const std::function<T(const TerrainData &, int)> &func) {
    int numSegments = 0;
    if (verticesPerFrame < static_cast<int>(terrainData.heightMap.grid.size())) {
        numSegments = std::ceil(terrainData.heightMap.grid.size() / verticesPerFrame);
    } else {
        numSegments = 1;
        verticesPerFrame = terrainData.heightMap.grid.size();
    }
    int segment = counter % numSegments;

    auto data = std::vector<T>(verticesPerFrame);
    for (unsigned int i = segment * verticesPerFrame;
         i < terrainData.heightMap.grid.size() && i < static_cast<unsigned int>((segment + 1) * verticesPerFrame);
         i++) {
        data[i - segment * verticesPerFrame] = func(terrainData, i);
    }
    int offset = segment * verticesPerFrame * sizeof(T);
    size_t size = sizeof(T) * data.size();

    buffer->bind();
    GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data.data()));
}

void TerrainErosion::renderTerrain(const TerrainData &terrainData, const glm::mat4 &modelMatrix,
                                   const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                                   const glm::mat3 &normalMatrix, const glm::vec3 &surfaceToLight,
                                   const glm::vec3 &lightColor, const float lightPower, const bool wireframe,
                                   const bool drawTriangles, int verticesPerFrame, const TerrainLevels &levels) {
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

    terrainData.va->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    static int counter = 0;
    uploadBufferDataStriped<float>(terrainData, terrainData.heightBuffer, counter, verticesPerFrame,
                                   [](const TerrainData &td, int i) {
                                       auto &vec = td.heightMap.grid[i];
                                       return td.heightMap.get(vec.x, vec.y);
                                   });
    counter++;

    if (drawTriangles) {
        GL_Call(glDrawElements(GL_TRIANGLES, terrainData.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    } else {
        GL_Call(glDrawElements(GL_POINTS, terrainData.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    terrainData.va->unbind();
    shader->unbind();
}

void TerrainErosion::initTerrainMesh(TerrainData &terrainData, const std::vector<glm::vec2> &vertices,
                                     HeightMap &heightMap, const std::vector<glm::ivec3> &indices) {
    terrainData.va = std::make_shared<VertexArray>(shader);
    terrainData.va->bind();

    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    terrainData.va->addVertexBuffer(positionBuffer);

    terrainData.heightMap = heightMap;
    terrainData.heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    terrainData.heightBuffer->setLayout(heightLayout);
    terrainData.heightBuffer->bind();
    glBufferData(GL_ARRAY_BUFFER, heightMap.grid.size() * sizeof(glm::ivec2), nullptr, GL_DYNAMIC_DRAW);
    terrainData.va->addVertexBuffer(terrainData.heightBuffer);

    terrainData.normalBuffer = std::make_shared<VertexBuffer>();
    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    terrainData.normalBuffer->setLayout(normalLayout);
    terrainData.normalBuffer->bind();
    glBufferData(GL_ARRAY_BUFFER, heightMap.grid.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    terrainData.va->addVertexBuffer(terrainData.normalBuffer);

    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    terrainData.va->setIndexBuffer(indexBuffer);
}

void TerrainErosion::renderPaths(const std::vector<Raindrop> &raindrops, const glm::mat4 &modelMatrix,
                                 const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
    RECORD_SCOPE_NAME("Render Paths");

    pathShader->bind();
    pathShader->setUniform("modelMatrix", modelMatrix);
    pathShader->setUniform("viewMatrix", viewMatrix);
    pathShader->setUniform("projectionMatrix", projectionMatrix);

    auto va = std::make_shared<VertexArray>(pathShader);
    unsigned int verticesCount = 0;
    for (const auto &raindrop : raindrops) {
        verticesCount += raindrop.path.size() + 1;
    }
    auto vertices = std::vector<glm::vec3>();
    vertices.reserve(verticesCount);
    for (const auto &raindrop : raindrops) {
        if (!raindrop.path.empty()) {
            vertices.push_back(raindrop.path[0] + glm::vec3(0.0F, 5.0F, 0.0F));
        }
        for (const auto &vertex : raindrop.path) {
            vertices.push_back(vertex + glm::vec3(0.0F, 0.1F, 0.0F));
        }
    }

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    va->addVertexBuffer(vb);

    auto indices = std::vector<unsigned int>();
    indices.reserve(verticesCount + raindrops.size());
    unsigned int currentIndex = 0;
    for (const auto &raindrop : raindrops) {
        if (!raindrop.path.empty()) {
            indices.push_back(currentIndex);
            currentIndex++;
        }
        for (unsigned int i = 0; i < raindrop.path.size(); i++) {
            indices.push_back(currentIndex);
            currentIndex++;
        }
        indices.push_back(~0);
    }
    auto ib = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(ib);

    GL_Call(glDrawElements(GL_LINE_STRIP, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    va->unbind();
    pathShader->unbind();
}

void TerrainErosion::recalculateNormals(TerrainData &terrainData, int verticesPerFrame) {
    RECORD_SCOPE_NAME("Calculate Normals");

    static int counter = 0;
    uploadBufferDataStriped<glm::vec3>(terrainData, terrainData.normalBuffer, counter, verticesPerFrame,
                                       [](const TerrainData &td, int i) {
                                           int x = td.heightMap.grid[i].x;
                                           int y = td.heightMap.grid[i].y;
                                           const float L = td.heightMap.get(x - 1, y);
                                           const float R = td.heightMap.get(x + 1, y);
                                           const float B = td.heightMap.get(x, y - 1);
                                           const float T = td.heightMap.get(x, y + 1);
                                           return glm::normalize(glm::vec3(2 * (L - R), 4, 2 * (B - T)));
                                       });
    counter++;
}

void TerrainErosion::runSimulation(TerrainData &terrainData, std::vector<Raindrop> &raindrops, bool letItRain,
                                   unsigned int simulationSpeed, bool onlyRainAroundCenterPoint,
                                   const glm::vec2 &centerPoint, unsigned int raindropCount, float radius,
                                   const SimulationParams &params) {
    static int counter = 0;
    counter++;
    if (!letItRain || counter % simulationSpeed != 0) {
        return;
    }

    RECORD_SCOPE_NAME("Rain Simulation");
    regenerateRaindrops(raindrops, onlyRainAroundCenterPoint, raindropCount, centerPoint, radius);
    for (auto &raindrop : raindrops) {
        simulateRaindrop(terrainData.heightMap, randomGenerator, randomDistribution, params, raindrop);
    }
}