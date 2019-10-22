#include "Texture.h"

#include <iostream>

Texture::Texture(unsigned int dataType) : dataType(dataType) { GL_Call(glGenTextures(1, &id)); }

Texture::~Texture() = default;

void Texture::bind() const { GL_Call(glBindTexture(GL_TEXTURE_2D, id)); }

void Texture::unbind() const { GL_Call(glBindTexture(GL_TEXTURE_2D, 0)); }

void Texture::update(const unsigned char *data, unsigned int width, unsigned int height, unsigned int unpackAlignment) {
    bind();
    if (unpackAlignment != 4) {
        ASSERT(unpackAlignment == 1 || unpackAlignment == 2 || unpackAlignment == 8);
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment));
    }

    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, dataType, width, height, 0, dataType, GL_UNSIGNED_BYTE, data));

    if (unpackAlignment != 4) {
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }
}

void Texture::update(const std::vector<glm::vec4> &data, unsigned int width, unsigned int height,
                     unsigned int unpackAlignment) {
    bind();
    if (unpackAlignment != 4) {
        ASSERT(unpackAlignment == 1 || unpackAlignment == 2 || unpackAlignment == 8);
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment));
    }

    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, dataType, width, height, 0, dataType, GL_FLOAT, data.data()));

    if (unpackAlignment != 4) {
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }
}

void Texture::update(Image &image) { update(image.pixels.data(), image.width, image.height); }

void Texture::update(const std::vector<glm::vec3> &data, unsigned int width, unsigned int height,
                     unsigned int unpackAlignment) {
    bind();
    if (unpackAlignment != 4) {
        ASSERT(unpackAlignment == 1 || unpackAlignment == 2 || unpackAlignment == 8);
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment));
    }

    if (width * height != data.size()) {
        std::cout << "Warning: width*height does not match size of data array." << std::endl;
    }

    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, dataType, width, height, 0, dataType, GL_FLOAT, data.data()));

    if (unpackAlignment != 4) {
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }
}
