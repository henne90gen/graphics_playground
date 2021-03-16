#pragma once

#include "Texture.h"
#include "util/FileUtils.h"
#include "util/OpenGLUtils.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#define DEFINE_SHADER(name)                                                                                            \
    extern "C" const unsigned int name##_len;                                                                          \
    extern "C" char *name[];                                                                                           \
    extern "C" int name##_line_lens[];                                                                                 \
    extern "C" const char *name##_file_path;

#define DEFINE_VERTEX_SHADER(name) DEFINE_SHADER(name##Vert)
#define DEFINE_FRAGMENT_SHADER(name) DEFINE_SHADER(name##Frag)
#define DEFINE_TESS_CONTROL_SHADER(name) DEFINE_SHADER(name##Tcs)
#define DEFINE_TESS_EVALUATION_SHADER(name) DEFINE_SHADER(name##Tes)

#define DEFINE_DEFAULT_SHADERS(name)                                                                                   \
    DEFINE_VERTEX_SHADER(name)                                                                                         \
    DEFINE_FRAGMENT_SHADER(name)

#define SHADER_CODE(name) ShaderCode(name##_len, name##_line_lens, name, name##_file_path)

#define CREATE_DEFAULT_SHADER(name) createDefaultShader(SHADER_CODE(name##Vert), SHADER_CODE(name##Frag))

enum ShaderDataType { Float = 0, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool };

struct ShaderCode {
    unsigned int lineCount = 0;
    int *lineLengths = nullptr;
    char **shaderSource = nullptr;
    std::string filePath = "";
    int64_t lastAccessTimeNano = 0;

    ShaderCode() = default;
    ShaderCode(const ShaderCode &code) = default;
    ShaderCode(const unsigned int lineCount, int *lineLengths, char **shaderSource, const char *filePath)
        : lineCount(lineCount), lineLengths(lineLengths), shaderSource(shaderSource), filePath(filePath),
          lastAccessTimeNano(getLastModifiedTimeNano(filePath)) {}
    ShaderCode(const unsigned int lineCount, int *lineLengths, char **shaderSource, std::string filePath)
        : lineCount(lineCount), lineLengths(lineLengths), shaderSource(shaderSource), filePath(std::move(filePath)),
          lastAccessTimeNano(getLastModifiedTimeNano(filePath)) {}
};

class Shader {
  private:
    unsigned int id;

    std::unordered_map<std::string, int> uniformLocations;
    std::unordered_map<std::string, int> attributeLocations;
    std::unordered_map<GLuint, ShaderCode> shaderComponents = {};
    std::unordered_map<std::string, ShaderCode> shaderLibs = {};

  public:
    explicit Shader() : id(0) {}

    ~Shader() = default;

    void bind();
    static void unbind();

    [[nodiscard]] inline unsigned int getId() const { return id; }

    void compile();

    void setUniform(const std::string &name, const std::shared_ptr<Texture> &texture, const GLuint slot) {
        GL_Call(glActiveTexture(slot));
        texture->bind();
        setUniform(name, static_cast<int>(slot - 0x84C0));
    }
    void setUniform(const std::string &name, float f) { GL_Call(glUniform1f(getUniformLocation(name), f)); }
    void setUniform(const std::string &name, int i) { GL_Call(glUniform1i(getUniformLocation(name), i)); }
    void setUniform(const std::string &name, bool b) { setUniform(name, static_cast<int>(b)); }
    void setUniform(const std::string &name, const glm::vec3 &v) {
        GL_Call(glUniform3f(getUniformLocation(name), v.x, v.y, v.z));
    }
    void setUniform(const std::string &name, const glm::vec4 &v) {
        GL_Call(glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w));
    }
    void setUniform(const std::string &name, const glm::quat &q) {
        GL_Call(glUniform4f(getUniformLocation(name), q.x, q.y, q.z, q.w));
    }
    void setUniform(const std::string &name, const glm::mat4 &m) {
        GL_Call(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, (float *)&m));
    }
    void setUniform(const std::string &name, const glm::mat3 &m) {
        GL_Call(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, (float *)&m));
    }

    int getAttributeLocation(const std::string &name);

    void attachVertexShader(const ShaderCode &shaderCode) { this->attach(GL_VERTEX_SHADER, shaderCode); }
    void attachFragmentShader(const ShaderCode &shaderCode) { this->attach(GL_FRAGMENT_SHADER, shaderCode); }
    void attachTessControlShader(const ShaderCode &shaderCode) { this->attach(GL_TESS_CONTROL_SHADER, shaderCode); }
    void attachTessEvaluationShader(const ShaderCode &shaderCode) {
        this->attach(GL_TESS_EVALUATION_SHADER, shaderCode);
    }
    void attachComputeShader(const ShaderCode &shaderCode) { this->attach(GL_COMPUTE_SHADER, shaderCode); }
    void attachShaderLib(const ShaderCode &shaderCode);

  private:
    GLuint load(GLuint shaderType, const ShaderCode &shaderCode);

    void attach(GLuint shaderType, const ShaderCode &shaderCode);

    int getUniformLocation(const std::string &name);
};

std::shared_ptr<Shader> createDefaultShader(const ShaderCode &vertexCode, const ShaderCode &fragmentCode);
