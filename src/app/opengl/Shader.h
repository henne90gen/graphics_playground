#pragma once

#include "Utils.h"

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

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

    template<typename T>
    void setUniform(const std::string &name, T f) { ASSERT(false); }

    template<>
    void setUniform<float>(const std::string &name, float f) { GL_Call(glUniform1f(getUniformLocation(name), f)); }

    template<>
    void setUniform<int>(const std::string &name, int i) { GL_Call(glUniform1i(getUniformLocation(name), i)); }

    template<>
    void setUniform<glm::vec3>(const std::string &name, glm::vec3 v) {
        GL_Call(glUniform3f(getUniformLocation(name), v.x, v.y, v.z));
    }

    template<>
    void setUniform<glm::mat4>(const std::string &name, glm::mat4 m) {
        GL_Call(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, (float *) &m));
    }

private:
    void compile();

    GLuint load(GLuint shaderType, std::string &filePath);

    int getUniformLocation(const std::string &name);
};
