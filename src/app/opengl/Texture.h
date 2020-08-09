#pragma once

#include <util/Image.h>
#include "util/OpenGLUtils.h"

class Texture {
public:
    Texture(unsigned int dataType = GL_RGB);

    ~Texture() = default;

    void update(const unsigned char *data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4) const;

    void update(const std::vector<glm::vec3> &data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4) const;

    void update(const std::vector<glm::vec4> &data, unsigned int width, unsigned int height, unsigned int unpackAlignment = 4) const;

    void update(Image &image) const;

    void bind() const;

    static void unbind() ;

private:
    unsigned int id = 0;
    unsigned int dataType;
};
