#pragma once

#include <vector>
#include <string>

struct Image {
    unsigned int width;
    unsigned int height;
    char channels;
    char bitDepth;
    std::vector<unsigned char> pixels;
};

int loadPng(const std::string &fileName, Image &image);

int loadJpg(const std::string &fileName, Image &image);

int loadImage(const std::string &fileName, Image &image);
