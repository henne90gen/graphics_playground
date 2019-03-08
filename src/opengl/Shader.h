#pragma once

#include "OpenGLUtils.h"

#include <string>
#include <unordered_map>

class Shader {
  private:
    unsigned int id;
    std::string vertexPath;
    std::string fragmentPath;
    std::unordered_map<std::string, int> uniformLocations;

  public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    ~Shader() {}

    void bind() const;

    void unbind() const;

    inline unsigned int getId() { return id; }

    template <typename T>
    void setUniform(const std::string &name, T f) {}

    template <> void setUniform<float>(const std::string &name, float f) { GL_Call(glUniform1f(getUniformLocation(name), f)); }

    template <> void setUniform<int>(const std::string &name, int i) { GL_Call(glUniform1i(getUniformLocation(name), i)); }

  private:
    void compile();
    GLuint load(GLuint shaderType, std::string &filePath);
    int getUniformLocation(const std::string &name);
};
