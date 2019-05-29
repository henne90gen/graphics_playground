#include "IndexBuffer.h"

#include "Utils.h"

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count) : count(count) {
    create(data);
}

IndexBuffer::IndexBuffer(const std::vector<unsigned int> &data) : count(data.size()) {
    create(data.data());
}

IndexBuffer::~IndexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void IndexBuffer::bind() const { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); }

void IndexBuffer::unbind() const { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }

void IndexBuffer::create(const unsigned int *data) {
    GL_Call(glGenBuffers(1, &id));
    bind();
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

