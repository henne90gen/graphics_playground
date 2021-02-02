#include "Landscape.h"

#include "Main.h"
#include "util/RenderUtils.h"

#include <array>
#include <cmath>
#include <memory>
#include <random>

constexpr unsigned int WIDTH = 10;
constexpr unsigned int HEIGHT = 10;
constexpr int INITIAL_POINT_DENSITY = 15;

DEFINE_SCENE_MAIN(Landscape)

DEFINE_SHADER(landscape_NoiseLib)

DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
DEFINE_DEFAULT_SHADERS(landscape_Texture)
DEFINE_DEFAULT_SHADERS(landscape_Lighting)

float lerp(float a, float b, float f) { return a + f * (b - a); }

void Landscape::setup() {
    getCamera().setFocalPoint(glm::vec3(0.0F, 150.0F, 0.0F));
    playerCamera.setFocalPoint(glm::vec3(0.0F, 33.0F, 0.0F));

    textureShader = CREATE_DEFAULT_SHADER(landscape_Texture);

    flatShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    flatShader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));

    lightingShader = CREATE_DEFAULT_SHADER(landscape_Lighting);

    quadVA = createQuadVA(textureShader);
    cubeVA = createCubeVA(flatShader);

    sky.init();
    trees.init();
    terrain.init();

    initGBuffer();
    initSSAOBuffer();
    initSSAOBlurBuffer();
    initKernelAndNoiseTexture();
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
    ImGui::SameLine();
    if (ImGui::Button("Show G-Buffer")) {
        thingToRender = 2;
    }
    ImGui::Separator();
    ImGui::End();

    if (thingToRender == 0) {
        renderTerrain(true);
    } else if (thingToRender == 1) {
        renderTextureViewer();
    } else if (thingToRender == 2) {
        renderTerrain(false);
        renderGBufferViewer();
    }
}

void Landscape::renderTerrain(const bool renderQuad) {
    static auto movement = glm::vec3(0.0F);
    static auto shaderToggles = ShaderToggles();
    static auto usePlayerPosition = false;

    static auto lightPower = 1.0F;
    static auto lightColor = glm::vec3(1.0F);
    static auto lightPosition = glm::vec3(0.0F, 150.0F, 0.0F);
    static auto sunDirection = glm::vec3(0.0F, 150.0F, 0.0F);

    ImGui::Begin("Settings");
    sky.showGui();
    ImGui::Separator();
    trees.showGui();
    ImGui::Separator();
    if (ImGui::TreeNode("Light Properties")) {
        ImGui::DragFloat3("Sun Direction", reinterpret_cast<float *>(&sunDirection));
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
    ImGui::Checkbox("Use ACESFilm", &shaderToggles.useACESFilm);
    ImGui::Checkbox("Show Player View", &usePlayerPosition);
    ImGui::Separator();
    terrain.showGui();
    ImGui::End();

    terrain.showLayersGui();

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
    GL_Call(glClearColor(0.25, 0.25, 0.25, 1.0));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    Camera camera;
    if (usePlayerPosition) {
        playerCamera.update(getInput());
        camera = playerCamera;
    } else {
        camera = getCamera();
    }
    terrain.render(camera.getProjectionMatrix(), camera.getViewMatrix(), lightPosition, sunDirection, lightColor,
                   lightPower, shaderToggles);

    renderLight(camera.getProjectionMatrix(), camera.getViewMatrix(), lightPosition, lightColor);

    trees.render(camera.getProjectionMatrix(), camera.getViewMatrix(), shaderToggles, terrain.terrainParams);

    static float animationTime = 0.0F;
    animationTime += static_cast<float>(getLastFrameTime());
    sky.render(camera.getProjectionMatrix(), camera.getViewMatrix(), animationTime);

    if (renderQuad) {
        renderGBufferToQuad(lightPosition, lightColor, sunDirection, false);
    }
}

void Landscape::renderGBufferToQuad(const glm::vec3 &lightPosition, const glm::vec3 &lightColor,
                                    const glm::vec3 &sunDirection, bool useAmbientOcclusion) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    lightingShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    lightingShader->setUniform("modelMatrix", modelMatrix);
    lightingShader->setUniform("gPosition", 0);
    lightingShader->setUniform("gNormal", 1);
    lightingShader->setUniform("gAlbedo", 2);
    lightingShader->setUniform("ssao", 3);
    lightingShader->setUniform("useAmbientOcclusion", useAmbientOcclusion);

    const float linear = 0.09F;
    const float quadratic = 0.032F;

    glm::vec3 lightPositionView = glm::vec3(getCamera().getViewMatrix() * glm::vec4(lightPosition, 1.0F));
    lightingShader->setUniform("light.Position", lightPositionView);
    lightingShader->setUniform("light.Color", lightColor);
    lightingShader->setUniform("light.Linear", linear);
    lightingShader->setUniform("light.Quadratic", quadratic);
    glm::vec3 sunDirectionView = glm::normalize(glm::vec3(getCamera().getViewMatrix() * glm::vec4(sunDirection, 1.0F)));
    lightingShader->setUniform("sunDirection", sunDirectionView);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glActiveTexture(GL_TEXTURE2));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glActiveTexture(GL_TEXTURE3));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer));

    quadVA->bind();
    quadVA->setShader(lightingShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void Landscape::renderGBufferViewer() {
    static auto currentTextureIdIndex = 0;
    std::array<unsigned int, 5> textureIds = {gPosition, gNormal, gAlbedo, ssaoColorBuffer, ssaoColorBlurBuffer};
    std::array<const char *, 5> textureIdLabels = {"Position", "Normal", "Albedo", "SSAO", "SSAO Blur"};
    ImGui::Begin("Settings");
    ImGui::Combo("", &currentTextureIdIndex, reinterpret_cast<const char *const *>(&textureIdLabels[0]),
                 textureIdLabels.size());
    ImGui::End();

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    textureShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureSampler", 0);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureIds[currentTextureIdIndex]));

    quadVA->bind();
    quadVA->setShader(textureShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
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
    quadVA->bind();
    quadVA->setShader(textureShader);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    texture->bind();

    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, texturePosition);
    glm::vec3 textureScale = glm::vec3(zoom, zoom * getAspectRatio(), 1.0F);
    modelMatrix = glm::scale(modelMatrix, textureScale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureSampler", 0);

    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void Landscape::initGBuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    // Create framebuffer object
    GL_Call(glGenFramebuffers(1, &gBuffer));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));

    // Create position buffer
    GL_Call(glGenTextures(1, &gPosition));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));

    // Create normal buffer
    GL_Call(glGenTextures(1, &gNormal));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));

    // create color buffer
    GL_Call(glGenTextures(1, &gAlbedo));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

    std::array<unsigned int, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, reinterpret_cast<unsigned int *>(&attachments[0]));

    // Create depth buffer
    GL_Call(glGenRenderbuffers(1, &depthBuffer));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));

    checkFramebufferStatus();
}

void Landscape::initSSAOBuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    GL_Call(glGenFramebuffers(1, &ssaoFbo));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo));

    // SSAO color buffer
    GL_Call(glGenTextures(1, &ssaoColorBuffer));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0));

    checkFramebufferStatus();
}

void Landscape::initSSAOBlurBuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    GL_Call(glGenFramebuffers(1, &ssaoBlurFbo));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFbo));

    // SSAO color blur buffer
    GL_Call(glGenTextures(1, &ssaoColorBlurBuffer));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBlurBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBlurBuffer, 0));

    checkFramebufferStatus();
}

void Landscape::initKernelAndNoiseTexture() {
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0F;

        // scale samples so that they're more aligned to center of kernel
        scale = lerp(0.1F, 1.0F, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0F - 1.0F, randomFloats(generator) * 2.0F - 1.0F,
                        0.0F); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &ssaoNoiseTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Landscape::onAspectRatioChange() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    playerCamera.setViewportSize(static_cast<float>(width), static_cast<float>(height));

    // update position buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update normal buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update color buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    // update depth buffer
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

    // SSAO color buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));

    // SSAO color blur buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBlurBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
}
