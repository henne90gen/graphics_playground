#pragma once

#include "util/OpenGLUtils.h"

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

#define SHADER_DEFINITION(name)                                                                                        \
    extern const unsigned int name##_len;                                                                              \
    extern const char *(name)[];                                                                                       \
    extern const int name##_line_lens[];

#define SHADER(vertexName, fragmentName)                                                                               \
    std::make_shared<Shader>(ShaderCode(vertexName##_len, vertexName##_line_lens, vertexName),                         \
                             ShaderCode(fragmentName##_len, fragmentName##_line_lens, fragmentName))

enum ShaderDataType { Float = 0, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool };

struct ShaderCode {
    const unsigned int lineCount;
    const int *lineLengths;
    const char **shaderSource;

    ShaderCode(const unsigned int lineCount, const int *lineLengths, const char **shaderSource)
        : lineCount(lineCount), lineLengths(lineLengths), shaderSource(shaderSource) {}
};

class Shader {
  private:
    unsigned int id;

    std::unordered_map<std::string, int> uniformLocations;
    std::unordered_map<std::string, int> attributeLocations;

  public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    Shader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode);

    ~Shader() {}

    void bind();
    static void unbind();

    inline unsigned int getId() { return id; }

    void setUniform(const std::string &name, float f) { GL_Call(glUniform1f(getUniformLocation(name), f)); }

    void setUniform(const std::string &name, int i) { GL_Call(glUniform1i(getUniformLocation(name), i)); }

    void setUniform(const std::string &name, bool b) { setUniform(name, (int)b); }

    void setUniform(const std::string &name, glm::vec3 v) {
        GL_Call(glUniform3f(getUniformLocation(name), v.x, v.y, v.z));
    }

    void setUniform(const std::string &name, glm::vec4 v) {
        GL_Call(glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w));
    }

    void setUniform(const std::string &name, glm::mat4 m) {
        GL_Call(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, (float *)&m));
    }

    void setUniform(const std::string &name, glm::mat3 m) {
        GL_Call(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, (float *)&m));
    }

    int getAttributeLocation(const std::string &name);

  private:
    void compile(const std::string &vertexPath, const std::string &fragmentPath);
    void compile(const ShaderCode &vertexCode, const ShaderCode &fragmentSource);

    static GLuint load(GLuint shaderType, const std::string &filePath);
    static GLuint load(GLuint shaderType, const ShaderCode &shaderSource);

    int getUniformLocation(const std::string &name);
};
