#include "Landscape.h"

#include "Main.h"
#include "util/RenderUtils.h"

#include <cmath>
#include <memory>

constexpr unsigned int WIDTH = 10;
constexpr unsigned int HEIGHT = 10;
constexpr int INITIAL_POINT_DENSITY = 15;

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1000.0F;

DEFINE_SCENE_MAIN(Landscape)
DEFINE_SHADER(landscape_Landscape)
DEFINE_SHADER(landscape_NoiseTexture)

void Landscape::setup() {
    noiseTextureShader = SHADER(landscape_NoiseTexture);
    noiseTextureShader->bind();
    noiseTextureVA = createQuadVA(noiseTextureShader);
    TextureSettings textureSettings = {};
    textureSettings.dataType = GL_RED;
    noiseTexture = std::make_shared<Texture>(textureSettings);

    shader = SHADER(landscape_Landscape);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    generatePoints(INITIAL_POINT_DENSITY);
}

void Landscape::destroy() {}

void Landscape::tick() {
    constexpr float timeSpeed = 0.01F;
    static auto modelScale = glm::vec3(7.0F, 7.0F, 35.0F);
    static auto modelRotation = glm::vec3(-1.0F, 0.0F, 0.0F);
    static auto cameraPosition = glm::vec3(-30.0F, -30.0F, -70.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    static auto texturePosition = glm::vec3(0.0F, 0.0F, 0.0F);
    static auto textureRotation = glm::vec3(0.0F);
    static auto textureScale = glm::vec3(1.0F);
    static auto pointDensity = INITIAL_POINT_DENSITY;
    static auto movement = glm::vec3(0.0F);
    static auto drawWireframe = false;
    static auto animate = false;
    static auto shouldRenderTerrain = false;
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
    if (shouldRenderTerrain) {
        if (ImGui::Button("Show Texture")) {
            shouldRenderTerrain = false;
        }
    } else {
        if (ImGui::Button("Show Terrain")) {
            shouldRenderTerrain = true;
        }
    }
    ImGui::Separator();
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    //    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), dragSpeed);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Texture Scale", reinterpret_cast<float *>(&textureScale), dragSpeed);
    ImGui::DragFloat3("Texture Position", reinterpret_cast<float *>(&texturePosition), dragSpeed);
    ImGui::DragFloat3("Texture Rotation", reinterpret_cast<float *>(&textureRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Noise Offset", reinterpret_cast<float *>(&movement), dragSpeed);
    ImGui::SliderInt("Point Density", &pointDensity, minimumPointDensity, maximumPointDensity);
    ImGui::Text("Point count: %d", pointDensity * pointDensity * WIDTH * HEIGHT);
    ImGui::Checkbox("Wireframe", &drawWireframe);
    ImGui::Checkbox("Animate", &animate);

    int layerToRemove = -1;
    for (int i = 0; i < static_cast<int>(layers.size()); i++) {
        if (layers[i]->renderMenu(i)) {
            layerToRemove = i;
        }
    }
    if (layerToRemove >= 0) {
        delete layers[layerToRemove];
        layers.erase(layers.begin() + layerToRemove);
    }

    ImGui::Separator();
    if (ImGui::Button("Add Layer")) {
        layers.push_back(new NoiseLayer());
    }

    ImGui::End();

    if (animate) {
        movement.z += timeSpeed;
    }

    if (lastPointDensity != pointDensity) {
        generatePoints(pointDensity);
    }

    updateHeightBuffer(pointDensity, movement, layers);

    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    if (shouldRenderTerrain) {
        renderTerrain(projectionMatrix, viewMatrix, modelRotation, modelScale, drawWireframe);
    } else {
        renderNoiseTexture(projectionMatrix, viewMatrix, textureRotation, texturePosition, textureScale);
    }
}

void Landscape::renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                              const glm::vec3 &modelRotation, const glm::vec3 &modelScale, const bool drawWireframe) {
    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
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

void Landscape::renderNoiseTexture(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                                   const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                                   glm::vec3 &textureScale) {
    noiseTextureShader->bind();
    noiseTextureVA->bind();

    GL_Call(glActiveTexture(GL_TEXTURE0));
    noiseTexture->bind();

    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, texturePosition);
    modelMatrix = glm::rotate(modelMatrix, textureRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.z, glm::vec3(0, 0, 1));
    textureScale.y = textureScale.x * getAspectRatio();
    modelMatrix = glm::scale(modelMatrix, textureScale);
    noiseTextureShader->setUniform("modelMatrix", modelMatrix);
    noiseTextureShader->setUniform("textureSampler", 0);

    GL_Call(glDrawElements(GL_TRIANGLES, noiseTextureVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    noiseTextureVA->unbind();
    noiseTextureShader->unbind();
}

void Landscape::generatePoints(unsigned int pointDensity) {
    vertexArray->bind();

    const auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    const auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    const unsigned int verticesCount = width * height;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
#pragma omp parallel for
    for (long i = 0; i < vertices.size(); i++) {
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

void Landscape::updateHeightBuffer(const unsigned int pointDensity, const glm::vec3 &movement,
                                   const std::vector<Layer *> &layers) {
    float maxHeight = 0.0F;
    float minHeight = 0.0F;
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);
#define SEQUENTIAL 1
#if SEQUENTIAL
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {

            float generatedHeight = 0.0F;
            for (auto *layer : layers) {
                if (!layer->enabled) {
                    continue;
                }

                float realX = static_cast<float>(x) / static_cast<float>(pointDensity) + movement.x;
                float realY = static_cast<float>(y) / static_cast<float>(pointDensity) + movement.y;
                generatedHeight += layer->getWeightedValue(WIDTH, HEIGHT, glm::vec3(realX, realY, movement.z));
            }

            if (generatedHeight > maxHeight) {
                maxHeight = generatedHeight;
            }
            if (generatedHeight < minHeight) {
                minHeight = generatedHeight;
            }
            heightMap[y * width + x] = generatedHeight;
        }
    }
#else
#pragma omp parallel for reduction(max : maxHeight)
    for (int i = 0; i < width * height; i++) {
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

#pragma omp parallel for
    for (int i = 0; i < heightMap.size(); i++) {
        heightMap[i] -= minHeight;
        heightMap[i] /= maxHeight - minHeight;
        ASSERT(heightMap[i] >= 0.0F);
        ASSERT(heightMap[i] <= 1.0F);
    }
    heightBuffer->update(heightMap);

    unsigned long colorCount = heightMap.size();
    auto textureValues = std::vector<unsigned char>(colorCount);
    for (int i = 0; i < colorCount; i++) {
        float colorValue = heightMap[i] * 255.0F;
        textureValues[i] = static_cast<unsigned char>(colorValue);
    }
    noiseTexture->update(textureValues.data(), width, height, 1);
}
