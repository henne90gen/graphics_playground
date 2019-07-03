#pragma once

#include <vector>
#include <glm/ext.hpp>
#include "BufferLayout.h"

class VertexBuffer {
public:
    VertexBuffer();

    VertexBuffer(const void *data, unsigned int size, const BufferLayout& l);

    explicit VertexBuffer(const std::vector<float> &data, const BufferLayout& l);

    explicit VertexBuffer(const std::vector<glm::vec2> &data, const BufferLayout& l);

    explicit VertexBuffer(const std::vector<glm::vec3> &data, const BufferLayout& l);

    ~VertexBuffer();

    void update(const void *data, unsigned int sizeInBytes);

    void update(const std::vector<float> &data);

    void update(const std::vector<glm::vec3> &data);

    void update(const std::vector<glm::vec2> &data);

    void setLayout(const BufferLayout &l) { this->layout = l; };

    const BufferLayout &getLayout() const { return layout; }

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
    BufferLayout layout;
};
