#pragma once

#include <vector>
#include <glm/ext.hpp>

class IndexBuffer {
public:
    IndexBuffer();

    IndexBuffer(const unsigned int *data, unsigned int count);

    explicit IndexBuffer(const std::vector<unsigned int> &data);

    explicit IndexBuffer(const std::vector<glm::ivec3> &data);

    ~IndexBuffer();

    void bind() const;

    void unbind() const;

    unsigned int getCount() const { return count; }

    void update(const unsigned int *data, unsigned int count);

    void update(const std::vector<unsigned int> &data);

    void update(const std::vector<glm::ivec3> &data);

private:
    unsigned int id = 0;
    unsigned int count = 0;
};
