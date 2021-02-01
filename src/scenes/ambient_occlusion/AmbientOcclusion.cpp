#include "AmbientOcclusion.h"

#include <Main.h>
#include <util/RenderUtils.h>

DEFINE_SCENE_MAIN(AmbientOcclusion)
DEFINE_DEFAULT_SHADERS(ambient_occlusion_AmbientOcclusion)

void AmbientOcclusion::setup() {
    shader = CREATE_DEFAULT_SHADER(ambient_occlusion_AmbientOcclusion);

    cube = createCubeVA(shader);
    quadVA = createQuadVA(shader);

    initFramebuffer();
}

void AmbientOcclusion::destroy() {}

void AmbientOcclusion::tick() {
    static auto position1 = glm::vec3(0.0F, 1.0F, 0.0F);
    static auto position2 = glm::vec3(0.0F, 0.0F, 0.5F);
    static auto lightPosition = glm::vec3(1.0F, 1.5F, 1.0F);

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Cube 1 Position", reinterpret_cast<float *>(&position1), 0.001F);
    ImGui::DragFloat3("Cube 2 Position", reinterpret_cast<float *>(&position2), 0.001F);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), 0.001F);
    ImGui::End();

    shader->bind();
    shader->setUniform("useTexture", false);
    shader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());
    shader->setUniform("viewMatrix", getCamera().getViewMatrix());

    renderSceneToFramebuffer(position1, position2, lightPosition);

    shader->bind();
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));
    shader->setUniform("projectionMatrix", glm::identity<glm::mat4>());
    shader->setUniform("viewMatrix", glm::identity<glm::mat4>());
    const glm::vec3 scale = glm::vec3(2.0F, 2.0F, 1.0F);
    auto modelMatrix = createModelMatrix(glm::vec3(), glm::vec3(), scale);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("useTexture", true);
    shader->setUniform("frameTexture", 0);
    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void AmbientOcclusion::renderSceneToFramebuffer(const glm::vec3 &position1, const glm::vec3 &position2,
                                                const glm::vec3 &lightPosition) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
    GL_Call(glClearColor(0.25, 0.25, 0.25, 1.0));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    cube->bind();

    auto modelMatrix1 = createModelMatrix(position1);
    shader->setUniform("modelMatrix", modelMatrix1);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    auto modelMatrix2 = createModelMatrix(position2);
    shader->setUniform("modelMatrix", modelMatrix2);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    auto modelMatrix3 = createModelMatrix(lightPosition, glm::vec3(0.0F), glm::vec3(0.1F));
    shader->setUniform("modelMatrix", modelMatrix3);
    GL_Call(glDrawElements(GL_TRIANGLES, cube->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void AmbientOcclusion::onAspectRatioChange() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));

    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
}

void AmbientOcclusion::initFramebuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    // Create a framebuffer object
    GL_Call(glGenFramebuffers(1, &fbo));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

    // Create a texture
    GL_Call(glGenTextures(1, &textureId));
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // Attach it to the framebuffer
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

    // Create a renderbuffer for the depth and stencil attachment of the fbo
    GL_Call(glGenRenderbuffers(1, &rbo));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

    // Attach renderbuffer to framebuffer
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));

    checkFramebufferStatus();
}
