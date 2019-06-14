#pragma once

#include <vector>
#include <glm/ext.hpp>

class VertexBuffer {
public:
    VertexBuffer();

    VertexBuffer(const void *data, unsigned int size);

    explicit VertexBuffer(const std::vector<float>& data);

    ~VertexBuffer();

    void update(const void *data, unsigned int sizeInBytes);

    void update(const std::vector<float> &data);

    void update(const std::vector<glm::vec3> &data);

    void update(const std::vector<glm::vec2> &data);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
};
