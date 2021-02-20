#include "ImageOps.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

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
        return loadPng(image) == 0;
    }
    if (hasExtension(image.fileName, "jpeg") || hasExtension(image.fileName, "jpg")) {
        return loadJpg(image) == 0;
    }
    if (hasExtension(image.fileName, "tga")) {
        return loadTga(image) == 0;
    }

    std::cerr << "Image: Image file type is not supported (" << fileName << ")" << std::endl;
    return false;
}

void writePng(Image &image) {
    FILE *fp = fopen(image.fileName.c_str(), "wb");
    if (fp == nullptr) {
        abort();
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        abort();
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        abort();
    }

    if (setjmp(png_jmpbuf(png))) {
        abort();
    }

    png_init_io(png, fp);

    png_byte colorType = 0;
    if (image.channels == 1) {
        colorType = 0;
    } else if (image.channels == 2) {
        colorType = 4;
    } else if (image.channels == 3) {
        colorType = PNG_COLOR_TYPE_RGB;
    } else if (image.channels == 4) {
        colorType = PNG_COLOR_TYPE_RGBA;
    } else {
        std::cout << "Number of channels not supported (" << image.channels << ")" << std::endl;
        return;
    }

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(                       //
          png,                          //
          info,                         //
          image.width, image.height,    //
          image.bitDepth,               //
          colorType,                    //
          PNG_INTERLACE_NONE,           //
          PNG_COMPRESSION_TYPE_DEFAULT, //
          PNG_FILTER_TYPE_DEFAULT       //
    );
    png_write_info(png, info);

    for (long y = image.height; y >= 0; y--) {
        auto pixels = static_cast<png_bytep>(image.pixels.data());
        pixels += y * image.width * image.channels;
        png_write_row(png, pixels);
    }

    png_write_end(png, nullptr);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

void writeJpg(const Image & /*image*/) { std::cerr << "Writing JPEG files is not supported yet" << std::endl; }

void ImageOps::save(Image &image) {
    if (hasExtension(image.fileName, "png")) {
        writePng(image);
        return;
    }
    if (hasExtension(image.fileName, "jpeg") || hasExtension(image.fileName, "jpg")) {
        writeJpg(image);
        return;
    }

    std::cerr << "Image file type is not supported (" << image.fileName << ")" << std::endl;
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
