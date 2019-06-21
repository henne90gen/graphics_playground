#pragma once

#include <vector>
#include <glm/ext.hpp>
#include "VertexBufferLayout.h"

class VertexBuffer {
public:
    VertexBuffer();

    VertexBuffer(const void *data, unsigned int size, const VertexBufferLayout& l);

    explicit VertexBuffer(const std::vector<float> &data, const VertexBufferLayout& l);

    explicit VertexBuffer(const std::vector<glm::vec3> &data, const VertexBufferLayout& l);

    ~VertexBuffer();

    void update(const void *data, unsigned int sizeInBytes);

    void update(const std::vector<float> &data);

    void update(const std::vector<glm::vec3> &data);

    void update(const std::vector<glm::vec2> &data);

    void setLayout(const VertexBufferLayout &l) { this->layout = l; };

    const VertexBufferLayout &getLayout() const { return layout; }

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;

    VertexBufferLayout layout;
};
