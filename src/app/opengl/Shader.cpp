#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "util/FileUtils.h"

Shader::Shader(std::string vertexPath, std::string fragmentPath)
        : id(0), vertexPath(std::move(vertexPath)), fragmentPath(std::move(fragmentPath)), lastModTimeVertex(0),
          lastModTimeFragment(0) {
}

void Shader::compile() {
    lastModTimeVertex = getLastModifiedTime(vertexPath);
    lastModTimeFragment = getLastModifiedTime(fragmentPath);
    uniformLocations.clear();

    GLuint newProgramId;
    GL_Call(newProgramId = glCreateProgram());
    GLuint vertexShaderId = load(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShaderId = load(GL_FRAGMENT_SHADER, fragmentPath);
    if ((vertexShaderId == 0U) || (fragmentShaderId == 0U)) {
        return;
    }
    GL_Call(glAttachShader(newProgramId, vertexShaderId));
    GL_Call(glAttachShader(newProgramId, fragmentShaderId));

    GL_Call(glLinkProgram(newProgramId));

    int success;
    int infoLogLength;
    GL_Call(glGetProgramiv(newProgramId, GL_LINK_STATUS, &success));
    GL_Call(glGetProgramiv(newProgramId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        GL_Call(glGetProgramInfoLog(newProgramId, infoLogLength, nullptr, &programErrorMessage[0]));
        std::cout << &programErrorMessage[0] << std::endl;
    }

    GL_Call(glDetachShader(newProgramId, vertexShaderId));
    GL_Call(glDetachShader(newProgramId, fragmentShaderId));

    GL_Call(glDeleteShader(vertexShaderId));
    GL_Call(glDeleteShader(fragmentShaderId));

    if (success != 0) {
        // assign new id at the very end to be able to tolerate failed compilation or failed linking
        id = newProgramId;
    }
}

GLuint Shader::load(GLuint shaderType, std::string &filePath) {
    std::string shaderCode;
    std::ifstream shaderStream(filePath, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    } else {
        std::cout << "Could not open " << filePath << std::endl;
        return GL_INVALID_VALUE;
    }

    GLuint shaderId;
    GL_Call(shaderId = glCreateShader(shaderType));

    char const *sourcePointer = shaderCode.c_str();
    GL_Call(glShaderSource(shaderId, 1, &sourcePointer, nullptr));
    GL_Call(glCompileShader(shaderId));

    int success;
    int infoLogLength;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    if (success == 0) {
        return 0;
    }

    return shaderId;
}

int Shader::getUniformLocation(const std::string &name) {
    auto locationItr = uniformLocations.find(name);
    if (locationItr != uniformLocations.end()) {
        return locationItr->second;
    }

    int location;
    GL_Call(location = glGetUniformLocation(id, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: Could not find uniform '" << name << "'" << std::endl;
    }

    uniformLocations[name] = location;
    return location;
}

void Shader::bind() {
    if (hasBeenModified()) {
        compile();
    }
    GL_Call(glUseProgram(id));
}

void Shader::unbind() const { GL_Call(glUseProgram(0)); }

bool Shader::hasBeenModified() {
    return getLastModifiedTime(vertexPath) > lastModTimeVertex ||
           getLastModifiedTime(fragmentPath) > lastModTimeFragment;
}
