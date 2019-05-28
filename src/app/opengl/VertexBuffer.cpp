#include "VertexBuffer.h"

#include "Utils.h"

VertexBuffer::VertexBuffer() { GL_Call(glGenBuffers(1, &id)); }

VertexBuffer::VertexBuffer(const std::vector<float>& data) {
    GL_Call(glGenBuffers(1, &id));
    update(data.data(), data.size() * sizeof(float));
}

VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
    GL_Call(glGenBuffers(1, &id));
    update(data, size);
}

void VertexBuffer::update(const void *data, unsigned int sizeInBytes) {
    bind();
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void VertexBuffer::bind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, id)); }

void VertexBuffer::unbind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); }
