#pragma once

#include <vector>
#include <string>

struct Image {
    unsigned int width;
    unsigned int height;
    char channels;
    char bitDepth;
    std::vector<char> pixels;
};

int loadPng(const std::string &file_name, Image &image);
