#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string shaderTypeToString(GLuint shaderType) {
    switch (shaderType) {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    case GL_TESS_CONTROL_SHADER:
        return "tesselation control";
    case GL_TESS_EVALUATION_SHADER:
        return "tesselation evaluation";
    default:
        return "unknown";
    }
}

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
    for (int i = 0; i < static_cast<int>(shaderCode.size()); i++) {
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
    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        int lineLength = static_cast<int>(lines[i].size());
        lineLengths[i] = lineLength;
        shaderSource[i] = (char *)std::malloc(lineLength * sizeof(char));
        for (int j = 0; j < static_cast<int>(lines[i].size()); j++) {
            shaderSource[i][j] = lines[i][j];
        }
    }

    return ShaderCode(lineCount, lineLengths, shaderSource, filePath.c_str());
}

void Shader::compile() {
    GLuint newProgramId = 0;
    GL_Call(newProgramId = glCreateProgram());
    std::cout << "Compiling shader program" << std::endl;

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
        std::cout << &programErrorMessage[0];
        std::cout.flush();
    }

    for (const auto &shaderId : shaderIds) {
        GL_Call(glDetachShader(newProgramId, shaderId));
        GL_Call(glDeleteShader(shaderId));
    }

    if (success != 0) {
        if (id != 0) {
            GL_Call(glDeleteProgram(id));
        }

        uniformLocations = {};
        attributeLocations = {};

        // assign new id at the very end to be able to tolerate failed compilation or failed linking
        id = newProgramId;
        std::cout << "Compiling shader program - Done" << std::endl;
    } else {
        std::cout << "Failed to compile shader program" << std::endl;
    }
}

ShaderCode insertShaderCode(const ShaderCode &original, const ShaderCode &import, int line) {
    ShaderCode result = {};
    result.filePath = original.filePath;
    result.lineCount = original.lineCount + import.lineCount - 1;
    result.lineLengths = reinterpret_cast<int *>(std::malloc(result.lineCount * sizeof(int)));
    result.shaderSource = reinterpret_cast<char **>(std::malloc(result.lineCount * sizeof(char *)));
    for (int i = 0; i < result.lineCount; i++) {
        int lineLength = 0;
        char *shaderSource = nullptr;
        if (i >= line && i < line + import.lineCount) {
            lineLength = import.lineLengths[i - line];
            shaderSource = import.shaderSource[i - line];
        } else if (i >= line + import.lineCount) {
            lineLength = original.lineLengths[i - import.lineCount + 1];
            shaderSource = original.shaderSource[i - import.lineCount + 1];
        } else {
            lineLength = original.lineLengths[i];
            shaderSource = original.shaderSource[i];
        }
        result.lineLengths[i] = lineLength;
        result.shaderSource[i] = shaderSource;
    }
    return result;
}

GLuint Shader::load(GLuint shaderType, const ShaderCode &shaderCode) {
    GLuint shaderId = 0;
    GL_Call(shaderId = glCreateShader(shaderType));

#if 0
    std::cout << "----------- Shader code -----------" << std::endl;
    for (int i = 0; i < shaderCode.lineCount; i++) {
        for (int j = 0; j < shaderCode.lineLengths[i]; j++) {
            std::cout << shaderCode.shaderSource[i][j];
        }
    }
    std::cout << "----------- End -----------" << std::endl;
#endif

    ShaderCode finalShaderCode = shaderCode;
    for (int i = 0; i < shaderCode.lineCount; i++) {
        std::string sourceLine = std::basic_string(shaderCode.shaderSource[i], shaderCode.lineLengths[i]);
        if (sourceLine.size() < 12) {
            continue;
        }
        if (sourceLine.substr(0, 10) != "#include \"") {
            continue;
        }
        std::string importString = sourceLine.substr(10, sourceLine.find_last_of('"') - 10);
        ShaderCode &importCode = shaderLibs[importString];
        finalShaderCode = insertShaderCode(shaderCode, importCode, i);
    }

    GL_Call(glShaderSource(shaderId, finalShaderCode.lineCount, finalShaderCode.shaderSource,
                           finalShaderCode.lineLengths));
    GL_Call(glCompileShader(shaderId));

    int success = 0;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
    if (success == 0) {
        std::cerr << "  Failed to compile " << shaderTypeToString(shaderType) << " shader (" << finalShaderCode.filePath
                  << ")" << std::endl;
    }

    int infoLogLength = 0;
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    if (success == 0) {
        return 0;
    }

    std::cout << "  Compiled " << shaderTypeToString(shaderType) << " shader (" << finalShaderCode.filePath << ")"
              << std::endl;
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

void Shader::bind() {
    bool recompile = false;
    // NOTE we need to wait a little bit (100ms) before actually reading the file, because Windows gives us an empty
    // file otherwise
    const int64_t waitTimeNano = 200 * 1000 * 1000;
    const auto currentTime = std::chrono::system_clock::now();
    const int64_t currentTimeNano =
          std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime).time_since_epoch().count();

    for (auto &entry : shaderComponents) {
        const int64_t lastAccessTimeNano = getLastModifiedTimeNano(entry.second.filePath);
        if (lastAccessTimeNano > entry.second.lastAccessTimeNano &&
            currentTimeNano > lastAccessTimeNano + waitTimeNano) {
            entry.second = readShaderCodeFromFile(entry.second.filePath);
            recompile = true;
            break;
        }
    }
    if (id == 0 || recompile) {
        compile();
    }

    GL_Call(glUseProgram(id));
}

void Shader::unbind() { GL_Call(glUseProgram(0)); }

void Shader::attach(GLuint shaderType, const ShaderCode &shaderCode) { shaderComponents[shaderType] = shaderCode; }

void Shader::attachShaderLib(const ShaderCode &shaderCode) {
    std::string key = shaderCode.filePath;
    int lastSlash = key.find_last_of('/');
    key = key.substr(lastSlash + 1);
    shaderLibs[key] = shaderCode;
}

std::shared_ptr<Shader> createDefaultShader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode) {
    auto result = std::make_shared<Shader>();
    result->attachVertexShader(vertexCode);
    result->attachFragmentShader(fragmentCode);
    return result;
}
