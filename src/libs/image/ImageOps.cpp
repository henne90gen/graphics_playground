#include "ImageOps.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "bmp.cpp"
#include "jpeg.cpp"
#include "png.cpp"
#include "tga.cpp"

bool hasExtension(const std::string &fileName, std::string extension) {
    unsigned long size = fileName.size();
    unsigned long extensionSize = extension.size();
    for (unsigned long i = 1; i <= extensionSize; i++) {
        if (fileName[size - i] != extension[extensionSize - i]) {
            return false;
        }
    }
    return true;
}

bool ImageOps::load(const std::string &fileName, Image &image) {
    if (fileName.empty()) {
        std::cout << "Image: File name cannot be empty." << std::endl;
        return false;
    }

    if (!std::filesystem::exists(fileName)) {
        std::cout << "Image: File '" << fileName << "' does not exist" << std::endl;
        return false;
    }

    image.fileName = fileName;
    if (hasExtension(image.fileName, "png")) {
        return loadPng(image);
    }
    if (hasExtension(image.fileName, "jpeg") || hasExtension(image.fileName, "jpg")) {
        return loadJpg(image);
    }
    if (hasExtension(image.fileName, "tga")) {
        return loadTga(image);
    }
    if (hasExtension(image.fileName, "bmp")) {
        return loadBmp(image);
    }

    std::cerr << "Image: Image file type is not supported (" << fileName << ")" << std::endl;
    return false;
}

bool ImageOps::save(Image &image) {
    if (hasExtension(image.fileName, "png")) {
        return writePng(image);
    }
    if (hasExtension(image.fileName, "jpeg") || hasExtension(image.fileName, "jpg")) {
        return writeJpg(image);
    }
    if (hasExtension(image.fileName, "bmp")) {
        return writeBmp(image);
    }

    std::cerr << "Image file type is not supported (" << image.fileName << ")" << std::endl;
    return false;
}

void ImageOps::createCheckerBoard(Image &image) {
    image.width = 128;
    image.height = 128;
    image.channels = 4;
    image.pixels = std::vector<uint8_t>(image.width * image.height * image.channels);
    for (unsigned long i = 0; i < image.pixels.size() / image.channels; i++) {
        const float fullBrightness = 255.0F;
        float r = fullBrightness;
        float g = fullBrightness;
        float b = fullBrightness;
        unsigned int row = i / image.width;
        if ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1)) {
            r = 0.0F;
            g = 0.0F;
            b = 0.0F;
        }
        unsigned int idx = i * image.channels;
        image.pixels[idx] = static_cast<char>(r);
        image.pixels[idx + 1] = static_cast<char>(g);
        image.pixels[idx + 2] = static_cast<char>(b);
    }
}
