#include "TerrainErosion.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/VectorUtils.h"

const int WIDTH = 300;
const int HEIGHT = 300;

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void TerrainErosion::setup() {
    std::random_device global_random_device = {};
    randomGenerator = std::mt19937(global_random_device());
    randomDistribution = std::uniform_real_distribution<double>(0.0, 1.0);

    GL_Call(glEnable(GL_PRIMITIVE_RESTART));
    GL_Call(glPrimitiveRestartIndex(~0));

    pathShader =
          std::make_shared<Shader>("scenes/terrain_erosion/PathVert.glsl", "scenes/terrain_erosion/PathFrag.glsl");

    shader = std::make_shared<Shader>("scenes/terrain_erosion/TerrainErosionVert.glsl",
                                      "scenes/terrain_erosion/TerrainErosionFrag.glsl");
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
    static auto modelScale = glm::vec3(0.4F, 2.0F, 0.4F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraPosition = glm::vec3(-50.0F, -100.0F, -160.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.65F, 0.0F, 0.0F);
    static bool wireframe = false;
    static bool shouldRenderPaths = false;
    static bool onlyRainAroundCenterPoint = false;
    static bool letItRain = true;
    static auto params = SimulationParams();
    static int raindropCount = 100;
    static auto centerPoint = glm::vec2(WIDTH / 2, HEIGHT / 2);
    static auto radius = 30.0F;
    static int simulationSpeed = 2;
    static auto raindrops = std::vector<Raindrop>();
    static bool pathsInitialized = false;
    if (!pathsInitialized) {
        pathsInitialized = true;
        regenerateRaindrops(raindrops, onlyRainAroundCenterPoint, raindropCount, centerPoint, radius);
    }

    showSettings(modelScale, cameraPosition, cameraRotation, wireframe, shouldRenderPaths, onlyRainAroundCenterPoint,
                 letItRain, params, raindropCount, centerPoint, radius, simulationSpeed);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    pathShader->bind();
    pathShader->setUniform("modelMatrix", modelMatrix);
    pathShader->setUniform("viewMatrix", viewMatrix);
    pathShader->setUniform("projectionMatrix", projectionMatrix);

    static int counter = 0;
    counter++;
    if (letItRain) {
        if (counter % simulationSpeed == 0) {
            regenerateRaindrops(raindrops, onlyRainAroundCenterPoint, raindropCount, centerPoint, radius);
            for (auto &raindrop : raindrops) {
                simulateRaindrop(params, raindrop);
            }
        }
        if (shouldRenderPaths) {
            renderPaths(raindrops);
        }
    }

    renderTerrain(wireframe);
}

void TerrainErosion::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                                  bool &wireframe, bool &shouldRenderPaths, bool &onlyRainAroundCenterPoint,
                                  bool &letItRain, SimulationParams &params, int &raindropCount, glm::vec2 &centerPoint,
                                  float &radius, int &simulationSpeed) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&cameraPosition));
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Raindrop Paths", &shouldRenderPaths);
    if (ImGui::Button("Regenerate Terrain")) {
        regenerateTerrain();
    }
    ImGui::Text("Point count: %d", WIDTH * HEIGHT);

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
            auto i = static_cast<unsigned int>(num * (WIDTH * HEIGHT));
            auto x = static_cast<float>(i % WIDTH);
            auto z = std::floor(static_cast<float>(i) / static_cast<float>(WIDTH));
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

    const glm::vec3 scale = {15.0F, 5.0F, 1.0F};
    const glm::vec3 frequencyScale = {15.0F, 5.0F, 1.0F};

    float maxHeight = 0;
    auto width = static_cast<unsigned int>(WIDTH);
    auto height = static_cast<unsigned int>(HEIGHT);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            float realX = static_cast<float>(x) / scale.x;
            float realY = static_cast<float>(y) / scale.y;

            float generatedHeight = 0.0F;
            generatedHeight += generateHeight(noise1, realX, realY, scale.z) * frequencyScale.x;
            generatedHeight += generateHeight(noise2, realX, realY, scale.z) * frequencyScale.y;
            generatedHeight += generateHeight(noise3, realX, realY, scale.z) * frequencyScale.z;

            heightMap[y * width + x] = generatedHeight;
            if (heightMap[y * width + x] > maxHeight) {
                maxHeight = heightMap[y * width + x];
            }
        }
    }
    heightBuffer->update(heightMap);

    shader->bind();
    shader->setUniform("maxHeight", maxHeight);
}

void TerrainErosion::renderTerrain(const bool wireframe) {
    shader->bind();
    terrainVA->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    heightBuffer->update(heightMap);
    GL_Call(glDrawElements(GL_TRIANGLES, terrainVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    terrainVA->unbind();
    shader->unbind();
}

void TerrainErosion::generateTerrainMesh() {
    terrainVA->bind();

    const unsigned int verticesCount = WIDTH * HEIGHT;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % WIDTH);
        auto z = std::floor(static_cast<float>(i) / static_cast<float>(HEIGHT));
        vertices[i] = glm::vec2(x, z);
    }

    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    terrainVA->addVertexBuffer(positionBuffer);

    heightMap = std::vector<float>(verticesCount);
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    terrainVA->addVertexBuffer(heightBuffer);

    const unsigned int indicesPerQuad = 6;
    unsigned int indicesCount = WIDTH * HEIGHT * indicesPerQuad;
    auto indices = std::vector<unsigned int>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < HEIGHT - 1; y++) {
        for (unsigned int x = 0; x < WIDTH - 1; x++) {
            indices[counter++] = (y + 1) * WIDTH + x;
            indices[counter++] = y * WIDTH + (x + 1);
            indices[counter++] = y * WIDTH + x;
            indices[counter++] = (y + 1) * WIDTH + x;
            indices[counter++] = (y + 1) * WIDTH + (x + 1);
            indices[counter++] = y * WIDTH + (x + 1);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());
    terrainVA->setIndexBuffer(indexBuffer);
}

void TerrainErosion::renderPaths(const std::vector<Raindrop> &raindrops) {
    pathShader->bind();

    auto va = std::make_shared<VertexArray>(pathShader);
    unsigned int verticesCount = 0;
    for (const auto &raindrop : raindrops) {
        verticesCount += raindrop.path.size() + 1;
    }
    auto vertices = std::vector<glm::vec3>();
    vertices.reserve(verticesCount);
    for (const auto &raindrop : raindrops) {
        if (!raindrop.path.empty()) {
            vertices.push_back(raindrop.path[0] + glm::vec3(0.0F, 10.0F, 0.0F));
        }
        for (const auto &vertex : raindrop.path) {
            vertices.push_back(vertex + glm::vec3(0.0F, 0.5F, 0.0F));
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

void TerrainErosion::setHeightMapValue(int x, int z, float value) {
    unsigned long safeZ = std::min(std::max(z, 0), HEIGHT - 1);
    unsigned long safeX = std::min(std::max(x, 0), WIDTH - 1);
    unsigned long index = safeZ * WIDTH + safeX;
    ASSERT(index < heightMap.size());
    heightMap[index] = value;
}

float TerrainErosion::getHeightMapValue(int x, int z) {
    unsigned long safeZ = std::min(std::max(z, 0), HEIGHT - 1);
    unsigned long safeX = std::min(std::max(x, 0), WIDTH - 1);
    unsigned long index = safeZ * WIDTH + safeX;
    ASSERT(index < heightMap.size());
    return heightMap[index];
}

void TerrainErosion::simulateRaindrop(const SimulationParams &params, Raindrop &raindrop) {
    float Kq = params.Kq;
    float Kw = params.Kw;
    float Kr = params.Kr;
    float Kd = params.Kd;
    float Ki = params.Ki;
    float minSlope = params.minSlope;
    float g = params.g;

#define DEPOSIT_AT(X, Z, W)                                                                                            \
    {                                                                                                                  \
        float delta = ds * (W);                                                                                        \
        float newH = getHeightMapValue((X), (Z)) + delta;                                                              \
        setHeightMapValue((X), (Z), newH);                                                                             \
    }

#if 1
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(pi.x, pi.y, (1 - f.x) * (1 - f.y))                                                                      \
    DEPOSIT_AT(pi.x + 1, pi.y, f.x * (1 - f.y))                                                                        \
    DEPOSIT_AT(pi.x, pi.y + 1, (1 - f.x) * f.y)                                                                        \
    DEPOSIT_AT(pi.x + 1, pi.y + 1, f.x * f.y)                                                                          \
    (H) += ds;
#else
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(pi.x, pi.y, 0.25f)                                                                                      \
    DEPOSIT_AT(pi.x + 1, pi.y, 0.25f)                                                                                  \
    DEPOSIT_AT(pi.x, pi.y + 1, 0.25f)                                                                                  \
    DEPOSIT_AT(pi.x + 1, pi.y + 1, 0.25f)                                                                              \
    (H) += ds;
#endif

    raindrop.velocity = 0.0F;
    raindrop.water = 1.0F;
    raindrop.sediment = 0.0F;
    raindrop.path.clear();

    glm::vec2 p = {raindrop.startingPosition.x, raindrop.startingPosition.y};
    glm::ivec2 pi = {static_cast<int>(p.x), static_cast<int>(p.y)};
    glm::vec2 f = p - glm::vec2(pi.x, pi.y);
    glm::vec2 d = {0.0F, 0.0F};
    float h00 = getHeightMapValue(pi.x, pi.y);
    float h10 = getHeightMapValue(pi.x + 1, pi.y);
    float h01 = getHeightMapValue(pi.x, pi.y + 1);
    float h11 = getHeightMapValue(pi.x + 1, pi.y + 1);
    float h = h00;

    const int maxPathLength = WIDTH * 4;
    for (unsigned int i = 0; i < maxPathLength; i++) {
        raindrop.path.emplace_back(p.x, h, p.y);

        glm::vec2 gradient = {
              (h10 - h00) * (1 - f.y) + (h11 - h01) * f.y, //
              (h01 - h00) * (1 - f.x) + (h11 - h10) * f.x  //
        };
        d = d * Ki - gradient * (1.0F - Ki);

        float dl = glm::length(d);
        if (dl <= FLT_EPSILON) {
            // pick random dir
            double a = randomDistribution(randomGenerator) * glm::two_pi<double>();
            d = {std::cos(a), std::sin(a)};
        }

        d = glm::normalize(d);

        glm::vec2 np = p + d;
        glm::ivec2 ni = {std::floor(np.x), std::floor(np.y)};
        glm::vec2 nf = np - glm::vec2(ni.x, ni.y);

        float nh00 = getHeightMapValue(ni.x, ni.y);
        float nh01 = getHeightMapValue(ni.x, ni.y + 1);
        float nh10 = getHeightMapValue(ni.x + 1, ni.y);
        float nh11 = getHeightMapValue(ni.x + 1, ni.y + 1);

        float nh0 = nh00 * (1 - nf.x) + nh10 * nf.x;
        float nh1 = nh01 * (1 - nf.x) + nh11 * nf.x;
        float nh = nh0 * (1 - nf.y) + nh1 * nf.y;

        // if higher than current, try to deposit sediment up to neighbour height
        if (nh >= h) {
            float ds = (nh - h) + 0.001F;

            if (ds >= raindrop.sediment) {
                // deposit all sediment and stop
                ds = raindrop.sediment;
                DEPOSIT(h)
                raindrop.sediment = 0.0F;
                break;
            }

            DEPOSIT(h)
            raindrop.sediment -= ds;
            raindrop.velocity = 0.0F;
        }

        // compute transport capacity
        float dh = h - nh;
        float slope = dh;
        //         float slope = dh / sqrtf(dh * dh + 1);

        float carryCapacity = std::max(slope, minSlope) * raindrop.velocity * raindrop.water * Kq;
        float ds = raindrop.sediment - carryCapacity;
        if (ds >= 0) {
            // deposit
            ds *= Kd;
            // ds=minval(ds, 1.0f);

            DEPOSIT(dh)
            raindrop.sediment -= ds;
        } else {
            // erode
            ds *= -Kr;
            ds = std::min(ds, dh * 0.99f);

#define ERODE(X, Z, W)                                                                                                 \
    {                                                                                                                  \
        float delta = ds * (W);                                                                                        \
        float newH = getHeightMapValue((X), (Z)) - delta;                                                              \
        setHeightMapValue((X), (Z), newH);                                                                             \
    }
#if 1
            for (int z = pi.y - 1; z <= pi.y + 2; z++) {
                float zo = static_cast<float>(z) - p.y;
                float zo2 = zo * zo;

                for (int x = pi.x - 1; x <= pi.x + 2; x++) {
                    float xo = static_cast<float>(x) - p.x;

                    float w = 1 - (xo * xo + zo2) * 0.25F;
                    if (w <= 0) {
                        continue;
                    }
                    w *= 0.1591549430918953f;

                    ERODE(x, z, w)
                }
            }
#else
            ERODE(pi.x, pi.y, (1 - f.x) * (1 - f.y))
            ERODE(pi.x + 1, pi.y, f.x * (1 - f.y))
            ERODE(pi.x, pi.y + 1, (1 - f.x) * f.y)
            ERODE(pi.x + 1, pi.y + 1, f.x * f.y)
#endif

#undef ERODE

            dh -= ds;
            raindrop.sediment += ds;
        }

        // move to the neighbour
        raindrop.velocity = std::sqrt(raindrop.velocity * raindrop.velocity + g * dh);
        raindrop.water *= 1 - Kw;

        p = np;
        pi = ni;
        f = nf;

        h = nh;
        h00 = nh00;
        h10 = nh10;
        h01 = nh01;
        h11 = nh11;
    }
}
