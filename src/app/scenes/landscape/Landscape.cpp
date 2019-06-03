#include "Landscape.h"

#include <cmath>
#include <array>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"

const unsigned int WIDTH = 10;
const unsigned int HEIGHT = 10;

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void Landscape::setup() {
    shader = new Shader("../../../src/app/scenes/landscape/Landscape.vertex",
                        "../../../src/app/scenes/landscape/Landscape.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    generatePoints();

    noise = new FastNoise();
}

void Landscape::generatePoints() {
    vertexArray->bind();

    const auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    const auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    const unsigned int verticesCount = width * height;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % width);
        auto y = std::floor(static_cast<float>(i) / static_cast<float>(width));
        x /= static_cast<float>(pointDensity);
        y /= static_cast<float>(pointDensity);
        vertices[i] = glm::vec2(x, y);
    }

    const unsigned long verticesSize = vertices.size() * 2 * sizeof(float);
    auto *positionBuffer = new VertexBuffer(vertices.data(), verticesSize);
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    const unsigned int heightMapCount = width * height;
    heightMap = std::vector<float>(heightMapCount);
    heightBuffer = new VertexBuffer();
    VertexBufferLayout heightLayout;
    heightLayout.add<float>(shader, "height", 1);
    vertexArray->addBuffer(*heightBuffer, heightLayout);

    const unsigned int indicesPerQuad = 6;
    unsigned int indicesCount = width * height * indicesPerQuad;
    auto indices = std::vector<unsigned int>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < height - 1; y++) {
        for (unsigned int x = 0; x < width - 1; x++) {
            indices[counter++] = (y + 1) * width + x;
            indices[counter++] = y * width + (x + 1);
            indices[counter++] = y * width + x;
            indices[counter++] = (y + 1) * width + x;
            indices[counter++] = (y + 1) * width + (x + 1);
            indices[counter++] = y * width + (x + 1);
        }
    }
    indexBuffer = new IndexBuffer(indices.data(), indices.size());
}

void Landscape::destroy() {
    delete noise;
}

void Landscape::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(7.0F, 7.0F, 2.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto translation = glm::vec3(-30.0F, -20.0F, -50.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(-1.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto scale = glm::vec3(5.0F, 5.0F, 1.0F);
    scale.x = pointDensity;
    scale.y = pointDensity;
    const float timeSpeed = 0.01F;
    scale.z += timeSpeed;
    static auto movement = glm::vec2(0.0F);
    static FastNoise::NoiseType noiseType = FastNoise::Simplex;
    int lastPointDensity = pointDensity;
    static float frequency = 0.3F;

    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat2("Movement", reinterpret_cast<float *>(&movement), dragSpeed);

    ImGui::NoiseTypeSelector(&noiseType);
    ImGui::SliderFloat("Frequency", &frequency, 0.0F, 1.0F);

    const int minimumPointDensity = 1;
    const int maximumPointDensity = 100;
    ImGui::SliderInt("Point Density", &pointDensity, minimumPointDensity, maximumPointDensity);
    ImGui::Text("Point count: %d", pointDensity * pointDensity * WIDTH * HEIGHT);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    noise->SetFrequency(frequency);
    if (lastPointDensity != pointDensity) {
        generatePoints();
    }

    updateHeightBuffer(scale, movement, noiseType);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();

    shader->unbind();
}

void Landscape::updateHeightBuffer(const glm::vec3 &scale, const glm::vec2 &movement,
                                   FastNoise::NoiseType &noiseType) {
    noise->SetNoiseType(noiseType);
    float maxHeight = 0;
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            float realX = static_cast<float>(x) / scale.x + movement.x;
            float realY = static_cast<float>(y) / scale.y + movement.y;

            float generatedHeight = noise->GetNoise(realX, realY, scale.z);
            const float offset = 1.0F;
            const float scaleFactor = 2.0F;
            generatedHeight += offset;
            generatedHeight /= scaleFactor;

            auto *ptr = const_cast<float *>(heightMap.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
            const float arbitraryScaleFactor = 10.0F;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ptr[y * width + x] = generatedHeight * arbitraryScaleFactor;
            if (heightMap[y * width + x] > maxHeight) {
                maxHeight = heightMap[y * width + x];
            }
        }
    }
    heightBuffer->update(heightMap.data(), heightMap.size() * sizeof(float));

    shader->setUniform("maxHeight", maxHeight);
}
