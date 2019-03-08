#pragma once

// #include "OpenGLUtils.h"
#include "opengl/VertexBuffer.h"
#include "opengl/VertexBufferLayout.h"

class VertexArray {
  public:
    VertexArray();
    ~VertexArray();

    void addBuffer(const VertexBuffer &vertexBuffer, const VertexBufferLayout &layout);

    void bind() const;
    void unbind() const;

  private:
    unsigned int id;
};
