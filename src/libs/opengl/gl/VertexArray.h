#pragma once

#include "BufferLayout.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

class VertexArray {
  public:
    explicit VertexArray(std::shared_ptr<Shader> s);
    ~VertexArray();

    [[nodiscard]] std::shared_ptr<IndexBuffer> &getIndexBuffer()  { return indexBuffer; }
    void setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer);

    void addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer);
    std::vector<std::shared_ptr<VertexBuffer>> &getVertexBuffers() { return vertexBuffers; }

    void bind() const;
    static void unbind();

    void setShader(std::shared_ptr<Shader> s);

  private:
    unsigned int id = 0;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
    void setupVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer);
};
