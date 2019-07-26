#pragma once

#include <vector>
#include <string>

struct Image {
    std::string fileName;
    unsigned int width;
    unsigned int height;
    char channels = 3;
    char bitDepth = 8;
    std::vector<unsigned char> pixels;
};

namespace ImageOps {
    bool load(const std::string &fileName, Image &image);

    void save(Image &image);
}
