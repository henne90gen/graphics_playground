#define PNG_DEBUG 3
#include <png.h>

bool loadPng(Image &image) {
    FILE *fp = fopen(image.fileName.c_str(), "rb");
    if (fp == nullptr) {
        std::cout << "File '" << image.fileName << "' could not be opened for reading" << std::endl;
        return false;
    }

    char header[8]; // 8 is the maximum size that can be checked
    fread(header, 1, 8, fp);
    if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8) != 0) {
        std::cout << "File '" << image.fileName << "' is not recognized as a PNG file" << std::endl;
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (png_ptr == nullptr) {
        std::cout << "png_create_read_struct failed" << std::endl;
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        std::cout << "png_create_info_struct failed" << std::endl;
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "Error during init_io" << std::endl;
        return false;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    image.width = png_get_image_width(png_ptr, info_ptr);
    image.height = png_get_image_height(png_ptr, info_ptr);
    image.bitDepth = png_get_bit_depth(png_ptr, info_ptr);
    int colorType = png_get_color_type(png_ptr, info_ptr);
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
        return false;
    }
    image.pixels = std::vector<uint8_t>(image.width * image.height * image.channels);

    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "[read_png_file] Error during read_image" << std::endl;
        return false;
    }

    auto row_pointers = static_cast<png_bytep *>(malloc(sizeof(png_bytep) * image.height));
    for (unsigned int y = 0; y < image.height; y++) {
        row_pointers[y] = static_cast<png_byte *>(malloc(png_get_rowbytes(png_ptr, info_ptr)));
    }

    png_read_image(png_ptr, row_pointers);

    if (image.bitDepth == 8) {
        for (int y = 0; y < image.height; y++) {
            png_bytep row = row_pointers[y];
            for (unsigned int x = 0; x < image.width; x++) {
                png_bytep px = &(row[x * image.channels]);
                image.pixels[y * image.width * image.channels + x * image.channels] = px[0];
                if (image.channels > 1) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 1] = px[1];
                }
                if (image.channels > 2) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 2] = px[2];
                }
                if (image.channels > 3) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 3] = px[3];
                }
            }
        }
    } else if (image.bitDepth == 16) {
// TODO somehow we don't need to scale the values from uint16 (0-65535) to uint8 (0-255)
// #define SCALE_TO_BYTE(pix) static_cast<uint8_t>((static_cast<float>(pix - min) / (max - min)) * 255.0F)
// #define SCALE_TO_BYTE(pix) static_cast<unsigned char>((static_cast<float>(pix) / 4095.0F) * 255.0F)
// #define SCALE_TO_BYTE(pix) static_cast<unsigned char>((static_cast<float>(pix) / 1023.0F) * 255.0F)
// #define SCALE_TO_BYTE(pix) static_cast<unsigned char>(static_cast<float>(pix))
#define SCALE_TO_BYTE(pix) pix
        for (int y = 0; y < image.height; y++) {
            png_uint_16p row = reinterpret_cast<png_uint_16p>(row_pointers[y]);
            for (int x = 0; x < image.width; x++) {
                png_uint_16p px = &(row[x * image.channels]);
                image.pixels[y * image.width * image.channels + x * image.channels] = SCALE_TO_BYTE(px[0]);
                if (image.channels > 1) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 1] = SCALE_TO_BYTE(px[1]);
                }
                if (image.channels > 2) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 2] = SCALE_TO_BYTE(px[2]);
                }
                if (image.channels > 3) {
                    image.pixels[y * image.width * image.channels + x * image.channels + 3] = SCALE_TO_BYTE(px[3]);
                }
            }
        }
    }

    fclose(fp);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    png_ptr = nullptr;
    info_ptr = nullptr;
    return true;
}

bool writePng(Image &image) {
    // TODO do cleanup when we encounter an error
    FILE *fp = fopen(image.fileName.c_str(), "wb");
    if (fp == nullptr) {
        return false;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        return false;
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
        return false;
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
    return true;
}
