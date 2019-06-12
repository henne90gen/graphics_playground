#pragma once

#include "util/OpenGLUtils.h"

class Texture {
public:
    Texture(unsigned int dataType = GL_RGB);

    ~Texture();

    void update(const char *data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
    unsigned int dataType;
};
