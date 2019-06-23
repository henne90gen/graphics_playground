#include "OpenGLUtils.h"

#include <fstream>
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
        std::cout << "OpenGL error [0x" << std::hex << error << "]: " << file << "/" << function << ":" << line << std::endl;
        noErrors = false;
    }
    return noErrors;
}

GLuint loadShader(GLuint shaderType, const char *file_path) {
    std::string shaderCode;
    std::ifstream shaderStream(file_path, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    }

    GL_Call(GLuint shaderId = glCreateShader(shaderType));

    char const *sourcePointer = shaderCode.c_str();
    GL_Call(glShaderSource(shaderId, 1, &sourcePointer, nullptr));
    GL_Call(glCompileShader(shaderId));

    int error;
    int infoLogLength;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &error));
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    return shaderId;
}

GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path) {
    GL_Call(GLuint programId = glCreateProgram());
    GLuint vertexShaderId = loadShader(GL_VERTEX_SHADER, vertex_file_path);
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragment_file_path);
    GL_Call(glAttachShader(programId, vertexShaderId));
    GL_Call(glAttachShader(programId, fragmentShaderId));

    GL_Call(glLinkProgram(programId));

    int error;
    int infoLogLength;
    GL_Call(glGetProgramiv(programId, GL_LINK_STATUS, &error));
    GL_Call(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        GL_Call(glGetProgramInfoLog(programId, infoLogLength, nullptr, &programErrorMessage[0]));
        std::cout << &programErrorMessage[0] << std::endl;
    }

    GL_Call(glDetachShader(programId, vertexShaderId));
    GL_Call(glDetachShader(programId, fragmentShaderId));

    GL_Call(glDeleteShader(vertexShaderId));
    GL_Call(glDeleteShader(fragmentShaderId));

    return programId;
}

glm::mat4 createViewMatrix(const glm::vec3 &translation, const glm::vec3 &cameraRotation) {
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    return viewMatrix;
}
