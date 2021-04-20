#include "OpenGLUtils.h"

#include <iostream>
#include <vector>

void GL_ClearError() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool GL_LogCall(const char *function, const char *file, int line) {
    bool noErrors = true;
    while (GLenum error = glGetError() != GL_NO_ERROR) {
        std::cout << "OpenGL error [0x" << std::hex << error << std::dec << "]: " << file << "/" << function << ": "
                  << line << std::endl;
        noErrors = false;
    }
    return noErrors;
}

glm::mat4 createViewMatrix(const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation) {
    auto viewMatrix = glm::identity<glm::mat4>();
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, cameraPosition);
    return viewMatrix;
}

glm::mat4 createModelMatrix(const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                            const glm::vec3 &modelScale) {
    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, modelPosition);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    return modelMatrix;
}

MappedMousePosition mapMouseOntoCanvas(const glm::vec2 &mousePos, const glm::mat4 &transformationMatrix,
                                       unsigned int canvasWidth, unsigned int canvasHeight, unsigned int displayWidth,
                                       unsigned int displayHeight) {
    const auto widthF = static_cast<float>(canvasWidth);
    const auto heightF = static_cast<float>(canvasHeight);
    const auto displayWidthF = static_cast<float>(displayWidth);
    const auto displayHeightF = static_cast<float>(displayHeight);

    auto mouseDisplaySpace = glm::vec2(mousePos.x / displayWidthF, (displayHeightF - mousePos.y) / displayHeightF);
    mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

    auto adjustedDisplayPos = glm::inverse(transformationMatrix) * glm::vec4(mouseDisplaySpace, 0.0F, 1.0F);
    auto canvasPos = (glm::vec2(adjustedDisplayPos.x, adjustedDisplayPos.y) + glm::vec2(1.0F, 1.0F)) / 2.0F;
    return {
          glm::vec2(canvasPos.x * widthF, heightF - (canvasPos.y * heightF)),
          glm::vec3(adjustedDisplayPos.x, adjustedDisplayPos.y, adjustedDisplayPos.z),
    };
}

void checkFramebufferStatus() {
    GLenum status = 0;
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
