#include "TerrainErosion.h"

#include <chrono>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void TerrainErosion::setup() {
    std::random_device global_random_device;
    randomGenerator = std::mt19937(global_random_device());
    randomDistribution = std::uniform_real_distribution<double>(0.0, 1.0);

    GL_Call(glEnable(GL_PRIMITIVE_RESTART));
    GL_Call(glPrimitiveRestartIndex(~0));

    pathShader =
          std::make_shared<Shader>("scenes/terrain_erosion/PathVert.glsl", "scenes/terrain_erosion/PathFrag.glsl");

    shader = std::make_shared<Shader>("scenes/terrain_erosion/TerrainVert.glsl",
                                      "scenes/terrain_erosion/TerrainFrag.glsl");
    shader->bind();

    terrainVA = std::make_shared<VertexArray>(shader);
    generateTerrainMesh();

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

    regenerateTerrain();
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
    static auto cameraRotation = glm::vec3(0.55F, 0.0F, 0.0F);
    static glm::vec3 surfaceToLight = {-4.5F, 7.0F, 0.0F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static float lightPower = 13.0F;
    static bool wireframe = false;
    static bool shouldRenderPaths = false;
    static auto terrainLevels = TerrainLevels();
    static bool onlyRainAroundCenterPoint = false;
    static bool letItRain = true;
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
                 shouldRenderPaths, onlyRainAroundCenterPoint, letItRain, params, raindropCount, centerPoint, radius,
                 simulationSpeed, terrainLevels);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    static int counter = 0;
    counter++;
    if (letItRain) {
        if (counter % simulationSpeed == 0) {
            regenerateRaindrops(raindrops, onlyRainAroundCenterPoint, raindropCount, centerPoint, radius);
            for (auto &raindrop : raindrops) {
                simulateRaindrop(heightMap, randomGenerator, randomDistribution, params, raindrop);
            }
        }
        if (shouldRenderPaths) {
            renderPaths(modelMatrix, viewMatrix, projectionMatrix, raindrops);
        }
    }

    recalculateNormals();
    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, terrainLevels);
}

void TerrainErosion::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                                  glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                                  bool &shouldRenderPaths, bool &onlyRainAroundCenterPoint, bool &letItRain,
                                  SimulationParams &params, int &raindropCount, glm::vec2 &centerPoint, float &radius,
                                  int &simulationSpeed, TerrainLevels &terrainLevels) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&cameraPosition));
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPos));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Raindrop Paths", &shouldRenderPaths);
    if (ImGui::Button("Regenerate Terrain")) {
        regenerateTerrain();
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
    ImGui::Checkbox("Only Simulate around Point", &onlyRainAroundCenterPoint);
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

void TerrainErosion::regenerateTerrain() {
    auto now = std::chrono::high_resolution_clock::now();
    int seed = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
    noise1->SetSeed(seed);
    noise2->SetSeed(seed);
    noise3->SetSeed(seed);

    const glm::vec3 scale = {10.0F, 10.0F, 1.0F};
    const glm::vec3 frequencyScale = {15.0F, 5.0F, 1.0F};

    auto width = static_cast<unsigned int>(HEIGHTMAP_WIDTH);
    auto height = static_cast<unsigned int>(HEIGHTMAP_HEIGHT);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            float realX = static_cast<float>(x) / scale.x;
            float realY = static_cast<float>(y) / scale.y;

            float generatedHeight = 0.0F;
            generatedHeight += generateHeight(noise1, realX, realY, scale.z) * frequencyScale.x;
            generatedHeight += generateHeight(noise2, realX, realY, scale.z) * frequencyScale.y;
            generatedHeight += generateHeight(noise3, realX, realY, scale.z) * frequencyScale.z;

            heightMap.data[y * width + x] = generatedHeight * 3.0F;
        }
    }
    heightBuffer->update(heightMap.data);
}

void TerrainErosion::renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                                   const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                                   const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, float &lightPower,
                                   bool wireframe, const TerrainLevels &levels) {
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

    terrainVA->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    heightBuffer->update(heightMap.data);
    GL_Call(glDrawElements(GL_TRIANGLES, terrainVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    terrainVA->unbind();
    shader->unbind();
}

void TerrainErosion::generateTerrainMesh() {
    terrainVA->bind();

    const unsigned int verticesCount = HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % HEIGHTMAP_WIDTH);
        auto z = std::floor(static_cast<float>(i) / static_cast<float>(HEIGHTMAP_HEIGHT));
        vertices[i] = glm::vec2(x, z);
    }

    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    terrainVA->addVertexBuffer(positionBuffer);

    heightMap = {HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, std::vector<float>(verticesCount)};
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    terrainVA->addVertexBuffer(heightBuffer);

    normals = std::vector<glm::vec3>(verticesCount);
    normalBuffer = std::make_shared<VertexBuffer>();
    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    normalBuffer->setLayout(normalLayout);
    terrainVA->addVertexBuffer(normalBuffer);

    const unsigned int indicesPerQuad = 6;
    unsigned int indicesCount = HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT * indicesPerQuad;
    auto indices = std::vector<unsigned int>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < HEIGHTMAP_HEIGHT - 1; y++) {
        for (unsigned int x = 0; x < HEIGHTMAP_WIDTH - 1; x++) {
            indices[counter++] = (y + 1) * HEIGHTMAP_WIDTH + x;
            indices[counter++] = y * HEIGHTMAP_WIDTH + (x + 1);
            indices[counter++] = y * HEIGHTMAP_WIDTH + x;
            indices[counter++] = (y + 1) * HEIGHTMAP_WIDTH + x;
            indices[counter++] = (y + 1) * HEIGHTMAP_WIDTH + (x + 1);
            indices[counter++] = y * HEIGHTMAP_WIDTH + (x + 1);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());
    terrainVA->setIndexBuffer(indexBuffer);
}

void TerrainErosion::renderPaths(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                                 const glm::mat4 &projectionMatrix, const std::vector<Raindrop> &raindrops) {
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

void TerrainErosion::recalculateNormals() {
    normals.clear();
    normals.reserve(heightMap.data.size());
    for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
        for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
            const float L = getHeightMapValue(heightMap, x - 1, y);
            const float R = getHeightMapValue(heightMap, x + 1, y);
            const float B = getHeightMapValue(heightMap, x, y - 1);
            const float T = getHeightMapValue(heightMap, x, y + 1);
            const glm::vec3 normal = glm::normalize(glm::vec3(2 * (L - R), 4, 2 * (B - T)));
            normals.push_back(normal);
        }
    }
    normalBuffer->update(normals);
}
