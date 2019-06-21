#pragma once

#include "Shader.h"

class BufferLayoutElement {
public:
    BufferLayoutElement(
            ShaderDataType type,
            std::string name,
            bool normalized = false
    ) : dataType(type),
        name(std::move(name)),
        normalized(normalized) {}

    unsigned int getSize() const;

    unsigned int getCount() const;

    GLenum getDataType() const;

    inline bool getNormalized() const { return normalized; }

    inline std::string getName() const { return name; }

private:
    ShaderDataType dataType;
    std::string name;
    bool normalized;
};
