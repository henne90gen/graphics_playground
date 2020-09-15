#pragma once

#include "util/OpenGLUtils.h"

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

enum ShaderDataType { Float = 0, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool };

class Shader {
  private:
    unsigned int id;
    std::string vertexPath;
    std::string fragmentPath;

    long lastModTimeVertex;
    long lastModTimeFragment;
    std::unordered_map<std::string, int> uniformLocations;
    std::unordered_map<std::string, int> attributeLocations;

  public:
    Shader(std::string vertexPath, std::string fragmentPath);
    Shader(unsigned char *vertexSource, unsigned int vertexSourceLength, unsigned char *fragmentSource,
           unsigned int fragmentSourceLength) {}

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
    void compile();

    static GLuint load(GLuint shaderType, std::string &filePath);

    int getUniformLocation(const std::string &name);

    bool hasBeenModified();
};
