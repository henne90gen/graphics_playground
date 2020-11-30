#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "util/FileUtils.h"

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) : id(0) {
    compile(vertexPath, fragmentPath);
}

Shader::Shader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode) : id(0) {
    compile(vertexCode, fragmentCode);
}

void Shader::compile(const std::string &vertexPath, const std::string &fragmentPath) {
    GLuint newProgramId = 0;
    GL_Call(newProgramId = glCreateProgram());
    GLuint vertexShaderId = load(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShaderId = load(GL_FRAGMENT_SHADER, fragmentPath);
    if ((vertexShaderId == 0U) || (fragmentShaderId == 0U)) {
        return;
    }
    GL_Call(glAttachShader(newProgramId, vertexShaderId));
    GL_Call(glAttachShader(newProgramId, fragmentShaderId));

    GL_Call(glLinkProgram(newProgramId));

    int success = 0;
    int infoLogLength = 0;
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

void Shader::compile(const ShaderCode &vertexCode, const ShaderCode &fragmentCode) {
    GLuint newProgramId = 0;
    GL_Call(newProgramId = glCreateProgram());
    GLuint vertexShaderId = load(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShaderId = load(GL_FRAGMENT_SHADER, fragmentCode);
    if ((vertexShaderId == 0U) || (fragmentShaderId == 0U)) {
        return;
    }
    GL_Call(glAttachShader(newProgramId, vertexShaderId));
    GL_Call(glAttachShader(newProgramId, fragmentShaderId));

    GL_Call(glLinkProgram(newProgramId));

    int success = 0;
    int infoLogLength = 0;
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

GLuint Shader::load(GLuint shaderType, const std::string &filePath) {
    std::string shaderCode;
    std::ifstream shaderStream(filePath, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    } else {
        std::cerr << "Could not open " << filePath << std::endl;
        return 0;
    }

    GLuint shaderId = 0;
    GL_Call(shaderId = glCreateShader(shaderType));

    // TODO(henne): process shader source into lines, so that error reporting works properly
    char const *sourcePointer = shaderCode.c_str();
    GL_Call(glShaderSource(shaderId, 1, &sourcePointer, nullptr));
    GL_Call(glCompileShader(shaderId));

    int success = 0;
    int infoLogLength = 0;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    if (success == 0) {
        std::cerr << "Failed to compile ";
        if (shaderType == GL_VERTEX_SHADER) {
            std::cerr << "vertex";
        } else if (shaderType == GL_FRAGMENT_SHADER) {
            std::cerr << "fragment";
        } else {
            std::cerr << "unknown";
        }
        std::cerr << "shader: " << filePath << std::endl;
        return 0;
    }

    return shaderId;
}

GLuint Shader::load(GLuint shaderType, const ShaderCode &shaderCode) {
    GLuint shaderId = 0;
    GL_Call(shaderId = glCreateShader(shaderType));

    GL_Call(glShaderSource(shaderId, shaderCode.lineCount, shaderCode.shaderSource, shaderCode.lineLengths));
    GL_Call(glCompileShader(shaderId));

    int success = 0;
    int infoLogLength = 0;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    if (success == 0) {
        std::cerr << "Failed to compile ";
        if (shaderType == GL_VERTEX_SHADER) {
            std::cerr << "vertex";
        } else if (shaderType == GL_FRAGMENT_SHADER) {
            std::cerr << "fragment";
        } else {
            std::cerr << "unknown";
        }
        std::cerr << "shader" << std::endl;
        return 0;
    }

    return shaderId;
}

int Shader::getUniformLocation(const std::string &name) {
    auto locationItr = uniformLocations.find(name);
    if (locationItr != uniformLocations.end()) {
        return locationItr->second;
    }

    int location = 0;
    GL_Call(location = glGetUniformLocation(id, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: Could not find uniform '" << name << "'" << std::endl;
    }

    uniformLocations[name] = location;
    return location;
}

int Shader::getAttributeLocation(const std::string &name) {
    auto locationItr = attributeLocations.find(name);
    if (locationItr != attributeLocations.end()) {
        return locationItr->second;
    }

    int location = 0;
    GL_Call(location = glGetAttribLocation(id, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: Could not find attribute '" << name << "'" << std::endl;
    }

    attributeLocations[name] = location;
    return location;
}

void Shader::bind() { GL_Call(glUseProgram(id)); }

void Shader::unbind() { GL_Call(glUseProgram(0)); }
