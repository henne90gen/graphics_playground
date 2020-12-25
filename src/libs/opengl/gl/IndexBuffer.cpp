#include "IndexBuffer.h"

#include "util/OpenGLUtils.h"

IndexBuffer::IndexBuffer() { GL_Call(glGenBuffers(1, &id)); }

IndexBuffer::IndexBuffer(const unsigned int *data, const unsigned int count) {
    GL_Call(glGenBuffers(1, &id));
    update(data, count);
}

IndexBuffer::IndexBuffer(const std::vector<unsigned int> &data) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

IndexBuffer::IndexBuffer(const std::vector<glm::ivec3> &data) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

IndexBuffer::~IndexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void IndexBuffer::bind() const { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); }

void IndexBuffer::unbind() { GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }

void IndexBuffer::update(const unsigned int *data, const unsigned int countParam) {
    count = countParam;
    bind();
    unsigned int sizeInBytes = count * sizeof(unsigned int);
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW));
}

void IndexBuffer::update(const std::vector<unsigned int> &data) {
    count = data.size();
    bind();
    unsigned int sizeInBytes = count * sizeof(unsigned int);
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void IndexBuffer::update(const std::vector<glm::ivec3> &data) {
    count = data.size() * 3;
    bind();
    unsigned int sizeInBytes = count * sizeof(unsigned int);
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}
