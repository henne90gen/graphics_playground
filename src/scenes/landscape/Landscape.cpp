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
DEFINE_DEFAULT_SHADERS(landscape_Texture)
DEFINE_DEFAULT_SHADERS(landscape_Tree)

void Landscape::setup() {
    treeShader = CREATE_DEFAULT_SHADER(landscape_Tree);
    textureShader = CREATE_DEFAULT_SHADER(landscape_Texture);
    textureShader->bind();
    textureVA = createQuadVA(textureShader);

    flatShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    flatShader->bind();
    cubeVA = createCubeVA(flatShader);

    terrainShader = CREATE_DEFAULT_SHADER(landscape_Landscape);
    terrainShader->attachTessControlShader(SHADER_CODE(landscape_LandscapeTcs));
    terrainShader->attachTessEvaluationShader(SHADER_CODE(landscape_LandscapeTes));
    terrainShader->compile();
    terrainShader->bind();

    terrainVA = generatePoints(terrainShader);

    initTextures();
    initModels();

    sky.init();
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
    static auto thingToRender = 0;
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto modelPosition = glm::vec3(0.0F);
    static auto modelRotation = glm::vec3(0.0F);
    static auto cameraPosition = glm::vec3(-100.0F, -150.0F, -100.0F);
    static auto cameraRotation = glm::vec3(0.5F, -0.9F, 0.0F);
    static auto playerPosition = glm::vec3(0.0F, -33.0F, 0.0F);
    static auto playerRotation = glm::vec3(-0.13F, 0.95F, 0.0F);
    static auto finiteDifference = 0.01F;
    static auto movement = glm::vec3(0.0F);
    static auto shaderToggles = ShaderToggles();
    static auto uvScaleFactor = 20.0F;
    static auto animate = false;
    static auto usePlayerPosition = false;
    static auto power = 1.1F;
    static auto bowlStrength = 20.0F;
    static auto platformHeight = 0.15F;

    static auto lightPower = 1.0F;
    static auto lightColor = glm::vec3(1.0F);
    static auto lightPosition = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto lightDirection = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto levels = TerrainLevels();
    static auto tessellation = 60.0F;
    static auto treeCount = 10000;

    static std::vector<NoiseLayer> layers = {
          NoiseLayer(450.0F, 20.0F), //
          NoiseLayer(300.0F, 15.0F), //
          NoiseLayer(200.0F, 10.0F), //
          NoiseLayer(150.0F, 7.5F),  //
          NoiseLayer(100.0F, 5.0F),  //
          NoiseLayer(80.0F, 4.0F),   //
          NoiseLayer(30.0F, 2.0F),   //
          NoiseLayer(7.5F, 0.75F),   //
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
    ImGui::Separator();
    ImGui::End();

    if (thingToRender == 0) {
        ImGui::Begin("Settings");
        sky.showGui();
        ImGui::Separator();
        ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);
        ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), dragSpeed);
        ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
        ImGui::Separator();
        ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition));
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
        ImGui::DragFloat("UV Scale", &uvScaleFactor);
        ImGui::Checkbox("Animate", &animate);
        ImGui::Checkbox("Show Player View", &usePlayerPosition);
        ImGui::DragFloat("Power", &power, 0.001F);
        ImGui::DragFloat("Bowl Strength", &bowlStrength, 0.1F);
        ImGui::SliderFloat("Platform Height", &platformHeight, 0.0F, 1.0F);
        ImGui::Separator();
        ImGui::DragFloat("Tesselation", &tessellation);
        ImGui::DragInt("Tree Count", &treeCount);
        ImGui::End();

        showLayerMenu(layers);

        glm::mat4 viewMatrix;
        if (usePlayerPosition) {
            viewMatrix = createViewMatrix(playerPosition, playerRotation);
        } else {
            viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
        }
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

        renderTerrain(projectionMatrix, viewMatrix, modelPosition, modelRotation, modelScale, lightPosition,
                      lightDirection, lightColor, lightPower, levels, shaderToggles, uvScaleFactor, tessellation,
                      layers, power, bowlStrength, finiteDifference, platformHeight);
        renderLight(projectionMatrix, viewMatrix, lightPosition, lightColor);
        renderTrees(projectionMatrix, viewMatrix, treeCount, layers, shaderToggles, finiteDifference, power,
                    bowlStrength, platformHeight);

        static float animationTime = 0.0F;
        animationTime += static_cast<float>(getLastFrameTime());
        sky.updateAndRender(cubeVA, projectionMatrix, viewMatrix, animationTime);
    } else if (thingToRender == 1) {
        static auto textureType = 0;
        static auto texturePosition = glm::vec3(0.0F);
        static auto textureZoom = 1.0F;

        ImGui::Begin("Settings");
        static const std::array<const char *, 3> items = {"Grass", "Dirt", "Rock"};
        ImGui::Combo("", &textureType, items.data(), items.size());
        ImGui::Separator();
        ImGui::DragFloat("Zoom", &textureZoom, dragSpeed);
        ImGui::DragFloat3("Position", reinterpret_cast<float *>(&texturePosition), dragSpeed);
        ImGui::End();

        if (textureType == 0) {
            renderTexture(texturePosition, textureZoom, grassTexture);
        } else if (textureType == 1) {
            renderTexture(texturePosition, textureZoom, dirtTexture);
        } else if (textureType == 2) {
            renderTexture(texturePosition, textureZoom, rockTexture);
        }
    }
}

void Landscape::renderTerrain(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                              const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                              const glm::vec3 &modelScale, const glm::vec3 &lightPosition,
                              const glm::vec3 &lightDirection, const glm::vec3 &lightColor, const float lightPower,
                              const TerrainLevels &levels, const ShaderToggles &shaderToggles,
                              const float uvScaleFactor, const float tessellation,
                              const std::vector<NoiseLayer> &noiseLayers, const float power, const float bowlStrength,
                              const float finiteDifference, const float platformHeight) {
    terrainShader->bind();
    terrainVA->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, modelPosition);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    terrainShader->setUniform("modelMatrix", modelMatrix);
    terrainShader->setUniform("viewMatrix", viewMatrix);
    terrainShader->setUniform("projectionMatrix", projectionMatrix);
    terrainShader->setUniform("normalMatrix", normalMatrix);

    glm::mat4 viewModel = inverse(viewMatrix);
    glm::vec3 cameraPosition(viewModel[3] / viewModel[3][3]); // Might have to divide by w if you can't assume w == 1
    terrainShader->setUniform("cameraPosition", cameraPosition);

    terrainShader->setUniform("tessellation", tessellation);

    for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
        terrainShader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
        terrainShader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
        terrainShader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
    }
    terrainShader->setUniform("numNoiseLayers", static_cast<int>(noiseLayers.size()));
    terrainShader->setUniform("finiteDifference", finiteDifference);
    terrainShader->setUniform("useFiniteDifferences", shaderToggles.useFiniteDifferences);
    terrainShader->setUniform("power", power);
    terrainShader->setUniform("bowlStrength", bowlStrength);
    terrainShader->setUniform("platformHeight", platformHeight);

    terrainShader->setUniform("grassLevel", levels.grassLevel);
    terrainShader->setUniform("rockLevel", levels.rockLevel);
    terrainShader->setUniform("blur", levels.blur);

    terrainShader->setUniform("lightDirection", lightDirection);
    terrainShader->setUniform("lightPosition", lightPosition);
    terrainShader->setUniform("lightColor", lightColor);
    terrainShader->setUniform("lightPower", lightPower);

    terrainShader->setUniform("showUVs", shaderToggles.showUVs);
    terrainShader->setUniform("showNormals", shaderToggles.showNormals);
    terrainShader->setUniform("showTangents", shaderToggles.showTangents);
    terrainShader->setUniform("uvScaleFactor", uvScaleFactor);

    terrainShader->setUniform("grassTexture", grassTexture, GL_TEXTURE0);
    terrainShader->setUniform("dirtTexture", dirtTexture, GL_TEXTURE1);
    terrainShader->setUniform("rockTexture", rockTexture, GL_TEXTURE2);

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glPatchParameteri(GL_PATCH_VERTICES, 3));
    GL_Call(glDrawElements(GL_PATCHES, terrainVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    terrainVA->unbind();
    terrainShader->unbind();
}

void Landscape::renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                            const glm::vec3 &lightPosition, const glm::vec3 &lightColor) {
    cubeVA->bind();
    flatShader->bind();
    cubeVA->setShader(flatShader);
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, lightPosition);
    flatShader->setUniform("modelMatrix", modelMatrix);
    flatShader->setUniform("viewMatrix", viewMatrix);
    flatShader->setUniform("projectionMatrix", projectionMatrix);
    flatShader->setUniform("flatColor", lightColor);
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void Landscape::renderTrees(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const int treeCount,
                            const std::vector<NoiseLayer> &noiseLayers, const ShaderToggles &shaderToggles,
                            const float finiteDifference, const float power, const float bowlStrength,
                            const float platformHeight) {
#if 1
    cubeVA->bind();
    treeShader->bind();
    cubeVA->setShader(treeShader);
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    treeShader->setUniform("modelMatrix", modelMatrix);
    treeShader->setUniform("viewMatrix", viewMatrix);
    treeShader->setUniform("projectionMatrix", projectionMatrix);

    for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
    }
    treeShader->setUniform("numNoiseLayers", static_cast<int>(noiseLayers.size()));
    treeShader->setUniform("finiteDifference", finiteDifference);
    treeShader->setUniform("useFiniteDifferences", shaderToggles.useFiniteDifferences);
    treeShader->setUniform("power", power);
    treeShader->setUniform("bowlStrength", bowlStrength);
    treeShader->setUniform("platformHeight", platformHeight);

    treeShader->setUniform("treeCount", treeCount);

    GL_Call(glDrawElementsInstanced(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
                                    treeCount));
#else
    if (!treeModel) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    static auto scale = 0.1F;
    ImGui::Begin("Settings");
    ImGui::DragFloat("Scale", &scale, 0.001);
    ImGui::End();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    treeShader->bind();
    treeShader->setUniform("modelMatrix", modelMatrix);
    treeShader->setUniform("viewMatrix", viewMatrix);
    treeShader->setUniform("projectionMatrix", projectionMatrix);

    for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
    }
    treeShader->setUniform("numNoiseLayers", static_cast<int>(noiseLayers.size()));
    treeShader->setUniform("finiteDifference", finiteDifference);
    treeShader->setUniform("useFiniteDifferences", shaderToggles.useFiniteDifferences);
    treeShader->setUniform("power", power);
    treeShader->setUniform("bowlStrength", bowlStrength);
    treeShader->setUniform("platformHeight", platformHeight);

    treeShader->setUniform("treeCount", treeCount);

    //    treeShader->setUniform("u_TextureSampler", 0);

    for (const auto &mesh : treeModel->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->terrainVA->bind();

        mesh->texture->bind();

        //        if (drawWireframe) {
        //            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        //        }

        GL_Call(glDrawElementsInstanced(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr,
                                        treeCount));
        //        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        //        if (drawWireframe) {
        //            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        //        }

        mesh->terrainVA->unbind();
    }
#endif
}

void Landscape::renderTexture(const glm::vec3 &texturePosition, const float zoom,
                              const std::shared_ptr<Texture> &texture) {
    textureShader->bind();
    textureVA->bind();

    GL_Call(glActiveTexture(GL_TEXTURE0));
    texture->bind();

    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, texturePosition);
    glm::vec3 textureScale = glm::vec3(zoom, zoom * getAspectRatio(), 1.0F);
    modelMatrix = glm::scale(modelMatrix, textureScale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureSampler", 0);

    GL_Call(glDrawElements(GL_TRIANGLES, textureVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    textureVA->unbind();
    textureShader->unbind();
}

std::shared_ptr<VertexArray> Landscape::generatePoints(const std::shared_ptr<Shader> &shader) {
    auto result = std::make_shared<VertexArray>(shader);

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
    result->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int i = (row * width + col) * 4;
            indices.emplace_back(i, i + 1, i + 2);
            indices.emplace_back(i, i + 2, i + 3);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(indexBuffer);

    return result;
}

std::shared_ptr<Texture> createTextureFromImage(const Image &image) {
    auto texture = std::make_shared<Texture>();
    image.applyToTexture(texture);

    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    return texture;
}

std::shared_ptr<Texture> loadTexture(const std::string &fileName) {
    Image image;
    if (!ImageOps::load("landscape_resources/assets/textures/" + fileName, image)) {
        ImageOps::createCheckerBoard(image);
    }

    return createTextureFromImage(image);
}

void Landscape::initTextures() {
    RECORD_SCOPE();
    const std::array<std::string, 3> fileNames = {
          "Ground037_1K_Color.png", //
          "Ground039_1K_Color.png", //
          "Ground022_1K_Color.png", //
    };

#define LOAD_TEXTURES_PARALLEL 1
#if LOAD_TEXTURES_PARALLEL
    std::array<Image, 3> images = {};
#pragma omp parallel for
    for (int i = 0; i < images.size(); i++) {
        if (!ImageOps::load("landscape_resources/textures/" + fileNames[i], images[i])) {
            ImageOps::createCheckerBoard(images[i]);
        }
    }

    grassTexture = createTextureFromImage(images[0]);
    dirtTexture = createTextureFromImage(images[1]);
    rockTexture = createTextureFromImage(images[2]);
#else
    grassTexture = loadTexture(fileNames[0]);
    dirtTexture = loadTexture(fileNames[1]);
    rockTexture = loadTexture(fileNames[2]);
#endif
}

void Landscape::initModels() {
#if 0
    treeModel = std::make_shared<Model>();
    treeModel->loadFromFile("landscape_resources/assets/models/Tree1/Tree1.obj", treeShader);
#endif
}
