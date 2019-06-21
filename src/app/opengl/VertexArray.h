#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

class VertexArray {
public:
    VertexArray();

    ~VertexArray();

    void addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer);

    void setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
    std::shared_ptr<IndexBuffer> indexBuffer;
};
