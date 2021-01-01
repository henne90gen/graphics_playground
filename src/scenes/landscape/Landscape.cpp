#include "Landscape.h"

#include "ImageOps.h"
#include "Main.h"
#include "util/RenderUtils.h"

#include <array>
#include <cmath>
#include <memory>

constexpr unsigned int WIDTH = 10;
constexpr unsigned int HEIGHT = 10;
constexpr int INITIAL_POINT_DENSITY = 15;

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 10000.0F;

DEFINE_SCENE_MAIN(Landscape)

DEFINE_DEFAULT_SHADERS(landscape_Landscape)
DEFINE_TESS_CONTROL_SHADER(landscape_Landscape)
DEFINE_TESS_EVALUATION_SHADER(landscape_Landscape)

DEFINE_DEFAULT_SHADERS(landscape_FlatColor)

DEFINE_DEFAULT_SHADERS(landscape_NoiseTexture)

void Landscape::setup() {
    textureShader = CREATE_DEFAULT_SHADER(landscape_NoiseTexture);
    textureShader->bind();
    textureVA = createQuadVA(textureShader);

    TextureSettings textureSettings = {};
    textureSettings.dataType = GL_RED;
    noiseTexture = std::make_shared<Texture>(textureSettings);

    normalTexture = std::make_shared<Texture>();

    flatShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    flatShader->bind();
    cubeVA = createCubeVA(flatShader);

    shader = CREATE_DEFAULT_SHADER(landscape_Landscape);
    shader->attachTessControlShader(SHADER_CODE(landscape_LandscapeTcs));
    shader->attachTessEvaluationShader(SHADER_CODE(landscape_LandscapeTes));
    shader->compile();
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    generatePoints();

    Image grassImage;
    if (!ImageOps::load("landscape_resources/textures/Ground037_1K_Color.png", grassImage)) {
        ImageOps::createCheckerBoard(grassImage);
    }

    grassTexture = std::make_shared<Texture>();
    grassImage.applyToTexture(grassTexture);
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void Landscape::destroy() {}

void showLayerMenu(std::vector<NoiseLayer> &layers) {
    ImGui::Begin("Layers");
    int layerToRemove = -1;
    for (int i = 0; i < static_cast<int>(layers.size()); i++) {
        ImGui::Separator();

        const auto btnLabel = "Remove Layer " + std::to_string(i);
        if (ImGui::Button(btnLabel.c_str())) {
            layerToRemove = i;
        }

        ImGui::SameLine();
        const auto enabledLabel = "Enabled " + std::to_string(i);
        ImGui::Checkbox(enabledLabel.c_str(), &layers[i].enabled);

        const auto frequencyLabel = "Frequency " + std::to_string(i);
        ImGui::SliderFloat(frequencyLabel.c_str(), &layers[i].frequency, 0.0F, 500.0F);

        const auto amplitudeLabel = "Amplitude " + std::to_string(i);
        ImGui::SliderFloat(amplitudeLabel.c_str(), &layers[i].amplitude, 0.0F, 100.0F);
    }
    if (layerToRemove >= 0) {
        layers.erase(layers.begin() + layerToRemove);
    }

    ImGui::Separator();
    static int layerType = 0;
    static const std::array<const char *, 2> items = {"Noise", "Ridge"};
    ImGui::Combo("", &layerType, items.data(), items.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Layer")) {
        switch (layerType) {
        case 0:
            layers.push_back(NoiseLayer());
            break;
        default:
            std::cout << "Invalid layer type" << std::endl;
            break;
        }
    }
    ImGui::End();
}

void Landscape::tick() {
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto modelPosition = glm::vec3(0.0F);
    static auto modelRotation = glm::vec3(0.0F);
    static auto cameraPosition = glm::vec3(-100.0F, -200.0F, -100.0F);
    static auto cameraRotation = glm::vec3(0.5F, -0.9F, 0.0F);
    static auto playerPosition = glm::vec3(0.0F, -13.0F, 0.0F);
    static auto playerRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    static auto texturePosition = glm::vec3(0.0F);
    static auto textureRotation = glm::vec3(0.0F);
    static auto textureScale = glm::vec3(1.0F);
    static auto finiteDifference = 0.01F;
    static auto movement = glm::vec3(0.0F);
    static auto shaderToggles = ShaderToggles();
    static auto uvScaleFactor = 20.0F;
    static auto animate = false;
    static auto usePlayerPosition = false;
    static auto thingToRender = 0;
    static auto power = 1.1F;
    static auto platformHeight = 0.15F;

    static auto lightPower = 1.0F;
    static auto lightColor = glm::vec3(1.0F);
    static auto lightPosition = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto lightDirection = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto levels = TerrainLevels();
    static auto tessellation = 60.0F;

    static std::vector<NoiseLayer> layers = {
          NoiseLayer(450.0F, 20.0F), //
          NoiseLayer(300.0F, 15.0F), //
          NoiseLayer(200.0F, 10.0F), //
          NoiseLayer(150.0F, 7.5F),  //
          NoiseLayer(100.0F, 5.0F),  //
          NoiseLayer(80.0F, 4.0F),   //
          NoiseLayer(30.0F, 3.0F),   //
          NoiseLayer(7.5F, 2.0F),    //
    };

    static std::vector<NoiseLayer *> normalLayers = {
          //          new NoiseLayer(1.0F, 1.0F),  //
          //          new NoiseLayer(0.7F, 3.0F),  //
          //          new NoiseLayer(0.5F, 5.0F),  //
          //          new NoiseLayer(0.3F, 7.0F),  //
          //          new NoiseLayer(0.1F, 10.0F), //
    };

    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    if (ImGui::Button("Show Terrain")) {
        thingToRender = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show Texture")) {
        thingToRender = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show Normal Texture")) {
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
    ImGui::DragFloat3("Light Direction", reinterpret_cast<float *>(&lightDirection));
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, 0.1F);
    ImGui::DragFloat3("Terrain Levels", reinterpret_cast<float *>(&levels), 0.001F);
    ImGui::DragFloat("Finite Difference", &finiteDifference, 0.001F);
    ImGui::Separator();
    ImGui::Checkbox("Wireframe", &shaderToggles.drawWireframe);
    ImGui::Checkbox("Show UVs", &shaderToggles.showUVs);
    ImGui::Checkbox("Show Normals", &shaderToggles.showNormals);
    ImGui::Checkbox("Show Tangents", &shaderToggles.showTangents);
    ImGui::Checkbox("Use Finite Differences", &shaderToggles.useFiniteDifferences);
    ImGui::DragFloat("uvScaleFactor", &uvScaleFactor);
    ImGui::Checkbox("Animate", &animate);
    ImGui::Checkbox("Show Player View", &usePlayerPosition);
    ImGui::DragFloat("Power", &power, 0.001F);
    ImGui::SliderFloat("Platform Height", &platformHeight, 0.0F, 1.0F);
    ImGui::Separator();
    ImGui::DragFloat("Tesselation", &tessellation);
    ImGui::End();

    showLayerMenu(layers);

    glm::mat4 viewMatrix;
    if (usePlayerPosition) {
        viewMatrix = createViewMatrix(playerPosition, playerRotation);
    } else {
        viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    }
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    if (thingToRender == 0) {
        renderTerrain(projectionMatrix, viewMatrix, modelPosition, modelRotation, modelScale, lightPosition,
                      lightDirection, lightColor, lightPower, levels, shaderToggles, uvScaleFactor, tessellation,
                      layers, power, finiteDifference);
        renderLight(projectionMatrix, viewMatrix, lightPosition, lightColor);
    } else if (thingToRender == 1) {
        renderNoiseTexture(textureRotation, texturePosition, textureScale);
    } else if (thingToRender == 2) {
        renderNormalTexture(textureRotation, texturePosition, textureScale);
    }
}

void Landscape::renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                              const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                              const glm::vec3 &modelScale, const glm::vec3 &lightPosition,
                              const glm::vec3 &lightDirection, const glm::vec3 &lightColor, float lightPower,
                              const TerrainLevels &levels, const ShaderToggles &shaderToggles, float uvScaleFactor,
                              const float tessellation, const std::vector<NoiseLayer> &layers, const float power,
                              const float finiteDifference) {
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

    glm::mat4 viewModel = inverse(viewMatrix);
    glm::vec3 cameraPosition(viewModel[3] / viewModel[3][3]); // Might have to divide by w if you can't assume w == 1
    shader->setUniform("cameraPosition", cameraPosition);

    shader->setUniform("tessellation", tessellation);

    for (int i = 0; i < static_cast<int64_t>(layers.size()); i++) {
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", layers[i].amplitude);
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", layers[i].frequency);
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", layers[i].enabled);
    }
    shader->setUniform("numNoiseLayers", static_cast<int>(layers.size()));
    shader->setUniform("finiteDifference", finiteDifference);

    shader->setUniform("grassLevel", levels.grassLevel);
    shader->setUniform("rockLevel", levels.rockLevel);
    shader->setUniform("blur", levels.blur);

    shader->setUniform("lightDirection", lightDirection);
    shader->setUniform("lightPosition", lightPosition);
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightPower", lightPower);

    shader->setUniform("showUVs", shaderToggles.showUVs);
    shader->setUniform("showNormals", shaderToggles.showNormals);
    shader->setUniform("showTangents", shaderToggles.showTangents);
    shader->setUniform("useFiniteDifferences", shaderToggles.useFiniteDifferences);
    shader->setUniform("uvScaleFactor", uvScaleFactor);
    shader->setUniform("power", power);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    grassTexture->bind();
    shader->setUniform("grassTexture", 0);

    //    GL_Call(glActiveTexture(GL_TEXTURE1));
    //    normalTexture->bind();
    //    shader->setUniform("normalSampler", 1);

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glPatchParameteri(GL_PATCH_VERTICES, 3));
    GL_Call(glDrawElements(GL_PATCHES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    vertexArray->unbind();
    shader->unbind();
}

void Landscape::renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                            const glm::vec3 &lightPosition, const glm::vec3 &lightColor) {
    cubeVA->bind();
    flatShader->bind();
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, lightPosition);
    flatShader->setUniform("modelMatrix", modelMatrix);
    flatShader->setUniform("viewMatrix", viewMatrix);
    flatShader->setUniform("projectionMatrix", projectionMatrix);
    flatShader->setUniform("flatColor", lightColor);
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
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

void Landscape::generatePoints() {
    std::vector<float> quadVertices = {
          0.0F, 0.0F, //
          1.0F, 0.0F, //
          1.0F, 1.0F, //
          0.0F, 1.0F, //
    };

    // generate a 10x10 grid of points in the range of -500 to 500
    int width = 10;
    int height = 10;
    std::vector<float> vertices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            for (int i = 0; i < static_cast<int64_t>(quadVertices.size()); i++) {
                float f = quadVertices[i];
                if (i % 2 == 0) {
                    f += static_cast<float>(row);
                    f -= 5.0F;
                    f *= 100.0F;
                } else if (i % 2 == 1) {
                    f += static_cast<float>(col);
                    f -= 5.0F;
                    f *= 100.0F;
                }
                vertices.push_back(f);
            }
        }
    }

    BufferLayout bufferLayout = {
          {ShaderDataType::Vec2, "position_in"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int i = (row * width + col) * 4;
            indices.emplace_back(i, i + 1, i + 2);
            indices.emplace_back(i, i + 2, i + 3);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);
}
