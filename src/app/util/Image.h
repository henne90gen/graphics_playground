#pragma once

#include <string>
#include <vector>

struct Image {
    std::string fileName;
    unsigned int width = 0;
    unsigned int height = 0;
    char channels = 3;
    char bitDepth = 8;
    std::vector<unsigned char> pixels;
};

namespace ImageOps {
bool load(const std::string &fileName, Image &image);

void save(Image &image);
} // namespace ImageOps
