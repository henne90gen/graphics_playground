#pragma once

#include <memory>
#include <string>
#include <vector>

#include <gl/Texture.h>

struct Image {
    std::string fileName;
    unsigned int width = 0;
    unsigned int height = 0;
    char channels = 3;
    char bitDepth = 8;
    std::vector<uint8_t> pixels;

    void applyToTexture(std::shared_ptr<Texture> &texture) const {
        if (channels == 3) {
            texture->setDataType(GL_RGB);
        } else if (channels == 4) {
            texture->setDataType(GL_RGBA);
        }
        texture->update(pixels, width, height);
    }
};
