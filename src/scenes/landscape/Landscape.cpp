#include "Landscape.h"

#include "Main.h"
#include "util/RenderUtils.h"

#include <array>
#include <cmath>
#include <random>

constexpr unsigned int WIDTH = 10;
constexpr unsigned int HEIGHT = 10;
constexpr int INITIAL_POINT_DENSITY = 15;

DEFINE_SCENE_MAIN(Landscape)

DEFINE_SHADER(landscape_NoiseLib)
DEFINE_SHADER(landscape_ScatterLib)

DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
DEFINE_DEFAULT_SHADERS(landscape_Texture)

DEFINE_VERTEX_SHADER(landscape_ScreenQuad)
DEFINE_FRAGMENT_SHADER(landscape_Lighting)
DEFINE_FRAGMENT_SHADER(landscape_SSAO)
DEFINE_FRAGMENT_SHADER(landscape_SSAOBlur)

float lerp(float a, float b, float f) { return a + f * (b - a); }

void Landscape::setup() {
    getCamera().setFocalPoint(glm::vec3(0.0F, 150.0F, 0.0F));
    playerCamera.setFocalPoint(glm::vec3(0.0F, 33.0F, 0.0F));

    flatShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    textureShader = CREATE_DEFAULT_SHADER(landscape_Texture);

    lightingShader = createDefaultShader(SHADER_CODE(landscape_ScreenQuadVert), SHADER_CODE(landscape_LightingFrag));
    lightingShader->attachShaderLib(SHADER_CODE(landscape_ScatterLib));
    ssaoShader = createDefaultShader(SHADER_CODE(landscape_ScreenQuadVert), SHADER_CODE(landscape_SSAOFrag));
    ssaoBlurShader = createDefaultShader(SHADER_CODE(landscape_ScreenQuadVert), SHADER_CODE(landscape_SSAOBlurFrag));

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
    static auto usePlayerPosition = false;
    static auto shaderToggles = ShaderToggles();
    static auto light = Light();
    static auto dayTime = 0.4F;
    static auto animateDayTime = false;
    static auto dayDir = 1.0F;
    if (animateDayTime) {
        dayTime += 0.0005F * dayDir;
        if (dayTime > 1.0F || dayTime < 0.0F) {
            dayDir *= -1.0F;
        }
    }

    float angle = (dayTime * 0.7F * glm::two_pi<float>()) - 0.1F * glm::two_pi<float>();
    light.fragmentToLightDir = glm::vec3(std::cos(angle), std::sin(angle), 0.0F);

    ImGui::Begin("Settings");
    ImGui::SliderFloat("Day Time", &dayTime, 0.0F, 1.0F);
    ImGui::SameLine();
    ImGui::Checkbox("", &animateDayTime);
    auto cameraPosition = getCamera().getPosition();
    auto cameraDirection = getCamera().getForwardDirection();
    auto tmp = glm::vec2(getCamera().getPitch(), getCamera().getYaw());
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition));
    ImGui::DragFloat3("Camera Direction", reinterpret_cast<float *>(&cameraDirection));
    ImGui::DragFloat2("Pitch | Yaw", reinterpret_cast<float *>(&tmp));
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
    if (thingToRender == 0 || thingToRender == 2) {
        ImGui::Begin("Settings");
        sky.showGui();
        ImGui::Separator();
        trees.showGui();
        ImGui::Separator();
        if (ImGui::TreeNode("Light Properties")) {
            ImGui::DragFloat3("Light Direction", reinterpret_cast<float *>(&light.fragmentToLightDir));
            ImGui::DragFloat("Light Distance", &light.distance);
            ImGui::DragFloat("Light Scale", &light.scale);
            ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&light.color));
            ImGui::DragFloat("Light Ambient Power", &light.ambient, 0.01F);
            ImGui::DragFloat("Light Diffuse Power", &light.diffuse, 0.01F);
            ImGui::DragFloat("Light Specular Power", &light.specular, 0.01F);
            ImGui::DragFloat3("Atmosphere", reinterpret_cast<float *>(&atmosphere), 0.01F);
            ImGui::TreePop();
        }
        ImGui::Separator();
        if (ImGui::TreeNode("Shader Toggles")) {
            ImGui::Checkbox("Wireframe", &shaderToggles.drawWireframe);
            ImGui::Checkbox("Show UVs", &shaderToggles.showUVs);
            ImGui::Checkbox("Show Normals", &shaderToggles.showNormals);
            ImGui::Checkbox("Show Tangents", &shaderToggles.showTangents);
            ImGui::Checkbox("Use Ambient Occlusion", &shaderToggles.useAmbientOcclusion);
            ImGui::Checkbox("Use Atmospheric Scattering", &shaderToggles.useAtmosphericScattering);
            ImGui::Checkbox("Use ACESFilm", &shaderToggles.useACESFilm);
            ImGui::DragFloat("Exposure", &shaderToggles.exposure, 0.01F);
            ImGui::DragFloat("Gamma", &shaderToggles.gamma, 0.01F);
            ImGui::TreePop();
        }
        ImGui::Checkbox("Show Player View", &usePlayerPosition);
        ImGui::Separator();
        terrain.showGui();
        ImGui::End();

        terrain.showLayersGui();
    }
    ImGui::End();

    Camera camera;
    if (usePlayerPosition) {
        playerCamera.update(getInput());
        camera = playerCamera;
    } else {
        camera = getCamera();
    }

    switch (thingToRender) {
    case 0:
        renderTerrain(camera, light, shaderToggles);
        renderSSAO();
        renderSSAOBlur();
        renderGBufferToQuad(camera, light, shaderToggles);
        break;
    case 1:
        renderTextureViewer();
        break;
    case 2:
        renderTerrain(camera, light, shaderToggles);
        renderSSAO();
        renderSSAOBlur();
        renderGBufferViewer();
        break;
    default:
        break;
    }
}

void Landscape::renderTerrain(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
    GL_Call(glClearColor(0.0, 0.0, 0.0, 1.0));
    //    GL_Call(glClearColor(1.0, 1.0, 1.0, 1.0));
    //    GL_Call(glClearColor(0.529F, 0.808F, 0.922F, 1.0));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    renderLight(camera.getProjectionMatrix(), camera.getViewMatrix(), light);

    terrain.render(camera.getProjectionMatrix(), camera.getViewMatrix(), shaderToggles);
    trees.render(camera.getProjectionMatrix(), camera.getViewMatrix(), shaderToggles, terrain.terrainParams);

    renderSky(camera.getProjectionMatrix(), camera.getViewMatrix());
}

void Landscape::renderSSAO() {
    glDisable(GL_BLEND);
    ssaoShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    ssaoShader->setUniform("modelMatrix", modelMatrix);
    ssaoShader->setUniform("screenWidth", static_cast<float>(getWidth()));
    ssaoShader->setUniform("screenHeight", static_cast<float>(getHeight()));
    ssaoShader->setUniform("gPosition", 0);
    ssaoShader->setUniform("gNormal", 1);
    ssaoShader->setUniform("texNoise", 2);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT));

    // Send kernel + rotation
    for (unsigned int i = 0; i < 64; ++i) {
        ssaoShader->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    }
    ssaoShader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());
    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glActiveTexture(GL_TEXTURE2));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture));

    quadVA->bind();
    quadVA->setShader(ssaoShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Landscape::renderSSAOBlur() {
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFbo);
    glClear(GL_COLOR_BUFFER_BIT);

    ssaoBlurShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    ssaoBlurShader->setUniform("modelMatrix", modelMatrix);
    ssaoBlurShader->setUniform("ssaoInput", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

    quadVA->bind();
    quadVA->setShader(ssaoBlurShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Landscape::renderGBufferToQuad(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    lightingShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    lightingShader->setUniform("modelMatrix", modelMatrix);
    lightingShader->setUniform("gPosition", 0);
    lightingShader->setUniform("gNormal", 1);
    lightingShader->setUniform("gAlbedo", 2);
    lightingShader->setUniform("gDoLighting", 3);
    lightingShader->setUniform("ssao", 4);

    lightingShader->setUniform("useAmbientOcclusion", shaderToggles.useAmbientOcclusion);
    lightingShader->setUniform("useAtmosphericScattering", shaderToggles.useAtmosphericScattering);
    lightingShader->setUniform("useACESFilm", shaderToggles.useACESFilm);
    lightingShader->setUniform("exposure", shaderToggles.exposure);
    lightingShader->setUniform("gamma", shaderToggles.gamma);

    glm::vec3 lightDirView = glm::vec3(camera.getViewMatrix() * glm::vec4(light.fragmentToLightDir, 0.0F));
    lightingShader->setUniform("light.FragmentToLightDir", lightDirView);
    lightingShader->setUniform("light.Color", light.color);
    lightingShader->setUniform("light.Ambient", light.ambient);
    lightingShader->setUniform("light.Diffuse", light.diffuse);
    lightingShader->setUniform("light.Specular", light.specular);

    lightingShader->setUniform("cameraPosition", camera.getPosition());
    lightingShader->setUniform("cameraDir", camera.getForwardDirection());
    lightingShader->setUniform("pitch", camera.getPitch());
    lightingShader->setUniform("yaw", camera.getYaw());
    lightingShader->setUniform("cameraOrientation", camera.getOrientation());
    lightingShader->setUniform("aspectRatio", getAspectRatio());
    lightingShader->setUniform("atmosphere", atmosphere);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glActiveTexture(GL_TEXTURE2));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glActiveTexture(GL_TEXTURE3));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gDoLighting));
    GL_Call(glActiveTexture(GL_TEXTURE4));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBlurBuffer));

    quadVA->bind();
    quadVA->setShader(lightingShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void Landscape::renderGBufferViewer() {
    glDisable(GL_BLEND);
    static auto currentTextureIdIndex = 0;
    std::array<unsigned int, 6> textureIds = {
          gPosition, gNormal, gAlbedo, gDoLighting, ssaoColorBuffer, ssaoColorBlurBuffer,
    };
    std::array<const char *, 6> textureIdLabels = {
          "Position", "Normal", "Albedo", "Is Cloud", "SSAO", "SSAO Blur",
    };
    ImGui::Begin("Settings");
    ImGui::Combo("Selected Buffer", &currentTextureIdIndex, reinterpret_cast<const char *const *>(&textureIdLabels[0]),
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

void Landscape::renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const Light &light) {
    cubeVA->bind();
    flatShader->bind();
    cubeVA->setShader(flatShader);
    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(light.scale));
    modelMatrix = glm::translate(modelMatrix, normalize(light.fragmentToLightDir) * light.distance);
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
    flatShader->setUniform("modelMatrix", modelMatrix);
    flatShader->setUniform("normalMatrix", normalMatrix);
    flatShader->setUniform("viewMatrix", viewMatrix);
    flatShader->setUniform("projectionMatrix", projectionMatrix);
    flatShader->setUniform("flatColor", light.color);
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void Landscape::renderTextureViewer() {
    static auto textureType = 0;
    static auto texturePosition = glm::vec3(0.0F);
    static auto textureZoom = 1.0F;

    ImGui::Begin("Settings");
    const std::array<const char *, 4> items = {
          "Tree Positions",
          "Grass",
          "Dirt",
          "Rock",
    };
    const std::array<unsigned int, 4> textures = {
          trees.getTreePositionTextureId(),
          terrain.getGrassTexture()->getInternalId(),
          terrain.getDirtTexture()->getInternalId(),
          terrain.getRockTexture()->getInternalId(),
    };
    ImGui::Combo("", &textureType, items.data(), items.size());
    ImGui::Separator();
    ImGui::DragFloat("Zoom", &textureZoom, 0.01F);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&texturePosition), 0.01F);
    ImGui::End();

    renderTexture(texturePosition, textureZoom, textures[textureType]);
}

void Landscape::renderTexture(const glm::vec3 &texturePosition, const float zoom, const unsigned int texture) {
    textureShader->bind();
    quadVA->bind();
    quadVA->setShader(textureShader);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, texture));

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
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

    // create do-lighting buffer
    GL_Call(glGenTextures(1, &gDoLighting));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gDoLighting));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gDoLighting, 0));

    std::array<unsigned int, 4> attachments = {
          GL_COLOR_ATTACHMENT0, //
          GL_COLOR_ATTACHMENT1, //
          GL_COLOR_ATTACHMENT2, //
          GL_COLOR_ATTACHMENT3, //
    };
    glDrawBuffers(attachments.size(), reinterpret_cast<unsigned int *>(&attachments[0]));

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
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update color buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update do-lighting buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gDoLighting));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));

    // update depth buffer
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

    // update SSAO color buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));

    // update SSAO color blur buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBlurBuffer));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
}

void Landscape::renderSky(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
    static float animationTime = 0.0F;
    animationTime += static_cast<float>(getLastFrameTime());
    sky.render(projectionMatrix, viewMatrix, animationTime);
}
