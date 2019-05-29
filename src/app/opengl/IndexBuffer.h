#pragma once

#include <vector>

class IndexBuffer {
public:
    IndexBuffer(const unsigned int *data, unsigned int count);

    explicit IndexBuffer(const std::vector<unsigned int> &data);

    ~IndexBuffer();

    void bind() const;

    void unbind() const;

    unsigned int getCount() const { return count; }

private:
    unsigned int id = 0;
    unsigned int count;

    void create(const unsigned int *data);
};
