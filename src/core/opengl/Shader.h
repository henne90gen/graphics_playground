#pragma once

#include "util/OpenGLUtils.h"

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

#define DEFINE_SHADER(name)                                                                                            \
    extern "C" const unsigned int name##Vert_len;                                                                      \
    extern "C" char *name##Vert[];                                                                                     \
    extern "C" int name##Vert_line_lens[];                                                                             \
    extern "C" const unsigned int name##Frag_len;                                                                      \
    extern "C" char *name##Frag[];                                                                                     \
    extern "C" int name##Frag_line_lens[];

#define SHADER(name)                                                                                                   \
    std::make_shared<Shader>(ShaderCode(name##Vert_len, name##Vert_line_lens, name##Vert),                             \
                             ShaderCode(name##Frag_len, name##Frag_line_lens, name##Frag))

enum ShaderDataType { Float = 0, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool };

struct ShaderCode {
    unsigned int lineCount = 0;
    int *lineLengths = nullptr;
    char **shaderSource = nullptr;

    ShaderCode() = default;
    ShaderCode(ShaderCode &code) = default;
    ShaderCode(const unsigned int lineCount, int *lineLengths, char **shaderSource)
        : lineCount(lineCount), lineLengths(lineLengths), shaderSource(shaderSource) {}
};

class Shader {
  private:
    unsigned int id;

    std::unordered_map<std::string, int> uniformLocations;
    std::unordered_map<std::string, int> attributeLocations;

  public:
    explicit Shader(const std::string &vertexPath, const std::string &fragmentPath);
    explicit Shader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode);

    ~Shader() = default;

    void bind();
    static void unbind();

    [[nodiscard]] inline unsigned int getId() const { return id; }

    void attach(GLuint shaderType, const std::string &filePath);

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
    std::unordered_map<GLuint, ShaderCode> shaderComponents = {};

    void compile();
    static GLuint load(GLuint shaderType, const ShaderCode &shaderCode);

    int getUniformLocation(const std::string &name);
};
