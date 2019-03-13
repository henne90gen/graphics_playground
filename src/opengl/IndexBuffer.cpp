#include "opengl/IndexBuffer.h"

#include "Utils.h"

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count) : count(count) {
    GL_Call(glGenBuffers(1, &id));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void IndexBuffer::bind() const { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); }

void IndexBuffer::unbind() const { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }
