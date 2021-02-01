#include "VertexArray.h"

#include <utility>

#include "util/OpenGLUtils.h"

VertexArray::VertexArray(std::shared_ptr<Shader> s) : shader(std::move(s)) { GL_Call(glGenVertexArrays(1, &id)); }

VertexArray::~VertexArray() { GL_Call(glDeleteVertexArrays(1, &id)); }

void VertexArray::bind() const { GL_Call(glBindVertexArray(id)); }

void VertexArray::unbind() { GL_Call(glBindVertexArray(0)); }

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer) {
    bind();
    buffer->bind();
    this->indexBuffer = buffer;
}

int getLocation(const std::shared_ptr<Shader> &shader, const std::string &name) {
    return shader->getAttributeLocation(name);
}

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) {
    if (!shader) {
        std::cerr << "There is no shader associated with this vertex array" << std::endl;
        ASSERT(false);
    }

    bind();
    shader->bind();
    setupVertexBuffer(vertexBuffer);
    vertexBuffers.push_back(vertexBuffer);
}

void VertexArray::setShader(std::shared_ptr<Shader> s) {
    shader = std::move(s);

    bind();
    shader->bind();

    for (const auto &vertexBuffer : vertexBuffers) {
        setupVertexBuffer(vertexBuffer);
    }
}

void VertexArray::setupVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) {
    vertexBuffer->bind();

    const auto &layout = vertexBuffer->getLayout();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (const auto &element : elements) {
        int location = getLocation(shader, element.getName());
        if (location != -1) {
            GL_Call(glEnableVertexAttribArray(location));
            GL_Call(glVertexAttribPointer(           //
                  location,                          //
                  element.getCount(),                //
                  element.getDataType(),             //
                  element.getNormalized(),           //
                  layout.getStride(),                //
                  reinterpret_cast<GLvoid *>(offset) //
                  ));
        }
        offset += element.getSize();
    }
}
