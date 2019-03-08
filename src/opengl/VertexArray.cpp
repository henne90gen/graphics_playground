#include "opengl/VertexArray.h"

#include "OpenGLUtils.h"

VertexArray::VertexArray() { GL_Call(glGenVertexArrays(1, &id)); }

VertexArray::~VertexArray() { GL_Call(glDeleteVertexArrays(1, &id)); }

void VertexArray::addBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout) {
    bind();

    vertexBuffer.bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto &element = elements[i];
        GL_Call(glEnableVertexAttribArray(i));
        GL_Call(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(),
                                      (const void *)offset));
        offset += element.count * element.getSize();
    }
}

void VertexArray::bind() const { GL_Call(glBindVertexArray(id)); }

void VertexArray::unbind() const { GL_Call(glBindVertexArray(0)); }
