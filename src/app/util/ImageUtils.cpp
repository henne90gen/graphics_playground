#include "ImageUtils.h"

#include <iostream>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3

#include <png.h>

int loadPng(const std::string &file_name, Image &image) {
    // TODO check error messages for string formatting patterns
    FILE *fp = fopen(file_name.c_str(), "rb");
    if (!fp) {
        std::cout << "[read_png_file] File %s could not be opened for reading" << file_name << std::endl;
        return 1;
    }

    char header[8];    // 8 is the maximum size that can be checked
    fread(header, 1, 8, fp);
    if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8)) {
        std::cout << "[read_png_file] File %s is not recognized as a PNG file" << file_name << std::endl;
        return 1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr)
        std::cout << "[read_png_file] png_create_read_struct failed" << std::endl;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cout << "[read_png_file] png_create_info_struct failed" << std::endl;
        return 1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[read_png_file] Error during init_io" << std::endl;
        return 1;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    image.width = png_get_image_width(png_ptr, info_ptr);
    image.height = png_get_image_height(png_ptr, info_ptr);
    image.colorType = png_get_color_type(png_ptr, info_ptr);
    image.bitDepth = png_get_bit_depth(png_ptr, info_ptr);

    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[read_png_file] Error during read_image" << std::endl;
        return 1;
    }

    auto row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image.height);
    for (unsigned int y = 0; y < image.height; y++) {
        row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, row_pointers);

    int channels;
    if (image.colorType == 0) {
        channels = 1;
    } else if (image.colorType == PNG_COLOR_TYPE_RGB) {
        channels = 3;
    } else if (image.colorType == 4) {
        channels = 2;
    } else if (image.colorType == PNG_COLOR_TYPE_RGBA) {
        channels = 4;
    } else {
        std::cout << "Color type " << image.colorType << " not supported." << std::endl;
        return 1;
    }

    for (unsigned int y = 0; y < image.height; y++) {
        png_bytep row = row_pointers[y];
        for (unsigned int x = 0; x < image.width; x++) {
            png_bytep px = &(row[x * channels]);
            image.pixels.push_back(px[0]);
            if (channels > 1) {
                image.pixels.push_back(px[1]);
            }
            if (channels > 2) {
                image.pixels.push_back(px[2]);
            }
            if (channels > 3) {
                image.pixels.push_back(px[3]);
            }
        }
    }

    fclose(fp);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    png_ptr = nullptr;
    info_ptr = nullptr;
    return 0;
}
