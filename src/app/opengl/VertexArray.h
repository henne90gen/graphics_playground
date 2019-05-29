#pragma once

// #include "OpenGLUtils.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray {
public:
    VertexArray();

    ~VertexArray();

    void addBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
};
