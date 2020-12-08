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
    textureShader = SHADER(landscape_NoiseTexture);
    textureShader->bind();
    textureVA = createQuadVA(textureShader);

    TextureSettings textureSettings = {};
    textureSettings.dataType = GL_RED;
    noiseTexture = std::make_shared<Texture>(textureSettings);

    normalTexture = std::make_shared<Texture>();

    shader = SHADER(landscape_Landscape);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    generatePoints(INITIAL_POINT_DENSITY);
}

void Landscape::destroy() {}

void showLayerMenu(std::vector<Layer *> &layers) {
    ImGui::Begin("Layers");
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
    static int layerType = 0;
    static const std::array<const char *, 3> items = {"Bowl", "Noise", "Ridge"};
    ImGui::Combo("", &layerType, items.data(), items.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Layer")) {
        switch (layerType) {
        case 0:
            layers.push_back(new BowlLayer());
            break;
        case 1:
            layers.push_back(new NoiseLayer());
            break;
        case 2:
            layers.push_back(new RidgeNoiseLayer());
            break;
        default:
            std::cout << "Invalid layer type" << std::endl;
            break;
        }
    }
    ImGui::End();
}

void Landscape::tick() {
    constexpr float timeSpeed = 0.01F;
    static auto modelScale = glm::vec3(50.0F, 50.0F, 50.0F);
    static auto modelPosition = glm::vec3(0.0F);
    static auto modelRotation = glm::vec3(0.0F);
    static auto cameraPosition = glm::vec3(0.0F, -200.0F, -160.0F);
    static auto cameraRotation = glm::vec3(0.55F, 0.0F, 0.0F);
    static auto playerPosition = glm::vec3(0.0F, -13.0F, 0.0F);
    static auto playerRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    static auto texturePosition = glm::vec3(0.0F);
    static auto textureRotation = glm::vec3(0.0F);
    static auto textureScale = glm::vec3(1.0F);
    static auto pointDensity = INITIAL_POINT_DENSITY;
    static auto movement = glm::vec3(0.0F);
    static auto drawWireframe = false;
    static auto showUVs = false;
    static auto animate = false;
    static auto usePlayerPosition = false;
    static auto thingToRender = 0;
    static auto power = 1.1F;
    static auto platformHeight = 0.15F;

    static auto lightPower = 100.0F;
    static auto lightColor = glm::vec3(1.0F);
    static auto surfaceToLight = glm::vec3(-100.0F, 10.0F, 0.0F);
    static auto levels = TerrainLevels();

    int lastPointDensity = pointDensity;

    static std::vector<Layer *> layers = {
          new BowlLayer(0.04F),              //
          new NoiseLayer(1.0F, 0.1F),        //
          new NoiseLayer(0.5F, 0.3F),        //
          new NoiseLayer(0.35F, 0.5F),       //
          new NoiseLayer(0.2F, 0.7F),        //
          new NoiseLayer(0.1F, 0.9F),        //
          new NoiseLayer(0.05F, 2.5F),       //
          new NoiseLayer(0.025F, 5.0F),      //
          new NoiseLayer(0.01F, 10.0F),      //
          new RidgeNoiseLayer(0.23F, 0.37F), //
    };

    static std::vector<Layer *> normalLayers = {
          new NoiseLayer(1.0F, 10.0F, 5.0F, FastNoise::NoiseType::Cellular), //
          new NoiseLayer(1.0F, 10.0F),                                       //
    };

    const float dragSpeed = 0.01F;
    const int minimumPointDensity = 1;
    const int maximumPointDensity = 80;
    ImGui::Begin("Settings");
    if (ImGui::Button("Show Terrain")) {
        thingToRender = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show Texture")) {
        thingToRender = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show Normals")) {
        thingToRender = 2;
    }
    ImGui::Separator();
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), dragSpeed);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Texture Scale", reinterpret_cast<float *>(&textureScale), dragSpeed);
    ImGui::DragFloat3("Texture Position", reinterpret_cast<float *>(&texturePosition), dragSpeed);
    ImGui::DragFloat3("Texture Rotation", reinterpret_cast<float *>(&textureRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Player Position", reinterpret_cast<float *>(&playerPosition), dragSpeed);
    ImGui::DragFloat3("Player Rotation", reinterpret_cast<float *>(&playerRotation), dragSpeed);
    ImGui::Separator();
    ImGui::DragFloat3("Noise Offset", reinterpret_cast<float *>(&movement), dragSpeed);
    ImGui::SliderInt("Point Density", &pointDensity, minimumPointDensity, maximumPointDensity);
    ImGui::Text("Point count: %d", pointDensity * pointDensity * WIDTH * HEIGHT);
    ImGui::Separator();
    ImGui::DragFloat3("Surface To Light", reinterpret_cast<float *>(&surfaceToLight));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, 0.1F);
    ImGui::DragFloat3("Terrain Levels", reinterpret_cast<float *>(&levels), dragSpeed);
    ImGui::Separator();
    ImGui::Checkbox("Wireframe", &drawWireframe);
    ImGui::Checkbox("Show UVs", &showUVs);
    ImGui::Checkbox("Animate", &animate);
    ImGui::Checkbox("Show Player View", &usePlayerPosition);
    ImGui::DragFloat("Power", &power, dragSpeed);
    ImGui::SliderFloat("Platform Height", &platformHeight, 0.0F, 1.0F);
    ImGui::End();

    if (thingToRender == 0 || thingToRender == 1) {
        showLayerMenu(layers);
    } else {
        showLayerMenu(normalLayers);
    }

    if (animate) {
        movement.z += timeSpeed;
    }

    if (lastPointDensity != pointDensity) {
        generatePoints(pointDensity);
    }

    static bool init = false;
    if (!init) {
        init = true;
        updateHeightBuffer(pointDensity, movement, layers, power, platformHeight);
        updateNormalTexture(pointDensity * 10, movement, normalLayers, power);
    }

    glm::mat4 viewMatrix;
    if (usePlayerPosition) {
        viewMatrix = createViewMatrix(playerPosition, playerRotation);
    } else {
        viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    }
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    if (thingToRender == 0) {
        renderTerrain(projectionMatrix, viewMatrix, modelPosition, modelRotation, modelScale, surfaceToLight,
                      lightColor, lightPower, levels, drawWireframe, showUVs);
    } else if (thingToRender == 1) {
        renderNoiseTexture(textureRotation, texturePosition, textureScale);
    } else if (thingToRender == 2) {
        renderNormalTexture(textureRotation, texturePosition, textureScale);
    }
}

void Landscape::renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                              const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                              const glm::vec3 &modelScale, const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor,
                              const float lightPower, const TerrainLevels &levels, const bool drawWireframe,
                              const bool showUVs) {
    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, modelPosition);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("normalMatrix", normalMatrix);

    shader->setUniform("grassLevel", levels.grassLevel);
    shader->setUniform("rockLevel", levels.rockLevel);
    shader->setUniform("blur", levels.blur);
    shader->setUniform("surfaceToLight", surfaceToLight);
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightPower", lightPower);
    shader->setUniform("showUVs", showUVs);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    normalTexture->bind();
    shader->setUniform("uNormalSampler", 0);

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

void Landscape::renderNoiseTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                                   glm::vec3 &textureScale) {
    textureShader->bind();
    textureVA->bind();

    GL_Call(glActiveTexture(GL_TEXTURE0));
    noiseTexture->bind();

    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, texturePosition);
    modelMatrix = glm::rotate(modelMatrix, textureRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.z, glm::vec3(0, 0, 1));
    textureScale.y = textureScale.x * getAspectRatio();
    modelMatrix = glm::scale(modelMatrix, textureScale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureSampler", 0);
    textureShader->setUniform("numChannels", 1);

    GL_Call(glDrawElements(GL_TRIANGLES, textureVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    textureVA->unbind();
    textureShader->unbind();
}

void Landscape::renderNormalTexture(const glm::vec3 &textureRotation, const glm::vec3 &texturePosition,
                                    glm::vec3 &textureScale) {
    textureShader->bind();
    textureVA->bind();

    GL_Call(glActiveTexture(GL_TEXTURE0));
    normalTexture->bind();

    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, texturePosition);
    modelMatrix = glm::rotate(modelMatrix, textureRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, textureRotation.z, glm::vec3(0, 0, 1));
    textureScale.y = textureScale.x * getAspectRatio();
    modelMatrix = glm::scale(modelMatrix, textureScale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureSampler", 0);
    textureShader->setUniform("numChannels", 3);

    GL_Call(glDrawElements(GL_TRIANGLES, textureVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    textureVA->unbind();
    textureShader->unbind();
}

void Landscape::generatePoints(unsigned int pointDensity) {
    vertexArray->bind();

    const auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    const auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    const unsigned int verticesCount = width * height;
    vertices = std::vector<glm::vec2>(verticesCount);
    uvs = std::vector<glm::vec2>(verticesCount);
#pragma omp parallel for
    for (long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % width);
        x /= static_cast<float>(pointDensity);
        x -= static_cast<float>(WIDTH) / 2.0F;

        auto y = std::floor(static_cast<float>(i) / static_cast<float>(width));
        y /= static_cast<float>(pointDensity);
        y -= static_cast<float>(HEIGHT) / 2.0F;

        vertices[i] = glm::vec2(x, y);

        auto u = static_cast<float>((i % width) % 2);
        auto row = static_cast<int>(std::floor(static_cast<float>(i) / static_cast<float>(width)));
        auto v = static_cast<float>((row + 1) % 2);
        uvs[i] = glm::vec2(u, v);
    }

    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    BufferLayout uvLayout = {{ShaderDataType::Float2, "uv"}};
    auto uvBuffer = std::make_shared<VertexBuffer>(uvs, uvLayout);
    vertexArray->addVertexBuffer(uvBuffer);

    BufferLayout tangentLayout = {{ShaderDataType::Float3, "tangent"}};
    tangentBuffer = std::make_shared<VertexBuffer>(tangentLayout);
    vertexArray->addVertexBuffer(tangentBuffer);
    BufferLayout biTangentLayout = {{ShaderDataType::Float3, "biTangent"}};
    biTangentBuffer = std::make_shared<VertexBuffer>(biTangentLayout);
    vertexArray->addVertexBuffer(biTangentBuffer);

    const unsigned int heightMapCount = width * height;
    heightMap = std::vector<float>(heightMapCount);
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    vertexArray->addVertexBuffer(heightBuffer);

    normalBuffer = std::make_shared<VertexBuffer>();
    BufferLayout normalLayout = {{ShaderDataType::Float3, "normal"}};
    normalBuffer->setLayout(normalLayout);
    vertexArray->addVertexBuffer(normalBuffer);

    const unsigned int trianglesPerQuad = 2;
    unsigned int indicesCount = width * height * trianglesPerQuad;
    indices = std::vector<glm::ivec3>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < height - 1; y++) {
        for (unsigned int x = 0; x < width - 1; x++) {
            indices[counter++] = glm::ivec3((y + 1) * width + x, y * width + (x + 1), y * width + x);
            indices[counter++] = glm::ivec3((y + 1) * width + x, (y + 1) * width + (x + 1), y * width + (x + 1));
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);
}

void updateNoiseTexture(const std::shared_ptr<Texture> &noiseTexture, const std::vector<float> &heightMap, int width,
                        int height) {
    unsigned long colorCount = heightMap.size();
    auto textureValues = std::vector<unsigned char>(colorCount);
#pragma omp parallel for
    for (int i = 0; i < colorCount; i++) {
        float colorValue = heightMap[i] * 255.0F;
        textureValues[i] = static_cast<unsigned char>(colorValue);
    }
    noiseTexture->update(textureValues.data(), width, height, 1);
}

float safeRetrieve(const std::vector<float> &heightMap, unsigned int width, int x, int y) {
    int i = y * static_cast<int>(width) + x;
    if (i < 0 || i >= heightMap.size()) {
        return 0.0F;
    }
    return heightMap[i];
}

void updateNormals(const std::shared_ptr<VertexBuffer> &normalBuffer, const std::vector<float> &heightMap,
                   const int width) {
    auto normals = std::vector<glm::vec3>(heightMap.size());
#pragma omp parallel for
    for (int i = 0; i < heightMap.size(); i++) {
        int x = i % width;
        int y = i / width;
        const float L = safeRetrieve(heightMap, width, x - 1, y);
        const float R = safeRetrieve(heightMap, width, x + 1, y);
        const float B = safeRetrieve(heightMap, width, x, y - 1);
        const float T = safeRetrieve(heightMap, width, x, y + 1);
        normals[i] = glm::normalize(glm::vec3(2 * (L - R), 4, 2 * (B - T)));
    }
    normalBuffer->update(normals);
}

void evaluateNoiseLayers(std::vector<float> &heightMap, const std::vector<Layer *> &layers, const unsigned int width,
                         const unsigned int height, const glm::vec3 &movement, const unsigned int pointDensity,
                         const float power, const float platformHeight) {
    float maxHeight = 0.0F;
    float minHeight = 0.0F;

#define SEQUENTIAL 1
#if SEQUENTIAL
#else
#pragma omp parallel for reduction(max : maxHeight), reduction(min : minHeight)
#endif
    for (int i = 0; i < width * height; i++) {
        unsigned int x = i % width;
        unsigned int y = i / width;

        float generatedHeight = 0.0F;
        for (auto *layer : layers) {
            if (!layer->enabled) {
                continue;
            }

            float realX = static_cast<float>(x) / static_cast<float>(pointDensity) + movement.x;
            float realY = static_cast<float>(y) / static_cast<float>(pointDensity) + movement.y;
            generatedHeight +=
                  layer->getWeightedValue(WIDTH, HEIGHT, glm::vec3(realX, realY, movement.z), generatedHeight);
        }

        generatedHeight = std::pow(generatedHeight, power);
        if (std::isnan(generatedHeight)) {
            generatedHeight = 0.0F;
        }

        if (generatedHeight > maxHeight) {
            maxHeight = generatedHeight;
        }
        if (generatedHeight < minHeight) {
            minHeight = generatedHeight;
        }
        heightMap[y * width + x] = generatedHeight;
    }

#pragma omp parallel for
    for (int i = 0; i < heightMap.size(); i++) {
        heightMap[i] -= minHeight;
        heightMap[i] /= maxHeight - minHeight;

        if (platformHeight <= 0.0F) {
            continue;
        }

        unsigned int x = i % width;
        unsigned int y = i / width;

        float cx = static_cast<float>(width) / 2.0F;
        float cy = static_cast<float>(height) / 2.0F;
        float platform = static_cast<float>(width) / 15.0F;
        float smoothing = static_cast<float>(width) / 20.0F;
        float posM = 1.0F / smoothing;
        float posN = 0.0F - posM * (cx - platform - smoothing);
        float negM = -1.0F / smoothing;
        float negN = 0.0F - negM * (cx + platform + smoothing);
        float w = 1.0F;
        if (x < cx - platform - smoothing || x > cx + platform + smoothing || //
            y < cy - platform - smoothing || y > cy + platform + smoothing) {
            w = 0.0F;
        } else if (x > cx - platform && x < cx + platform && //
                   y > cy - platform && y < cy + platform) {
            w = 1.0F;
        } else {
            bool isLeftEdge = x <= cx - platform && x >= cx - platform - smoothing;
            bool isRightEdge = x >= cx + platform && x <= cx + platform + smoothing;
            bool isTopEdge = y >= cy + platform && y <= cy + platform + smoothing;
            bool isBottomEdge = y <= cy - platform && y >= cy - platform - smoothing;
            if (isLeftEdge) {
                w *= posM * x + posN;
            } else if (isRightEdge) {
                w *= negM * x + negN;
            }
            if (isBottomEdge) {
                w *= posM * y + posN;
            } else if (isTopEdge) {
                w *= negM * y + negN;
            }
        }
        heightMap[i] = (1.0F - w) * heightMap[i] + w * platformHeight;
    }
}

void updateTangentAndBiTangent(const std::vector<glm::ivec3> &indices, const std::vector<glm::vec2> &vertices,
                               const std::vector<float> &heights, const std::vector<glm::vec2> &uvs,
                               std::vector<glm::vec3> &tangents, std::vector<glm::vec3> &biTangents) {
    for (const auto &index : indices) {
        auto p1_ = vertices[index.x];
        auto p2_ = vertices[index.y];
        auto p3_ = vertices[index.z];
        auto p1 = glm::vec3(p1_.x, heights[index.x], p1_.y);
        auto p2 = glm::vec3(p2_.x, heights[index.y], p2_.y);
        auto p3 = glm::vec3(p3_.x, heights[index.z], p3_.y);
        auto uv1 = uvs[index.x];
        auto uv2 = uvs[index.y];
        auto uv3 = uvs[index.z];

        auto edge1 = p2 - p1;
        auto edge2 = p3 - p1;
        auto deltaUV1 = uv2 - uv1;
        auto deltaUV2 = uv3 - uv1;

        float f = 1.0F / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = {0.0, 0.0, 0.0};
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);

        glm::vec3 biTangent = {0.0, 0.0, 0.0};
        biTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        biTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        biTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        biTangent = glm::normalize(biTangent);
        biTangents.push_back(biTangent);
        biTangents.push_back(biTangent);
        biTangents.push_back(biTangent);
    }
}

void Landscape::updateHeightBuffer(const unsigned int pointDensity, const glm::vec3 &movement,
                                   const std::vector<Layer *> &layers, const float power, const float platformHeight) {
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    evaluateNoiseLayers(heightMap, layers, width, height, movement, pointDensity, power, platformHeight);
    heightBuffer->update(heightMap);

    updateNoiseTexture(noiseTexture, heightMap, width, height);
    updateNormals(normalBuffer, heightMap, width);

    auto tangents = std::vector<glm::vec3>();
    auto biTangents = std::vector<glm::vec3>();
    updateTangentAndBiTangent(indices, vertices, heightMap, uvs, tangents, biTangents);
    tangentBuffer->update(tangents);
    biTangentBuffer->update(biTangents);
}

void Landscape::updateNormalTexture(const unsigned int pointDensity, const glm::vec3 &movement,
                                    const std::vector<Layer *> &layers, const float power) {
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    auto heightMap = std::vector<float>(width * height);
    evaluateNoiseLayers(heightMap, layers, width, height, movement, pointDensity, power, 0.0F);

    auto normals = std::vector<glm::vec3>(heightMap.size());
#pragma omp parallel for
    for (int i = 0; i < heightMap.size(); i++) {
        const int x = i % width;
        const int y = i / width;
        const float L = safeRetrieve(heightMap, width, x - 1, y);
        const float R = safeRetrieve(heightMap, width, x + 1, y);
        const float B = safeRetrieve(heightMap, width, x, y - 1);
        const float T = safeRetrieve(heightMap, width, x, y + 1);
        const glm::vec3 normal = glm::normalize(glm::vec3(2 * (L - R), 4, 2 * (B - T)));
        normals[i] = normal;
    }
    normalTexture->update(normals, width, height);
}
