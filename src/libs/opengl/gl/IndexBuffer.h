#pragma once

#include <glm/ext.hpp>
#include <vector>

class IndexBuffer {
  public:
    IndexBuffer();

    explicit IndexBuffer(const unsigned int *data, unsigned int count);
    explicit IndexBuffer(const std::vector<unsigned int> &data);
    explicit IndexBuffer(const std::vector<glm::ivec3> &data);

    ~IndexBuffer();

    void bind() const;
    static void unbind();

    [[nodiscard]] unsigned int getGLID() const { return id; }
    [[nodiscard]] unsigned int getCount() const { return count; }

    void update(const unsigned int *data, unsigned int count);
    void update(const std::vector<unsigned int> &data);
    void update(const std::vector<glm::ivec3> &data);

  private:
    unsigned int id = 0;
    unsigned int count = 0;
};
