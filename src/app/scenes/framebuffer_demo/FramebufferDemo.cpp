#include "FramebufferDemo.h"

#include "util/RenderUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 1000.0F;

void FramebufferDemo::setup() {
    shader = std::make_shared<Shader>("scenes/framebuffer_demo/FramebufferDemoVert.glsl",
                                      "scenes/framebuffer_demo/FramebufferDemoFrag.glsl");
    shader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    Object cube1 = {createCubeVA(shader), glm::vec3(0, 0, -1), glm::vec3(0.5, 1.0, 0.75), 0.25};
    Object cube2 = {createCubeVA(shader), glm::vec3(0.5, 0, -1), glm::vec3(0.75, 0.0, 0.5), 0.25};
    Object cube3 = {createCubeVA(shader), glm::vec3(0, 0.5, -1), glm::vec3(0.0, 0.5, 0.75), 0.25};

    objects.push_back(cube1);
    objects.push_back(cube2);
    objects.push_back(cube3);

    mirrorVA = createQuadVA(shader);

    initFramebuffer();
}

void FramebufferDemo::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void FramebufferDemo::destroy() {}

void FramebufferDemo::tick() {
    static glm::vec3 cameraPosition = {0.0F, 0.0F, 0.0F};
    static glm::vec3 cameraRotation = {0.0F, 0.0F, 0.0F};
    static glm::vec3 mirrorPosition = {-0.4F, -0.2F, -0.995F};
    static glm::vec3 mirrorRotation = {-0.6F, 0.85F, 0.0F};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.001F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.001F);
    ImGui::DragFloat3("Mirror Position", reinterpret_cast<float *>(&mirrorPosition), 0.001F);
    ImGui::DragFloat3("Mirror Rotation", reinterpret_cast<float *>(&mirrorRotation), 0.001F);
    ImGui::End();

    glm::mat4 cameraRotationMatrix = createViewMatrix({0, 0, 0}, cameraRotation);
    glm::vec3 cameraDirection = cameraRotationMatrix * glm::vec4(0.0F, 0.0F, -1.0F, 0.0F);
    glm::vec3 cameraUp = cameraRotationMatrix * glm::vec4(0.0F, 1.0F, 0.0F, 0.0F);

    glm::mat4 mirrorRotationMatrix = createViewMatrix({0, 0, 0}, mirrorRotation);
    glm::vec3 mirrorNormal = mirrorRotationMatrix * glm::vec4(0.0F, 0.0F, 1.0F, 0.0F);
    glm::vec3 mirrorUp = mirrorRotationMatrix * glm::vec4(0.0F, 1.0F, 0.0F, 0.0F);

    glm::vec3 mirrorCameraDirection = glm::normalize(glm::reflect(mirrorPosition - cameraPosition, mirrorNormal));
    glm::mat4 viewMatrix = glm::lookAt(mirrorPosition, mirrorPosition + mirrorCameraDirection, mirrorUp);
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
    GL_Call(glClearColor(0.25, 0.25, 0.25, 1.0));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    renderObjects(viewMatrix);
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
    renderObjects(viewMatrix);

    renderMirror(viewMatrix, mirrorPosition, mirrorRotation);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FramebufferDemo::renderObjects(const glm::mat4 &viewMatrix) {
    shader->bind();
    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_ProjectionMatrix", projectionMatrix);
    shader->setUniform("u_ViewMatrix", viewMatrix);
    for (const auto &obj : objects) {

        glm::mat4 modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::translate(modelMatrix, obj.position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(obj.scale, obj.scale, obj.scale));

        shader->setUniform("u_Color", obj.color);
        shader->setUniform("u_ModelMatrix", modelMatrix);

        obj.va->bind();
        GL_Call(glDrawElements(GL_TRIANGLES, obj.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

        obj.va->unbind();
    }
    shader->unbind();
}

void FramebufferDemo::renderMirror(const glm::mat4 &viewMatrix, const glm::vec3 &mirrorPosition,
                                   const glm::vec3 &mirrorRotation) {
    glm::mat4 modelMatrix = glm::mat4(1.0);
    const float scale = 0.2F;
    modelMatrix = glm::translate(modelMatrix, mirrorPosition);
    modelMatrix = glm::rotate(modelMatrix, mirrorRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, mirrorRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, mirrorRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, {scale, scale, scale});

    shader->bind();
    shader->setUniform("u_UseTexture", true);
    shader->setUniform("u_ProjectionMatrix", projectionMatrix);
    shader->setUniform("u_ViewMatrix", viewMatrix);
    shader->setUniform("u_ModelMatrix", modelMatrix);
    shader->setUniform("u_Texture", 0);
    shader->setUniform("u_Color", glm::vec3(1.0, 1.0, 1.0));

    GL_Call(glBindTexture(GL_TEXTURE_2D, mirrorTextureId));

    mirrorVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, mirrorVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void FramebufferDemo::initFramebuffer() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();

    // Create a framebuffer object
    GL_Call(glGenFramebuffers(1, &fbo));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

    // Create a texture
    GL_Call(glGenTextures(1, &mirrorTextureId));
    GL_Call(glBindTexture(GL_TEXTURE_2D, mirrorTextureId));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // Attach it to the framebuffer
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0));

    // Create a renderbuffer for the depth and stencil attachment of the fbo
    unsigned int rbo;
    GL_Call(glGenRenderbuffers(1, &rbo));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

    // Attach renderbuffer to framebuffer
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));

    checkFramebufferStatus();
}

void FramebufferDemo::checkFramebufferStatus() const {
    GLenum status;
    GL_Call(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete: ";
        switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            std::cerr << "Could not define a framebuffer";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "Some attachment is incomplete";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "Some attachment is missing";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "A draw buffer is incomplete";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "A read buffer is incomplete";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "Framebuffers are not supported";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            std::cerr << "Multisample is incomplete";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            std::cerr << "Layer targets are incomplete";
            break;
        default:
            std::cerr << "Unknown error";
            break;
        }
        std::cerr << std::endl;
    }
}
