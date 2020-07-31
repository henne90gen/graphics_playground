#include "TerrainErosion.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"

const unsigned int WIDTH = 15 * 15;
const unsigned int HEIGHT = 15 * 15;

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void TerrainErosion::setup() {
    std::srand(std::time(nullptr));

    pathShader =
          std::make_shared<Shader>("scenes/terrain_erosion/PathVert.glsl", "scenes/terrain_erosion/PathFrag.glsl");

    shader = std::make_shared<Shader>("scenes/terrain_erosion/TerrainErosionVert.glsl",
                                      "scenes/terrain_erosion/TerrainErosionFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    generatePoints();

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

    updateHeightBuffer();
}

void TerrainErosion::destroy() {}

void TerrainErosion::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(0.5F, 2.0F, 0.5F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraPosition = glm::vec3(-50.0F, -100.0F, -160.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.65F, 0.0F, 0.0F);
    static bool wireframe = false;
    static glm::vec3 startingPosition = {100.0F, 0.0F, 0.0F};

    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    if (ImGui::Button("Regenerate Terrain")) {
        auto now = std::chrono::high_resolution_clock::now();
        int seed = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
        noise1->SetSeed(seed);
        noise2->SetSeed(seed);
        noise3->SetSeed(seed);
        updateHeightBuffer();
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

    renderTerrain(wireframe);

    std::vector<glm::vec3> path = {};

    static unsigned int counter = 0;
    counter++;
    if (counter % 10 == 0) {
        unsigned int i = std::rand() % (WIDTH * HEIGHT);
        auto x = static_cast<float>(i % WIDTH);
        auto y = heightMap[i];
        auto z = std::floor(static_cast<float>(i) / static_cast<float>(WIDTH));
        startingPosition = glm::vec3(x, y, z);
    }

    simulateRainDrop(path, startingPosition);

    renderPath(path);
}

void TerrainErosion::renderTerrain(const bool wireframe) {
    shader->bind();
    vertexArray->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    vertexArray->unbind();
    shader->unbind();
}

void TerrainErosion::generatePoints() {
    vertexArray->bind();

    const unsigned int verticesCount = WIDTH * HEIGHT;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % WIDTH);
        auto z = std::floor(static_cast<float>(i) / static_cast<float>(HEIGHT));
        vertices[i] = glm::vec2(x, z);
    }

    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    heightMap = std::vector<float>(verticesCount);
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    vertexArray->addVertexBuffer(heightBuffer);

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
    vertexArray->setIndexBuffer(indexBuffer);
}

float generateHeight(const FastNoise *noise, const float x, const float y, const float z) {
    float generatedHeight = noise->GetNoise(x, y, z);
    const float offset = 1.0F;
    generatedHeight += offset;
    const float scaleFactor = 2.0F;
    generatedHeight /= scaleFactor;
    return generatedHeight;
}

void TerrainErosion::updateHeightBuffer() {
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

void TerrainErosion::renderPath(const std::vector<glm::vec3> &path) {
    pathShader->bind();

    auto va = std::make_shared<VertexArray>(pathShader);
    auto vertices = std::vector<glm::vec3>(path.size());
    for (unsigned int i = 0; i < path.size(); i++) {
        vertices[i] = path[i] + glm::vec3(0.0F, 0.1F, 0.0F);
    }

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    va->addVertexBuffer(vb);

    auto indices = std::vector<unsigned int>(path.size());
    for (unsigned int i = 0; i < path.size(); i++) {
        indices[i] = i;
    }
    auto ib = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(ib);

    GL_Call(glDrawElements(GL_LINE_STRIP, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    va->unbind();
    pathShader->unbind();
}

bool isInBound(unsigned int row, unsigned int column) { return row < HEIGHT && column < WIDTH; }

void TerrainErosion::simulateRainDrop(std::vector<glm::vec3> &path, const glm::vec3 &start) {
#define COMPARE_AND_UPDATE(row, column)                                                                                \
    if (isInBound(row, column)) {                                                                                      \
        glm::vec3 vec = glm::vec3(column, heightMap[row * WIDTH + column], row);                                       \
        float gradient = current.y - vec.y;                                                                            \
        if (gradient > 0.0F && gradient < smallestGradient) {                                                          \
            smallestGradient = gradient;                                                                               \
            smallestGradientVec = vec;                                                                                 \
        }                                                                                                              \
    }

    path.push_back(start);

    glm::vec3 current = start;
    while (true) {
        float smallestGradient = std::numeric_limits<float>::max();
        glm::vec3 smallestGradientVec = {};

        // left
        unsigned int row = current.z;
        unsigned int column = current.x - 1.0F;
        COMPARE_AND_UPDATE(row, column)

        // right
        row = current.z;
        column = current.x + 1.0F;
        COMPARE_AND_UPDATE(row, column)

        // top
        row = current.z - 1.0F;
        column = current.x;
        COMPARE_AND_UPDATE(row, column)

        // bottom
        row = current.z + 1.0F;
        column = current.x;
        COMPARE_AND_UPDATE(row, column)

        // top-left
        row = current.z - 1.0F;
        column = current.x - 1.0F;
        COMPARE_AND_UPDATE(row, column)

        // top-right
        row = current.z - 1.0F;
        column = current.x + 1.0F;
        COMPARE_AND_UPDATE(row, column)

        // bottom-left
        row = current.z + 1.0F;
        column = current.x - 1.0F;
        COMPARE_AND_UPDATE(row, column)

        // bottom-right
        row = current.z + 1.0F;
        column = current.x + 1.0F;
        COMPARE_AND_UPDATE(row, column)

        if (smallestGradient == std::numeric_limits<float>::max()) {
            break;
        }

        path.push_back(smallestGradientVec);
        current = smallestGradientVec;
    }
}
