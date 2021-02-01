#include "AmbientOcclusion.h"

#include <Main.h>
#include <util/RenderUtils.h>

#include <array>

DEFINE_SCENE_MAIN(AmbientOcclusion)

DEFINE_DEFAULT_SHADERS(ambient_occlusion_Texture)
DEFINE_DEFAULT_SHADERS(ambient_occlusion_Geometry)
DEFINE_DEFAULT_SHADERS(ambient_occlusion_Lighting)

void AmbientOcclusion::setup() {
    textureShader = CREATE_DEFAULT_SHADER(ambient_occlusion_Texture);
    geometryShader = CREATE_DEFAULT_SHADER(ambient_occlusion_Geometry);
    lightingShader = CREATE_DEFAULT_SHADER(ambient_occlusion_Lighting);

    cube = createCubeVA(geometryShader);
    quadVA = createQuadVA(lightingShader);

    initFramebuffer();
}

void AmbientOcclusion::destroy() {}

void AmbientOcclusion::tick() {
    static auto position1 = glm::vec3(0.0F, 1.0F, 0.0F);
    static auto position2 = glm::vec3(0.0F, 0.0F, 0.5F);
    static auto lightPosition = glm::vec3(1.0F, 1.5F, 1.0F);
    static auto lightColor = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto shouldRenderTexture = true;
    static auto currentTextureIdIndex = 0;
    unsigned int textureIds[3] = {gPosition, gNormal, gAlbedo};
    const char *textureIdLabels[3] = {"Position", "Normal", "Albedo"};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Cube 1 Position", reinterpret_cast<float *>(&position1), 0.001F);
    ImGui::DragFloat3("Cube 2 Position", reinterpret_cast<float *>(&position2), 0.001F);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), 0.001F);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
    ImGui::Checkbox("Render Texture", &shouldRenderTexture);
    if (shouldRenderTexture) {
        ImGui::Combo("", &currentTextureIdIndex, textureIdLabels, 3);
    }
    ImGui::End();

    renderSceneToFramebuffer(position1, position2, lightPosition);

    if (shouldRenderTexture) {
        renderTexture(textureIds[currentTextureIdIndex]);
    } else {
        renderScreenQuad(lightPosition, lightColor);
    }
}

void AmbientOcclusion::renderSceneToFramebuffer(const glm::vec3 &position1, const glm::vec3 &position2,
                                                const glm::vec3 &lightPosition) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
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

void AmbientOcclusion::renderScreenQuad(const glm::vec3 &lightPosition, const glm::vec3 &lightColor) {
    lightingShader->bind();
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    lightingShader->setUniform("modelMatrix", modelMatrix);
    lightingShader->setUniform("gPosition", 0);
    lightingShader->setUniform("gNormal", 1);
    lightingShader->setUniform("gAlbedo", 2);

    const float linear = 0.09F;
    const float quadratic = 0.032F;

    glm::vec3 lightPositionView = glm::vec3(getCamera().getViewMatrix() * glm::vec4(lightPosition, 1.0F));
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
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    // update position buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update normal buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));

    // update color buffer
    GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
}

void AmbientOcclusion::initFramebuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    // Create a framebuffer object
    GL_Call(glGenFramebuffers(1, &fbo));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

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
