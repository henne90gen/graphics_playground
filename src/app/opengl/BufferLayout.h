#pragma once

#include "util/OpenGLUtils.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "Shader.h"
#include "BufferLayoutElement.h"

class BufferLayout {
public:
    BufferLayout() = default;

    BufferLayout(const std::initializer_list<BufferLayoutElement> &elements);

    ~BufferLayout() = default;

    inline unsigned int getStride() const { return stride; }

    inline const std::vector<BufferLayoutElement> getElements() const { return elements; }

private:
    unsigned int stride = 0;
    std::vector<BufferLayoutElement> elements;
};
