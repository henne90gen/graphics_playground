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

DEFINE_SCENE_MAIN(Landscape)

DEFINE_SHADER(landscape_NoiseLib)
DEFINE_SHADER(landscape_ScatterLib)

DEFINE_DEFAULT_SHADERS(landscape_Landscape)
DEFINE_TESS_CONTROL_SHADER(landscape_Landscape)
DEFINE_TESS_EVALUATION_SHADER(landscape_Landscape)

DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
DEFINE_DEFAULT_SHADERS(landscape_Texture)

void Landscape::setup() {
    getCamera().setFocalPoint(glm::vec3(0.0F, 150.0F, 0.0F));
    playerCamera.setFocalPoint(glm::vec3(0.0F, 33.0F, 0.0F));

    textureShader = CREATE_DEFAULT_SHADER(landscape_Texture);
    textureShader->bind();
    textureVA = createQuadVA(textureShader);

    flatShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    flatShader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));
    flatShader->bind();
    cubeVA = createCubeVA(flatShader);

    sky.init();
    trees.init();
    terrain.init();
}

void Landscape::destroy() {}

void Landscape::tick() {
    static auto thingToRender = 0;

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
        renderTerrain();
    } else if (thingToRender == 1) {
renderTextureViewer();
    }
}

void Landscape::renderTerrain() {
    static auto movement = glm::vec3(0.0F);
    static auto shaderToggles = ShaderToggles();
    static auto usePlayerPosition = false;

    static auto lightPower = 1.0F;
    static auto lightColor = glm::vec3(1.0F);
    static auto lightPosition = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto lightDirection = glm::vec3(0.0F, 150.0F, 0.0F);

    ImGui::Begin("Settings");
    sky.showGui();
    ImGui::Separator();
    trees.showGui();
    ImGui::Separator();
    if (ImGui::TreeNode("Light Properties")) {
        ImGui::DragFloat3("Light Direction", reinterpret_cast<float *>(&lightDirection));
        ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition));
        ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
        ImGui::DragFloat("Light Power", &lightPower, 0.1F);
        ImGui::TreePop();
    }
    ImGui::Separator();
    ImGui::Checkbox("Wireframe", &shaderToggles.drawWireframe);
    ImGui::Checkbox("Show UVs", &shaderToggles.showUVs);
    ImGui::Checkbox("Show Normals", &shaderToggles.showNormals);
    ImGui::Checkbox("Show Tangents", &shaderToggles.showTangents);
    ImGui::Checkbox("Use Atmospheric Scattering", &shaderToggles.useAtmosphericScattering);
    ImGui::Checkbox("Show Player View", &usePlayerPosition);
    ImGui::Separator();
    terrain.showGui();
    ImGui::End();

    terrain.showLayersGui();

    Camera camera;
    if (usePlayerPosition) {
        playerCamera.update(getInput());
        camera = playerCamera;
    } else {
        camera = getCamera();
    }
    terrain.render(camera.getProjectionMatrix(), camera.getViewMatrix(), lightPosition, lightDirection, lightColor,
                   lightPower, shaderToggles);

    renderLight(camera.getProjectionMatrix(), camera.getViewMatrix(), lightPosition, lightColor);

    trees.render(camera.getProjectionMatrix(), camera.getViewMatrix(), shaderToggles, terrain.terrainParams);

    static float animationTime = 0.0F;
    animationTime += static_cast<float>(getLastFrameTime());
    sky.render(camera.getProjectionMatrix(), camera.getViewMatrix(), animationTime);
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

void Landscape::renderTextureViewer() {
    static auto textureType = 0;
    static auto texturePosition = glm::vec3(0.0F);
    static auto textureZoom = 1.0F;

    ImGui::Begin("Settings");
    static const std::array<const char *, 3> items = {"Grass", "Dirt", "Rock"};
    ImGui::Combo("", &textureType, items.data(), items.size());
    ImGui::Separator();
    ImGui::DragFloat("Zoom", &textureZoom, 0.01F);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&texturePosition), 0.01F);
    ImGui::End();

    if (textureType == 0) {
        renderTexture(texturePosition, textureZoom, terrain.getGrassTexture());
    } else if (textureType == 1) {
        renderTexture(texturePosition, textureZoom, terrain.getDirtTexture());
    } else if (textureType == 2) {
        renderTexture(texturePosition, textureZoom, terrain.getRockTexture());
    }
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
