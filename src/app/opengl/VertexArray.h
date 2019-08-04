#pragma once

#include "VertexBuffer.h"
#include "BufferLayout.h"
#include "IndexBuffer.h"

class VertexArray {
public:
    explicit VertexArray(std::shared_ptr<Shader> s);

    ~VertexArray();

    void setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer);

    void addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer);

    void bind() const;

    void unbind() const;

    std::shared_ptr<IndexBuffer> getIndexBuffer() const { return indexBuffer; }

private:
    unsigned int id = 0;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
};
