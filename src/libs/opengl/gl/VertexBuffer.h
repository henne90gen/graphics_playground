#pragma once

#include "BufferLayout.h"
#include <glm/ext.hpp>
#include <vector>

class VertexBuffer {
  public:
    VertexBuffer();

    explicit VertexBuffer(const BufferLayout &l);
    explicit VertexBuffer(const void *data, unsigned int sizeInBytes, const BufferLayout &l);
    explicit VertexBuffer(const std::vector<float> &data, const BufferLayout &l);
    explicit VertexBuffer(const std::vector<glm::vec2> &data, const BufferLayout &l);
    explicit VertexBuffer(const std::vector<glm::vec3> &data, const BufferLayout &l);
    explicit VertexBuffer(const std::vector<glm::vec4> &data, const BufferLayout &l);

    ~VertexBuffer();

    void update(const void *data, unsigned int sizeInBytes) const;
    void update(const std::vector<float> &data) const;
    void update(const std::vector<glm::vec2> &data) const;
    void update(const std::vector<glm::vec3> &data) const;
    void update(const std::vector<glm::vec4> &data) const;

    void setLayout(const BufferLayout &l) { this->layout = l; };
    [[nodiscard]] const BufferLayout &getLayout() const { return layout; }

    [[nodiscard]] unsigned int getGLID() const { return id; }

    void bind() const;
    static void unbind();

  private:
    unsigned int id = 0;
    BufferLayout layout;
};
