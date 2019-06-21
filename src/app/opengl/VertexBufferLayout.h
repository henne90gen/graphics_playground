#include <utility>

#include <utility>

#pragma once

#include "util/OpenGLUtils.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "Shader.h"

class LayoutElement {
public:
    LayoutElement(
            ShaderDataType type,
            std::string name,
            bool normalized = false
    ) : dataType(type),
        name(std::move(name)),
        normalized(normalized) {}

    unsigned int getSize() const {
        switch (dataType) {
            case Float:
            case Float2:
            case Float3:
            case Float4:
            case Int:
            case Int2:
            case Int3:
            case Int4:
                return 4 * getCount();
            case Bool:
                return 1;
        }
        ASSERT(false);
    }

    unsigned int getCount() const {
        switch (dataType) {
            case Float:
                return 1;
            case Float2:
                return 2;
            case Float3:
                return 3;
            case Float4:
                return 4;
            case Int:
                return 1;
            case Int2:
                return 2;
            case Int3:
                return 3;
            case Int4:
                return 4;
            case Bool:
                return 1;
        }
        ASSERT(false);
    }

    GLenum getDataType() const {
        switch (dataType) {
            case Float:
            case Float2:
            case Float3:
            case Float4:
                return GL_FLOAT;
            case Int:
            case Int2:
            case Int3:
            case Int4:
                return GL_INT;
            case Bool:
                return GL_BOOL;
        }
        ASSERT(false);
    }

    inline bool getNormalied() const { return normalized; }

    inline std::string getName() const { return name; }

private:
    ShaderDataType dataType;
    std::string name;
    bool normalized;
};

class VertexBufferLayout {
public:
    VertexBufferLayout() = default;

    VertexBufferLayout(const std::initializer_list<LayoutElement> &elements) : elements(elements) {
        for (auto &element : elements) {
            stride += element.getSize();
        }
    }

    ~VertexBufferLayout() = default;

    inline unsigned int getStride() const { return stride; }

    inline const std::vector<LayoutElement> getElements() const { return elements; }

private:
    std::vector<LayoutElement> elements;
    unsigned int stride = 0;
};
