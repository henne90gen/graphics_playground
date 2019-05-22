#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

Shader::Shader(std::string vertexPath, std::string fragmentPath)
        : id(0), vertexPath(std::move(vertexPath)), fragmentPath(std::move(fragmentPath)) {
    compile();
}

void Shader::compile() {
    GL_Call(id = glCreateProgram());
    GLuint vertexShaderId = load(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShaderId = load(GL_FRAGMENT_SHADER, fragmentPath);
    GL_Call(glAttachShader(id, vertexShaderId));
    GL_Call(glAttachShader(id, fragmentShaderId));

    GL_Call(glLinkProgram(id));

    int error;
    int infoLogLength;
    GL_Call(glGetProgramiv(id, GL_LINK_STATUS, &error));
    GL_Call(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        GL_Call(glGetProgramInfoLog(id, infoLogLength, nullptr, &programErrorMessage[0]));
        std::cout << &programErrorMessage[0] << std::endl;
    }

    GL_Call(glDetachShader(id, vertexShaderId));
    GL_Call(glDetachShader(id, fragmentShaderId));

    GL_Call(glDeleteShader(vertexShaderId));
    GL_Call(glDeleteShader(fragmentShaderId));
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

int Shader::getUniformLocation(const std::string &name) {
    auto locationItr = uniformLocations.find(name);
    if (locationItr != uniformLocations.end()) {
        return locationItr->second;
    }

    GL_Call(int location = glGetUniformLocation(id, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist." << std::endl;
    }

    uniformLocations[name] = location;
    return location;
}

void Shader::bind() const { GL_Call(glUseProgram(id)); }

void Shader::unbind() const { GL_Call(glUseProgram(0)); }
