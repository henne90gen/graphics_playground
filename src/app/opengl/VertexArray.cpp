#include <utility>

#include "VertexArray.h"

#include "util/OpenGLUtils.h"

VertexArray::VertexArray(std::shared_ptr<Shader> s) : shader(std::move(s)) {
    GL_Call(glGenVertexArrays(1, &id));
}

VertexArray::~VertexArray() { GL_Call(glDeleteVertexArrays(1, &id)); }

int getLocation(const std::shared_ptr<Shader> &shader, const std::string &name) {
    GL_Call(int location = glGetAttribLocation(shader->getId(), name.c_str()));
    if (location == -1) {
        std::cout << "Warning: could not find attribute '" << name << "'" << std::endl;
    }
    return location;
}

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) {
    if (!shader) {
        std::cout << "There is no shader associated with this vertex array" << std::endl;
        ASSERT(false);
    }

    bind();

    vertexBuffer->bind();
    const auto &layout = vertexBuffer->getLayout();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (auto &element : elements) {
        int location = getLocation(shader, element.getName());
        GL_Call(glEnableVertexAttribArray(location));
        GL_Call(glVertexAttribPointer(
                location,
                element.getCount(),
                element.getDataType(),
                element.getNormalized(),
                layout.getStride(),
                reinterpret_cast<GLvoid *>(offset) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        ));
        offset += element.getSize();
    }

    vertexBuffers.push_back(vertexBuffer);
}

void VertexArray::bind() const { GL_Call(glBindVertexArray(id)); }

void VertexArray::unbind() const { GL_Call(glBindVertexArray(0)); }

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer) {
    buffer->bind();
    this->indexBuffer = buffer;
}
