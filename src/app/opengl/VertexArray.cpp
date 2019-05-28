#include "VertexArray.h"

#include "Utils.h"

VertexArray::VertexArray() { GL_Call(glGenVertexArrays(1, &id)); }

VertexArray::~VertexArray() { GL_Call(glDeleteVertexArrays(1, &id)); }

void VertexArray::addBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout) {
    bind();

    vertexBuffer.bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (auto &element : elements) {
        GL_Call(glEnableVertexAttribArray(element.location));
        GL_Call(glVertexAttribPointer(
                element.location,
                element.count,
                element.type,
                element.normalized,
                layout.getStride(),
                reinterpret_cast<GLvoid *>(offset) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        ));
        offset += element.count * element.getSize();
    }
}

void VertexArray::bind() const { GL_Call(glBindVertexArray(id)); }

void VertexArray::unbind() const { GL_Call(glBindVertexArray(0)); }
