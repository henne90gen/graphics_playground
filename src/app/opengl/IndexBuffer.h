#pragma once

#include <vector>

class IndexBuffer {
public:
    IndexBuffer();

    IndexBuffer(const unsigned int *data, unsigned int count);

    explicit IndexBuffer(const std::vector<unsigned int> &data);

    ~IndexBuffer();

    void bind() const;

    void unbind() const;

    unsigned int getCount() const { return count; }

    void update(const unsigned int *data, unsigned int count);

    void update(const std::vector<unsigned int> &data);

private:
    unsigned int id = 0;
    unsigned int count = 0;
};
