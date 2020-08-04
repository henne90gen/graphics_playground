#include "TerrainErosion.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"

const int WIDTH = 300;
const int HEIGHT = 300;

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void TerrainErosion::setup() {
    std::srand(std::time(nullptr));

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
    static auto raindrops = std::vector<Raindrop>();
    static bool pathsInitialized = false;
    if (!pathsInitialized) {
        pathsInitialized = true;
        regenerateRaindrops(raindrops);
    }

    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    if (ImGui::Button("Regenerate Terrain")) {
        regenerateTerrain();
        adjustRaindropsToTerrain(raindrops);
    }
    if (ImGui::Button("Regenerate Raindrop Starting Positions")) {
        regenerateRaindrops(raindrops);
    }
    ImGui::Text("Point count: %d", WIDTH * HEIGHT);
    ImGui::End();

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

    regenerateRaindrops(raindrops);
    for (auto &raindrop : raindrops) {
        simulateRaindrop(raindrop);
    }
    renderPaths(raindrops);

    renderTerrain(wireframe);
}

void TerrainErosion::regenerateRaindrops(std::vector<Raindrop> &raindrops) const {
    unsigned int pathCount = 100;
    raindrops.clear();
    raindrops.reserve(pathCount);
    for (unsigned int j = 0; j < pathCount; j++) {
        unsigned int i = rand() % (WIDTH * HEIGHT);
        auto x = static_cast<float>(i % WIDTH);
        auto y = heightMap[i];
        auto z = std::floor(static_cast<float>(i) / static_cast<float>(WIDTH));
        Raindrop raindrop = {};
        raindrop.startingPosition = glm::vec3(x, y, z);
        raindrops.push_back(raindrop);
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

void TerrainErosion::adjustRaindropsToTerrain(std::vector<Raindrop> &raindrops) {
    for (auto &raindrop : raindrops) {
        unsigned int index = raindrop.startingPosition.z * WIDTH + raindrop.startingPosition.x;
        raindrop.startingPosition.y = heightMap[index];
    }
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

void TerrainErosion::renderPaths(const std::vector<Raindrop> &paths) {
    pathShader->bind();

    auto va = std::make_shared<VertexArray>(pathShader);
    unsigned int verticesCount = 0;
    for (const auto &path : paths) {
        verticesCount += path.path.size();
    }
    auto vertices = std::vector<glm::vec3>();
    vertices.reserve(verticesCount);
    for (const auto &path : paths) {
        for (const auto &vertex : path.path) {
            vertices.push_back(vertex + glm::vec3(0.0F, 0.1F, 0.0F));
        }
    }

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    va->addVertexBuffer(vb);

    auto indices = std::vector<unsigned int>();
    indices.reserve(verticesCount + paths.size());
    unsigned int currentIndex = 0;
    for (const auto &path : paths) {
        for (unsigned int i = 0; i < path.path.size(); i++) {
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

void TerrainErosion::simulateRaindrop(Raindrop &raindrop) {
    float Kq = 10;          // constant parameter for soil carry capacity formula
    float Kw = 0.001f;      // water evaporation speed
    float Kr = 0.9f;        // erosion speed
    float Kd = 0.02f;       // deposition speed
    float Ki = 0.1F;        // direction inertia
    float minSlope = 0.05f; // minimum slope for soil carry capacity formula
    float g = 20;           // gravity

#define HMAP(X, Z) heightMap[std::min((Z), HEIGHT - 1) * WIDTH + std::min((X), WIDTH - 1)]

#define DEPOSIT_AT(X, Z, W)                                                                                            \
    {                                                                                                                  \
        float delta = ds * (W);                                                                                        \
        HMAP((X), (Z)) += delta;                                                                                       \
    }

#if 1
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(xi, zi, (1 - xf) * (1 - zf))                                                                            \
    DEPOSIT_AT(xi + 1, zi, xf * (1 - zf))                                                                              \
    DEPOSIT_AT(xi, zi + 1, (1 - xf) * zf)                                                                              \
    DEPOSIT_AT(xi + 1, zi + 1, xf * zf)                                                                                \
    (H) += ds;
#else
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(xi, zi, 0.25f)                                                                                          \
    DEPOSIT_AT(xi + 1, zi, 0.25f)                                                                                      \
    DEPOSIT_AT(xi, zi + 1, 0.25f)                                                                                      \
    DEPOSIT_AT(xi + 1, zi + 1, 0.25f)                                                                                  \
    (H) += ds;
#endif

    raindrop.velocity = 0.0F;
    raindrop.acceleration = 0.0F;
    raindrop.water = 1.0F;
    raindrop.sediment = 0.0F;
    raindrop.path.clear();

    glm::vec3 current = raindrop.startingPosition;
    float xp = current.x;
    float zp = current.z;
    float xf = 0, zf = 0;
    float dx = 0.0F;
    float dz = 0.0F;
    auto xi = static_cast<int>(current.x);
    auto zi = static_cast<int>(current.z);
    float h00 = HMAP(xi, zi);
    float h10 = HMAP(xi + 1, zi);
    float h01 = HMAP(xi, zi + 1);
    float h11 = HMAP(xi + 1, (zi + 1));
    float h = h00;

    const int maxPathLength = WIDTH * 4;
    for (unsigned int i = 0; i < maxPathLength; i++) {
        raindrop.path.emplace_back(xp, h, zp);

        float gradientX = (h00 + h01) - (h10 + h11);
        float gradientZ = (h00 + h10) - (h01 - h11);

        dx = (dx - gradientX) * Ki + gradientX;
        dz = (dz - gradientZ) * Ki + gradientZ;

        float dl = std::sqrt(dx * dx + dz * dz);
        if (dl <= FLT_EPSILON) {
            // pick random dir
            float a = (((float)std::rand() / (float)(RAND_MAX))) * glm::two_pi<float>();
            dx = cosf(a);
            dz = sinf(a);
        } else {
            dx /= dl;
            dz /= dl;
        }

        float nxp = xp + dx;
        float nzp = zp + dz;

        int nxi = std::floor(nxp);
        int nzi = std::floor(nzp);
        float nxf = nxp - (float)nxi;
        float nzf = nzp - (float)nzi;

        float nh00 = HMAP(nxi, nzi);
        float nh01 = HMAP(nxi, nzi + 1);
        float nh10 = HMAP(nxi + 1, nzi);
        float nh11 = HMAP(nxi + 1, nzi + 1);

        float nh = (nh00 * (1 - nxf) + nh10 * nxf) * (1 - nzf) + (nh01 * (1 - nxf) + nh11 * nxf) * nzf;

        // if higher than current, try to deposit sediment up to neighbour height
        if (nh >= h) {
            float ds = (nh - h) + 0.001f;

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
        //        float slope = dh;
        float slope = dh / sqrtf(dh * dh + 1);

        float q = std::max(slope, minSlope) * raindrop.velocity * raindrop.water * Kq;
        float ds = raindrop.sediment - q;
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
        HMAP((X), (Z)) -= delta;                                                                                       \
    }
#if 1
            for (int z = zi - 1; z <= zi + 2; ++z) {
                float zo = z - zp;
                float zo2 = zo * zo;

                for (int x = xi - 1; x <= xi + 2; ++x) {
                    float xo = x - xp;

                    float w = 1 - (xo * xo + zo2) * 0.25f;
                    if (w <= 0)
                        continue;
                    w *= 0.1591549430918953f;

                    ERODE(x, z, w)
                }
            }
#else
            ERODE(xi, zi, (1 - xf) * (1 - zf))
            ERODE(xi + 1, zi, xf * (1 - zf))
            ERODE(xi, zi + 1, (1 - xf) * zf)
            ERODE(xi + 1, zi + 1, xf * zf)
#endif

            dh -= ds;

#undef ERODE

            raindrop.sediment += ds;
        }

        // move to the neighbour
        raindrop.velocity = std::sqrt(raindrop.velocity * raindrop.velocity + g * dh);
        raindrop.water *= 1 - Kw;

        xp = nxp;
        zp = nzp;
        xi = nxi;
        zi = nzi;
        xf = nxf;
        zf = nzf;

        h = nh;
        h00 = nh00;
        h10 = nh10;
        h01 = nh01;
        h11 = nh11;
    }
}
