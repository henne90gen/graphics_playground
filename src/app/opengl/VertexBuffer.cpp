#include <utility>

#include "VertexBuffer.h"

#include "util/OpenGLUtils.h"

VertexBuffer::VertexBuffer() { GL_Call(glGenBuffers(1, &id)); }

VertexBuffer::VertexBuffer(const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
}

VertexBuffer::VertexBuffer(const void *data, unsigned int sizeInBytes, const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data, sizeInBytes);
}

VertexBuffer::VertexBuffer(const std::vector<float> &data, const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::VertexBuffer(const std::vector<glm::vec2> &data, const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::VertexBuffer(const std::vector<glm::vec3> &data, const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::VertexBuffer(const std::vector<glm::vec4> &data, const BufferLayout &l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::~VertexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void VertexBuffer::bind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, id)); }

void VertexBuffer::unbind() { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

void VertexBuffer::update(const void *data, unsigned int sizeInBytes) const {
    bind();
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<float> &data) const {
    bind();
    unsigned int sizeInBytes = data.size() * sizeof(float);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<glm::vec2> &data) const {
    bind();
    unsigned int sizeInBytes = data.size() * sizeof(glm::vec2);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<glm::vec3> &data) const {
    bind();
    unsigned int sizeInBytes = data.size() * sizeof(glm::vec3);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<glm::vec4> &data) const {
    bind();
    unsigned int sizeInBytes = data.size() * sizeof(glm::vec4);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}
