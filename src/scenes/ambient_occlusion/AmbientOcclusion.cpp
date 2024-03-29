#include "AmbientOcclusion.h"

#include <Main.h>
#include <util/RenderUtils.h>

#include <array>
#include <random>

DEFINE_SCENE_MAIN(AmbientOcclusion)

DEFINE_DEFAULT_SHADERS(ambient_occlusion_Geometry)

DEFINE_VERTEX_SHADER(ambient_occlusion_ScreenQuad)
DEFINE_FRAGMENT_SHADER(ambient_occlusion_Texture)
DEFINE_FRAGMENT_SHADER(ambient_occlusion_Lighting)
DEFINE_FRAGMENT_SHADER(ambient_occlusion_SSAO)
DEFINE_FRAGMENT_SHADER(ambient_occlusion_SSAOBlur)

void AmbientOcclusion::setup() {
    geometryShader = CREATE_DEFAULT_SHADER(ambient_occlusion_Geometry);
    textureShader = createDefaultShader(SHADER_CODE(ambient_occlusion_ScreenQuadVert),
                                        SHADER_CODE(ambient_occlusion_TextureFrag));
    lightingShader = createDefaultShader(SHADER_CODE(ambient_occlusion_ScreenQuadVert),
                                         SHADER_CODE(ambient_occlusion_LightingFrag));
    ssaoShader =
          createDefaultShader(SHADER_CODE(ambient_occlusion_ScreenQuadVert), SHADER_CODE(ambient_occlusion_SSAOFrag));
    ssaoBlurShader = createDefaultShader(SHADER_CODE(ambient_occlusion_ScreenQuadVert),
                                         SHADER_CODE(ambient_occlusion_SSAOBlurFrag));

    cube = createCubeVA(geometryShader);
    quadVA = createQuadVA(lightingShader);

    initGBuffer();
    initSSAOBuffer();
    initSSAOBlurBuffer();
    initKernelAndNoiseTexture();
}

void AmbientOcclusion::destroy() {}

void AmbientOcclusion::tick() {
    static auto position1 = glm::vec3(0.0F, 1.0F, 0.0F);
    static auto position2 = glm::vec3(0.0F, 0.0F, 0.5F);
    static auto lightPosition = glm::vec3(1.0F, 1.5F, 1.0F);
    static auto lightColor = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto useAmbientOcclusion = true;
    static auto usePointLight = true;
    static auto currentTextureIdIndex = 4;
    static auto shouldRenderTexture = false;
    std::array<unsigned int, 5> textureIds = {gPosition, gNormal, gAlbedo, ssaoColorBuffer, ssaoColorBlurBuffer};
    std::array<const char *, 5> textureIdLabels = {"Position", "Normal", "Albedo", "SSAO", "SSAO Blur"};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Cube 1 Position", reinterpret_cast<float *>(&position1), 0.001F);
    ImGui::DragFloat3("Cube 2 Position", reinterpret_cast<float *>(&position2), 0.001F);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), 0.001F);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
    ImGui::Checkbox("Use Ambient Occlusion", &useAmbientOcclusion);
    ImGui::Checkbox("Use Point Light", &usePointLight);
    ImGui::Checkbox("Render Texture", &shouldRenderTexture);
    if (shouldRenderTexture) {
        ImGui::Combo("##", &currentTextureIdIndex, reinterpret_cast<const char *const *>(textureIdLabels.data()),
                     textureIdLabels.size());
    }
    ImGui::End();

    renderSceneToGBuffer(position1, position2, lightPosition);

    renderSSAO();
    renderSSAOBlur();

    if (shouldRenderTexture) {
        renderTexture(textureIds[currentTextureIdIndex]);
    } else {
        renderGBufferToQuad(lightPosition, lightColor, useAmbientOcclusion, usePointLight);
    }
}

void AmbientOcclusion::renderSceneToGBuffer(const glm::vec3 &position1, const glm::vec3 &position2,
                                            const glm::vec3 &lightPosition) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
    GL_Call(glClearColor(0.25, 0.25, 0.25, 1.0));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    cube->bind();
    geometryShader->bind();
    geometryShader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());
    geometryShader->setUniform("viewMatrix", getCamera().getViewMatrix());

    auto modelMatrix1 = createModelMatrix(position1);
    geometryShader->setUniform("modelMatrix", modelMatrix1);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    auto modelMatrix2 = createModelMatrix(position2);
    geometryShader->setUniform("modelMatrix", modelMatrix2);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    auto modelMatrix3 = createModelMatrix(lightPosition, glm::vec3(0.0F), glm::vec3(0.1F));
    geometryShader->setUniform("modelMatrix", modelMatrix3);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void AmbientOcclusion::renderSSAO() {
    ssaoShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    ssaoShader->setUniform("modelMatrix", modelMatrix);
    ssaoShader->setUniform("screenWidth", static_cast<float>(getWidth()));
    ssaoShader->setUniform("screenHeight", static_cast<float>(getHeight()));
    ssaoShader->setUniform("gPosition", 0);
    ssaoShader->setUniform("gNormal", 1);
    ssaoShader->setUniform("texNoise", 2);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
    glClear(GL_COLOR_BUFFER_BIT);

    // Send kernel + rotation
    for (unsigned int i = 0; i < 64; ++i) {
        ssaoShader->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    }
    ssaoShader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);

    quadVA->bind();
    quadVA->setShader(ssaoShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AmbientOcclusion::renderSSAOBlur() {
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

void AmbientOcclusion::renderGBufferToQuad(const glm::vec3 &lightPosition, const glm::vec3 &lightColor,
                                           bool useAmbientOcclusion, bool usePointLight) {
    lightingShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    lightingShader->setUniform("modelMatrix", modelMatrix);
    lightingShader->setUniform("gPosition", 0);
    lightingShader->setUniform("gNormal", 1);
    lightingShader->setUniform("gAlbedo", 2);
    lightingShader->setUniform("ssao", 3);
    lightingShader->setUniform("useAmbientOcclusion", useAmbientOcclusion);
    lightingShader->setUniform("usePointLight", usePointLight);

    const float linear = 0.09F;
    const float quadratic = 0.032F;

    const auto lightPositionView = getCamera().getViewMatrix() * glm::vec4(lightPosition, usePointLight ? 1.0F : 0.0F);
    lightingShader->setUniform("light.Position", lightPositionView);
    lightingShader->setUniform("light.Color", lightColor);
    lightingShader->setUniform("light.Linear", linear);
    lightingShader->setUniform("light.Quadratic", quadratic);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glActiveTexture(GL_TEXTURE2));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glActiveTexture(GL_TEXTURE3));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoColorBlurBuffer));

    quadVA->bind();
    quadVA->setShader(lightingShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void AmbientOcclusion::renderTexture(unsigned int textureId) {
    textureShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    textureShader->setUniform("modelMatrix", modelMatrix);
    textureShader->setUniform("textureId", 0);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));

    quadVA->bind();
    quadVA->setShader(textureShader);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void AmbientOcclusion::onAspectRatioChange() {
    const auto width = getWidth();
    const auto height = getHeight();
    std::cout << "New dimensions: " << width << "x" << height << std::endl;

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

void AmbientOcclusion::initGBuffer() {
    const auto width = getWidth();
    const auto height = getHeight();
    std::cout << "Creating G framebuffer with dimensions: " << width << "x" << height << std::endl;

    // Framebuffer object
    GL_Call(glGenFramebuffers(1, &gBuffer));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));

    // Position buffer
    GL_Call(glGenTextures(1, &gPosition));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));

    // Normal buffer
    GL_Call(glGenTextures(1, &gNormal));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));

    // Color buffer
    GL_Call(glGenTextures(1, &gAlbedo));
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

    std::array<unsigned int, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    GL_Call(glDrawBuffers(3, reinterpret_cast<unsigned int *>(attachments.data())));

    // Depth buffer
    GL_Call(glGenRenderbuffers(1, &depthBuffer));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));

    checkFramebufferStatus();
}

void AmbientOcclusion::initSSAOBuffer() {
    const auto width = getWidth();
    const auto height = getHeight();
    std::cout << "Creating SSAO framebuffer with dimensions: " << width << "x" << height << std::endl;

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

void AmbientOcclusion::initSSAOBlurBuffer() {
    const auto width = getWidth();
    const auto height = getHeight();
    std::cout << "Creating SSAO-Blur framebuffer with dimensions: " << width << "x" << height << std::endl;

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

float custom_lerp(float a, float b, float f) { return a + f * (b - a); }

void AmbientOcclusion::initKernelAndNoiseTexture() {
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0F;

        // scale samples so that they're more aligned to center of kernel
        scale = custom_lerp(0.1F, 1.0F, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        const glm::vec3 noise(randomFloats(generator) * 2.0F - 1.0F, randomFloats(generator) * 2.0F - 1.0F,
                              0.0F); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    GL_Call(glGenTextures(1, &ssaoNoiseTexture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data()));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}
