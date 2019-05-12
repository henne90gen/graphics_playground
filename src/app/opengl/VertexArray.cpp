#include "VertexArray.h"

#include "Utils.h"

VertexArray::VertexArray() { GL_Call(glGenVertexArrays(1, &id)); }

VertexArray::~VertexArray() { GL_Call(glDeleteVertexArrays(1, &id)); }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
void VertexArray::addBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout) {
    bind();

    vertexBuffer.bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto &element = elements[i];
        GL_Call(glEnableVertexAttribArray(element.location));
        GL_Call(glVertexAttribPointer(element.location, element.count, element.type, element.normalized,
                                      layout.getStride(), (const GLvoid *) offset));
        offset += element.count * element.getSize();
    }
}
#pragma clang diagnostic pop

void VertexArray::bind() const { GL_Call(glBindVertexArray(id)); }

void VertexArray::unbind() const { GL_Call(glBindVertexArray(0)); }
