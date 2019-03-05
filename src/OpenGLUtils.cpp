#include "OpenGLUtils.h"

#include <glad/glad.h>

#include <iostream>

void GL_ClearError() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool GL_LogCall(const char *function, const char *file, int line) {
    bool noErrors = true;
    while (GLenum error = glGetError() != GL_NO_ERROR) {
        std::cout << "OpenGL error [" << error << "]: " << file << "/" << function << "[" << line << "]" << std::endl;
        noErrors = false;
    }
    return noErrors;
}
