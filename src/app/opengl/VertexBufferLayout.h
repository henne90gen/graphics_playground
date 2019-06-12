#pragma once

#include "util/OpenGLUtils.h"

#include <iostream>
#include <string>
#include <vector>

#include "Shader.h"

struct LayoutElement {
    unsigned int type;
    unsigned int count;
    bool normalized;
    int location;

    unsigned int getSize() const {
        switch (type) {
            case GL_FLOAT:
                return 4;
            case GL_UNSIGNED_INT:
                return 4;
            case GL_UNSIGNED_BYTE:
                return 1;
        }
        ASSERT(false);
        return 0;
    }
};

class VertexBufferLayout {
public:
    VertexBufferLayout() : stride(0) {}

    ~VertexBufferLayout() {}

    template<typename T>
    void add(Shader *shader, const std::string &name, unsigned int count) {
        // static_assert(false, "Please use one of the specialized templates");
    }

    template<>
    void add<float>(Shader *shader, const std::string &name, unsigned int count) {
        int location = getLocation(shader, name);
        elements.push_back({GL_FLOAT, count, GL_FALSE, location});
        stride += count * sizeof(GLfloat);
    }

    template<>
    void add<unsigned int>(Shader *shader, const std::string &name, unsigned int count) {
        int location = getLocation(shader, name);
        elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE, location});
        stride += count * sizeof(GLuint);
    }

    template<>
    void add<unsigned char>(Shader *shader, const std::string &name, unsigned int count) {
        int location = getLocation(shader, name);
        elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE, location});
        stride += count * sizeof(GLbyte);
    }

    inline unsigned int getStride() const { return stride; }

    inline const std::vector<LayoutElement> getElements() const { return elements; }

private:
    std::vector<LayoutElement> elements;
    unsigned int stride;

    int getLocation(Shader *shader, const std::string &name) {
        GL_Call(int location = glGetAttribLocation(shader->getId(), name.c_str()));
        if (location == -1) {
            std::cout << "Warning: could not find attribute '" << name << "'" << std::endl;
        }
        return location;
    }
};
