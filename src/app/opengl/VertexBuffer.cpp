#include <utility>

#include "VertexBuffer.h"

#include "util/OpenGLUtils.h"

VertexBuffer::VertexBuffer() { GL_Call(glGenBuffers(1, &id)); }

VertexBuffer::VertexBuffer(const std::vector<float> &data, const VertexBufferLayout& l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::VertexBuffer(const std::vector<glm::vec3> &data, const VertexBufferLayout& l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data);
}

VertexBuffer::VertexBuffer(const void *data, unsigned int size, const VertexBufferLayout& l) : layout(l) {
    GL_Call(glGenBuffers(1, &id));
    update(data, size);
}

VertexBuffer::~VertexBuffer() { GL_Call(glDeleteBuffers(1, &id)); }

void VertexBuffer::bind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, id)); }

void VertexBuffer::unbind() const { GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

void VertexBuffer::update(const void *data, unsigned int sizeInBytes) {
    bind();
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<float> &data) {
    bind();
    unsigned int sizeInBytes = data.size() * sizeof(float);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<glm::vec3> &data) {
    bind();
    int numFloatsPerVec3 = 3;
    unsigned int sizeInBytes = data.size() * numFloatsPerVec3 * sizeof(float);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}

void VertexBuffer::update(const std::vector<glm::vec2> &data) {
    bind();
    int numFloatsPerVec2 = 2;
    unsigned int sizeInBytes = data.size() * numFloatsPerVec2 * sizeof(float);
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data.data(), GL_STATIC_DRAW));
}
