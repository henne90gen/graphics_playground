#include <jpeglib.h>

bool loadJpg(Image &image) {
    FILE *infile = fopen(image.fileName.c_str(), "rb");
    if (infile == nullptr) {
        std::cout << "File '" << image.fileName << "' could not be opened for reading" << std::endl;
        return false;
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

    return true;
}

bool writeJpg(const Image & /*image*/) {
    std::cerr << "Writing JPEG files is not supported yet" << std::endl;
    return false;
}
