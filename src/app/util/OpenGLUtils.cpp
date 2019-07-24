#include "OpenGLUtils.h"
#include "Image.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

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

void saveScreenshot(unsigned int windowWidth, unsigned int windowHeight) {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H:%M:%S");
    const std::string fileName = "../../../screenshot-" + buffer.str() + ".png";

    Image image = {};
    image.fileName = fileName;
    image.width = windowWidth;
    image.height = windowHeight;
    image.channels = 3;

    const int numberOfPixels = image.width * image.height * image.channels;
    image.pixels = std::vector<unsigned char>(numberOfPixels);

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.pixels.data()));

    ImageOps::save(image);
    std::cout << "Saved screenshot " << image.fileName << std::endl;
}
