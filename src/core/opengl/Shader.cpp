#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "util/FileUtils.h"

ShaderCode readShaderCodeFromFile(const std::string &filePath) {
    std::ifstream shaderStream(filePath, std::ios::in);
    if (!shaderStream.is_open()) {
        std::cerr << "Could not open " << filePath << std::endl;
        return ShaderCode();
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    std::string shaderCode = sstr.str();
    shaderStream.close();

    std::vector<std::string> lines = {};
    int j = 0;
    for (int i = 0; i < shaderCode.size(); i++) {
        if (shaderCode[i] == '\n') {
            lines.push_back(shaderCode.substr(j, i - j));
            j = i;
        }
    }

    unsigned int lineCount = lines.size();
    unsigned int lineLengthsSize = lineCount * sizeof(int);
    int *lineLengths = (int *)std::malloc(lineLengthsSize);
    unsigned int shaderSourceSize = lineCount * sizeof(char *);
    char **shaderSource = (char **)std::malloc(shaderSourceSize);
    for (int i = 0; i < lines.size(); i++) {
        int lineLength = lines[i].size();
        lineLengths[i] = lineLength;
        shaderSource[i] = (char *)std::malloc(lineLength * sizeof(char));
        for (int j = 0; j < lines[i].size(); j++) {
            shaderSource[i][j] = lines[i][j];
        }
    }
    return ShaderCode(lineCount, lineLengths, shaderSource);
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) : id(0) {
    auto vertexCode = readShaderCodeFromFile(vertexPath);
    auto fragmentCode = readShaderCodeFromFile(fragmentPath);
    shaderComponents[GL_VERTEX_SHADER] = vertexCode;
    shaderComponents[GL_FRAGMENT_SHADER] = fragmentCode;
    compile();
}

Shader::Shader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode) : id(0) {
    shaderComponents[GL_VERTEX_SHADER] = vertexCode;
    shaderComponents[GL_FRAGMENT_SHADER] = fragmentCode;
    compile();
}

void Shader::compile() {
    if (id != 0) {
        GL_Call(glDeleteProgram(id));
    }

    GLuint newProgramId = 0;
    GL_Call(newProgramId = glCreateProgram());

    auto shaderIds = std::vector<GLuint>();
    for (const auto &shaderCode : shaderComponents) {
        GLuint shaderId = load(shaderCode.first, shaderCode.second);
        if (shaderId == 0U) {
            return;
        }
        GL_Call(glAttachShader(newProgramId, shaderId));
        shaderIds.push_back(shaderId);
    }

    GL_Call(glLinkProgram(newProgramId));

    int success = 0;
    int infoLogLength = 0;
    GL_Call(glGetProgramiv(newProgramId, GL_LINK_STATUS, &success));
    GL_Call(glGetProgramiv(newProgramId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (success != 1 || infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        GL_Call(glGetProgramInfoLog(newProgramId, infoLogLength, nullptr, &programErrorMessage[0]));
        std::cout << &programErrorMessage[0] << std::endl;
    }

    for (const auto &shaderId : shaderIds) {
        GL_Call(glDetachShader(newProgramId, shaderId));
        GL_Call(glDeleteShader(shaderId));
    }

    if (success != 0) {
        // assign new id at the very end to be able to tolerate failed compilation or failed linking
        id = newProgramId;
    }
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
        } else if (shaderType == GL_TESS_CONTROL_SHADER) {
            std::cerr << "tesselation control";
        } else if (shaderType == GL_TESS_EVALUATION_SHADER) {
            std::cerr << "tesselation evaluation";
        } else {
            std::cerr << "unknown";
        }
        std::cerr << " shader" << std::endl;
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

void Shader::attach(GLuint shaderType, const std::string &filePath) {
    auto shaderCode = readShaderCodeFromFile(filePath);
    shaderComponents[shaderType] = shaderCode;
    compile();
}
