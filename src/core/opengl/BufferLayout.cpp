#include "BufferLayout.h"

BufferLayout::BufferLayout(const std::initializer_list<BufferLayoutElement> &elements) : elements(elements) {
    for (auto &element : elements) {
        stride += element.getSize();
    }
}
