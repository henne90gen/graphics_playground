#include "Image.h"

#define PNG_DEBUG 3

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <jpeglib.h>
#include <png.h>
#include <utility>

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

int loadPng(Image &image) {
    FILE *fp = fopen(image.fileName.c_str(), "rbe");
    if (fp == nullptr) {
        std::cout << "File '" << image.fileName << "' could not be opened for reading" << std::endl;
        return 1;
    }

    char header[8]; // 8 is the maximum size that can be checked
    fread(header, 1, 8, fp);
    if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8) != 0) {
        std::cout << "File '" << image.fileName << "' is not recognized as a PNG file" << std::endl;
        return 1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (png_ptr == nullptr) {
        std::cout << "png_create_read_struct failed" << std::endl;
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        std::cout << "png_create_info_struct failed" << std::endl;
        return 1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "Error during init_io" << std::endl;
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

    auto row_pointers = static_cast<png_bytep *>(malloc(sizeof(png_bytep) * image.height));
    for (unsigned int y = 0; y < image.height; y++) {
        row_pointers[y] = static_cast<png_byte *>(malloc(png_get_rowbytes(png_ptr, info_ptr)));
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

int loadJpg(Image &image) {
    FILE *infile = fopen(image.fileName.c_str(), "rbe");
    if (infile == nullptr) {
        std::cout << "File '" << image.fileName << "' could not be opened for reading" << std::endl;
        return 1;
    }

    jpeg_decompress_struct cinfo = {};
    jpeg_error_mgr err = {};

    cinfo.err = jpeg_std_error(&err);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    image.width = cinfo.output_width;
    image.height = cinfo.output_height;
    image.channels = cinfo.actual_number_of_colors;

    int row_stride = image.width * cinfo.output_components; /* physical row width in output buffer */
    JSAMPARRAY pJpegBuffer = (*cinfo.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo), JPOOL_IMAGE, row_stride,
                                                        1); /* Output row buffer */

    image.channels = 4;
    char r = 0;
    char g = 0;
    char b = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        (void)jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
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
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
}

bool ImageOps::load(const std::string &fileName, Image &image) {
    if (fileName.empty()) {
        return false;
    }

    image.fileName = fileName;
    if (hasExtension(image.fileName, "png")) {
        return loadPng(image) == 0;
    }
    if (hasExtension(image.fileName, "jpeg") || hasExtension(image.fileName, "jpg")) {
        return loadJpg(image) == 0;
    }

    std::cerr << "Image file type is not supported (" << fileName << ")" << std::endl;
    return false;
}

void writePng(Image &image) {
    FILE *fp = fopen(image.fileName.c_str(), "wbe");
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
