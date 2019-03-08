#pragma once

#include "OpenGLUtils.h"

#include <vector>

struct LayoutElement {
    unsigned int type;
    unsigned int count;
    bool normalized;

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

    template <typename T> void add(unsigned int count) {
        // static_assert(false, "Please use one of the specialized templates");
    }

    template <> void add<float>(unsigned int count) {
        elements.push_back({GL_FLOAT, count, GL_FALSE});
        stride += count * sizeof(GLfloat);
    }

    template <> void add<unsigned int>(unsigned int count) {
        elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
        stride += count * sizeof(GLuint);
    }

    template <> void add<unsigned char>(unsigned int count) {
        elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
        stride += count * sizeof(GLbyte);
    }

    inline unsigned int getStride() const { return stride; }

    inline const std::vector<LayoutElement> getElements() const { return elements; }

  private:
    std::vector<LayoutElement> elements;
    unsigned int stride;
};
