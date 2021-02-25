#pragma pack(push, 1)
struct TgaColorMapSpec {
    uint16_t firstEntryIndex = 0;
    uint16_t colorMapEntryCount = 0;
    uint8_t colorMapEntrySize = 0;
};

struct TgaImageSpec {
    uint16_t originX = 0;
    uint16_t originY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t bitsPerPixel = 0;
    uint8_t imageDescriptor = 0;
};

struct TgaHeader {
    uint8_t imageIdLength = 0;
    uint8_t colorMapType = 0;
    uint8_t imageType = 0;
    TgaColorMapSpec colorMapSpec = {};
    TgaImageSpec imageSpec = {};
};
#pragma pack(pop)

bool loadTga(Image &image) {
    std::ifstream is(image.fileName);
    TgaHeader header = {};
    size_t sizeToRead = sizeof(header);
    is.read(reinterpret_cast<char *>(&header), sizeToRead);

    if (header.imageType != 2) {
        std::cout << "Image: Image type " << header.imageType << " is not supported." << std::endl;
        return false;
    }

    if (header.imageIdLength > 0) {
        is.seekg(header.imageIdLength, std::ios_base::cur);
    }

    uint32_t colorMapSize = header.colorMapSpec.colorMapEntryCount * header.colorMapSpec.colorMapEntrySize;
    if (colorMapSize > 0) {
        is.seekg(colorMapSize, std::ios_base::cur);
    }

    image.width = header.imageSpec.width;
    image.height = header.imageSpec.height;
    image.channels = header.imageSpec.bitsPerPixel / 8;
    image.pixels.resize(image.width * image.height * image.channels);

    uint32_t imageSize = header.imageSpec.width * header.imageSpec.height * header.imageSpec.bitsPerPixel / 8;
    // TODO this seems dangerous
    is.read(reinterpret_cast<char *>(image.pixels.data()), imageSize);

    //    if (is.fail()) {
    //        std::cout << "Image: Failed to read tga." << std::endl;
    //        return 1;
    //    }
    return true;
}
