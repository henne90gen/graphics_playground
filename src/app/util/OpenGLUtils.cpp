#include "OpenGLUtils.h"
#include "Image.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void GL_ClearError() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool GL_LogCall(const char *function, const char *file, int line) {
    bool noErrors = true;
    while (GLenum error = glGetError() != GL_NO_ERROR) {
        std::cout << "OpenGL error [0x" << std::hex << error << "]: " << file << "/" << function << ":" << line
                  << std::endl;
        noErrors = false;
    }
    return noErrors;
}

glm::mat4 createViewMatrix(const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation) {
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, cameraPosition);
    return viewMatrix;
}

MappedMousePosition
mapMouseOntoCanvas(const InputData *input, const glm::mat4 &transformationMatrix, unsigned int canvasWidth,
                   unsigned int canvasHeight, unsigned int displayWidth, unsigned int displayHeight) {
    const auto widthF = static_cast<float>(canvasWidth);
    const auto heightF = static_cast<float>(canvasHeight);
    const auto displayWidthF = static_cast<float>(displayWidth);
    const auto displayHeightF = static_cast<float>(displayHeight);

    auto &mousePos = input->mouse.pos;

    auto mouseDisplaySpace = glm::vec2(mousePos.x / displayWidthF, (displayHeightF - mousePos.y) / displayHeightF);
    mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

    auto adjustedDisplayPos = glm::inverse(transformationMatrix) * glm::vec4(mouseDisplaySpace, 0.0F, 1.0F);
    auto canvasPos = (glm::vec2(adjustedDisplayPos.x, adjustedDisplayPos.y) + glm::vec2(1.0F, 1.0F)) / 2.0F;
    return {
            glm::vec2(canvasPos.x * widthF, heightF - (canvasPos.y * heightF)),
            glm::vec2(adjustedDisplayPos.x, adjustedDisplayPos.y)
    };
}
