#include "BufferLayoutElement.h"

unsigned int BufferLayoutElement::getSize() const {
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
    return 0;
}

GLenum BufferLayoutElement::getDataType() const {
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
    return 0;
}

unsigned int BufferLayoutElement::getCount() const {
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
    return 0;
}
