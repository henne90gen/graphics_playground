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
