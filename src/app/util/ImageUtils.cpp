#include "ImageUtils.h"

#include <iostream>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3

#include <png.h>
#include <jpeglib.h>

int loadPng(const std::string &fileName, Image &image) {
    // TODO check error messages for string formatting patterns
    FILE *fp = fopen(fileName.c_str(), "rb");
    if (!fp) {
        std::cout << "File '" << fileName << "' could not be opened for reading" << std::endl;
        return 1;
    }

    char header[8];    // 8 is the maximum size that can be checked
    fread(header, 1, 8, fp);
    if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8)) {
        std::cout << "File '" << fileName << "' is not recognized as a PNG file" << std::endl;
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
    int colorType = png_get_color_type(png_ptr, info_ptr);
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

    if (colorType == 0) {
        image.channels = 1;
    } else if (colorType == PNG_COLOR_TYPE_RGB) {
        image.channels = 3;
    } else if (colorType == 4) {
        image.channels = 2;
    } else if (colorType == PNG_COLOR_TYPE_RGBA) {
        image.channels = 4;
    } else {
        std::cout << "Color type " << colorType << " not supported." << std::endl;
        return 1;
    }

    for (unsigned int y = 0; y < image.height; y++) {
        png_bytep row = row_pointers[y];
        for (unsigned int x = 0; x < image.width; x++) {
            png_bytep px = &(row[x * image.channels]);
            image.pixels.push_back(px[0]);
            if (image.channels > 1) {
                image.pixels.push_back(px[1]);
            }
            if (image.channels > 2) {
                image.pixels.push_back(px[2]);
            }
            if (image.channels > 3) {
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

int loadJpg(const std::string &fileName, Image &image) {
    FILE *infile = fopen(fileName.c_str(), "rb");
    if (infile == nullptr) {
        std::cout << "File '" << fileName << "' could not be opened for reading" << std::endl;
        return 1;
    }

    jpeg_decompress_struct cinfo = {};
    jpeg_error_mgr err = {};

    cinfo.err = jpeg_std_error(&err);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    image.width = cinfo.output_width;
    image.height = cinfo.output_height;
    image.channels = cinfo.actual_number_of_colors;

    int row_stride = image.width * cinfo.output_components; /* physical row width in output buffer */
    JSAMPARRAY pJpegBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride,
                                                        1); /* Output row buffer */

    image.channels = 4;
    char r, g, b;
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
        for (unsigned int x = 0; x < image.width; x++) {
            r = pJpegBuffer[0][cinfo.output_components * x];
            if (cinfo.output_components > 2) {
                g = pJpegBuffer[0][cinfo.output_components * x + 1];
                b = pJpegBuffer[0][cinfo.output_components * x + 2];
            } else {
                g = r;
                b = r;
            }
            image.pixels.push_back(r);
            image.pixels.push_back(g);
            image.pixels.push_back(b);
            image.pixels.push_back(255);
        }
    }
    fclose(infile);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 0;
}

int loadImage(const std::string &fileName, Image &image) {
    unsigned long size = fileName.size();
    if (fileName[size - 1] == 'g' && fileName[size - 2] == 'n' && fileName[size - 3] == 'p') {
        return loadPng(fileName, image);
    } else if (fileName[size - 1] == 'g' && fileName[size - 2] == 'p' && fileName[size - 3] == 'j') {
        return loadJpg(fileName, image);
    }
    return 0;
}
