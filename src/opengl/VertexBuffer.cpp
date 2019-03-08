#include "opengl/VertexBuffer.h"

#include "OpenGLUtils.h"

VertexBuffer::VertexBuffer() { GL_Call(glGenBuffers(1, &id)); }

VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
    GL_Call(glGenBuffers(1, &id));
    update(data, size);
}

void VertexBuffer::update(const void *data, unsigned int size) {
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, id));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void VertexBuffer::bind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, id)); }

void VertexBuffer::unbind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); }
