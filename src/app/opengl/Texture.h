#pragma once

#include <util/Image.h>
#include "util/OpenGLUtils.h"

class Texture {
public:
    Texture(unsigned int dataType = GL_RGB);

    ~Texture();

    void update(const unsigned char *data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4);

    void update(const std::vector<glm::vec3> &data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4);

    void update(const std::vector<glm::vec4> &data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4);

    void update(Image &image);

    void bind() const;

    void unbind() const;

private:
    unsigned int id = 0;
    unsigned int dataType;
};
