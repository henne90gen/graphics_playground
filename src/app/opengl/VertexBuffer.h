#pragma once

#include <vector>

class VertexBuffer {
public:
    VertexBuffer();

    VertexBuffer(const void *data, unsigned int size);

    explicit VertexBuffer(const std::vector<float>& data);

    ~VertexBuffer();

    void update(const void *data, unsigned int sizeInBytes);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
};
