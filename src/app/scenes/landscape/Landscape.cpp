#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-macro-usage"

#include "Landscape.h"

#include <cmath>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

constexpr unsigned int WIDTH = 10;
constexpr unsigned int HEIGHT = 10;
constexpr int INITIAL_POINT_DENSITY = 15;

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1000.0F;

DEFINE_SHADER(landscape_Landscape)

void Landscape::setup() {
    shader = SHADER(landscape_Landscape);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    generatePoints(INITIAL_POINT_DENSITY);
}

void Landscape::destroy() {}

void Landscape::tick() {
    constexpr float timeSpeed = 0.01F;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(7.0F, 7.0F, 2.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto translation = glm::vec3(-30.0F, -30.0F, -50.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(-1.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto scale = glm::vec3(5.0F, 5.0F, 1.0F);
    static auto pointDensity = INITIAL_POINT_DENSITY;
    static auto movement = glm::vec2(0.0F);
    static auto drawWireframe = false;
    static auto animate = false;
    int lastPointDensity = pointDensity;

    static std::vector<Layer *> layers = {
          new BowlLayer(0.04F),       //
          new NoiseLayer(1.0F, 0.1F), //
          new NoiseLayer(0.5F, 0.3F), //
          new NoiseLayer(0.2F, 0.5F), //
    };

    const float dragSpeed = 0.01F;
    const int minimumPointDensity = 1;
    const int maximumPointDensity = 80;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale), dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat2("Movement", reinterpret_cast<float *>(&movement), dragSpeed);
    ImGui::SliderInt("Point Density", &pointDensity, minimumPointDensity, maximumPointDensity);
    ImGui::Text("Point count: %d", pointDensity * pointDensity * WIDTH * HEIGHT);
    ImGui::Checkbox("Wireframe", &drawWireframe);
    ImGui::Checkbox("Animate", &animate);

    int layerToRemove = -1;
    for (int i = 0; i < static_cast<int>(layers.size()); i++) {
        layers[i]->renderMenu(i);
    }
    if (layerToRemove >= 0) {
        delete layers[layerToRemove];
        layers.erase(layers.begin() + layerToRemove);
    }

    ImGui::Separator();
    if (ImGui::Button("Add Layer")) {
        layers.push_back(new NoiseLayer());
    }

    ImGui::End(); //        layers.emplace_back();

    scale.x = pointDensity;
    scale.y = pointDensity;
    if (animate) {
        scale.z += timeSpeed;
    }

    shader->bind();
    vertexArray->bind();

    if (lastPointDensity != pointDensity) {
        generatePoints(pointDensity);
    }

    updateHeightBuffer(pointDensity, scale, movement, layers);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    vertexArray->unbind();
    shader->unbind();
}

void Landscape::generatePoints(unsigned int pointDensity) {
    vertexArray->bind();

    const auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    const auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    const unsigned int verticesCount = width * height;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
#pragma omp parallel for
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % width);
        auto y = std::floor(static_cast<float>(i) / static_cast<float>(width));
        x /= static_cast<float>(pointDensity);
        y /= static_cast<float>(pointDensity);
        vertices[i] = glm::vec2(x, y);
    }

    const unsigned long verticesSize = vertices.size() * 2 * sizeof(float);
    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices.data(), verticesSize, positionLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    const unsigned int heightMapCount = width * height;
    heightMap = std::vector<float>(heightMapCount);
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    vertexArray->addVertexBuffer(heightBuffer);

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
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());
    vertexArray->setIndexBuffer(indexBuffer);
}

void Landscape::updateHeightBuffer(const unsigned int pointDensity, const glm::vec3 &scale, const glm::vec2 &movement,
                                   const std::vector<Layer *> &layers) {
    float maxHeight = 0;
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);
#define SEQUENTIAL 0
#if SEQUENTIAL
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
#else
#pragma omp parallel for reduction(max : maxHeight)
    for (unsigned int i = 0; i < width * height; i++) {
        unsigned int x = i % width;
        unsigned int y = i / width;
        float realX = static_cast<float>(x) / scale.x + movement.x;
        float realY = static_cast<float>(y) / scale.y + movement.y;

        float generatedHeight = 0.0F;
        for (auto *layer : layers) {
            if (!layer->enabled) {
                continue;
            }

            generatedHeight += layer->getWeightedValue(WIDTH, HEIGHT, realX, realY, scale.z);
        }

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
#endif
    heightBuffer->update(heightMap);

    shader->setUniform("maxHeight", maxHeight);
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop
