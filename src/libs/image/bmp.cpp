#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct BmpFileHeader {
    uint16_t identifier = 0x4D42;
    uint32_t fileSizeInBytes;
    uint16_t reserved0 = 0;
    uint16_t reserved1 = 0;
    uint32_t pixelOffset = 54;
};

struct BmpInfoHeader {
    uint32_t size = 40;
    int32_t width;
    int32_t height;
    uint16_t numPlanes = 1;
    uint16_t bitsPerPixel = 24;
    uint32_t compression = 0;
    uint32_t imageSize = 0;
    uint32_t xPixelsPerM;
    uint32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t importantColors = 0;
};

struct BmpFile {
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    uint8_t *pixels = nullptr;
};
#pragma pack(pop)

std::ostream &operator<<(std::ostream &os, BmpFile &f) {
    size_t fileHeaderSize = sizeof(f.fileHeader);
    size_t infoHeaderSize = sizeof(f.infoHeader);
    os.write(reinterpret_cast<const char *>(&f.fileHeader), fileHeaderSize);
    os.write(reinterpret_cast<const char *>(&f.infoHeader), infoHeaderSize);
    uint32_t pixelSize = f.fileHeader.fileSizeInBytes - f.fileHeader.pixelOffset;
    os.write(reinterpret_cast<const char *>(f.pixels), pixelSize);
    return os;
}

bool writeBmp(const Image &image) {
    BmpFileHeader fileHeader = {};
    fileHeader.fileSizeInBytes = fileHeader.pixelOffset + image.width * image.height * 3;

    BmpInfoHeader infoHeader = {};
    infoHeader.width = image.width;
    infoHeader.height = image.height;
    infoHeader.xPixelsPerM = 0;
    infoHeader.yPixelsPerM = 0;
    infoHeader.colorsUsed = 0;

    int rowSize = static_cast<int>(std::ceil((infoHeader.bitsPerPixel * image.width) / 32)) * 4;
    uint8_t *pBuf = nullptr;
    if (rowSize == image.width * 3) {
        pBuf = const_cast<uint8_t *>(image.pixels.data());
    } else {
        // we have to create a copy of the pixels to account for additional padding
        // that is necessary at the end of each pixel row

        int pixelsSize = rowSize * image.height;
        pBuf = reinterpret_cast<uint8_t *>(std::malloc(pixelsSize));
        std::memset(pBuf, 0, pixelsSize);

        // TODO this could be optimized with omp and memcpy (copying each row
        //  instead of each pixel)
        for (int row = 0; row < image.height; row++) {
            for (int col = 0; col < image.width; col++) {
                int pBufIndex = row * rowSize + col * 3;
                int pixelsIndex = (row * image.width + col) * 3;
                pBuf[pBufIndex + 0] = image.pixels[pixelsIndex + 0];
                pBuf[pBufIndex + 1] = image.pixels[pixelsIndex + 1];
                pBuf[pBufIndex + 2] = image.pixels[pixelsIndex + 2];
            }
        }
    }

    BmpFile bmpFile = {
          fileHeader,
          infoHeader,
          pBuf,
    };

    std::ofstream file(image.fileName, std::ios::binary);
    file << bmpFile;
    file.flush();
    file.close();
    return true;
}

std::istream &operator>>(std::istream &is, BmpFile &f) {
    is.read(reinterpret_cast<char *>(&f.fileHeader), sizeof(f.fileHeader));
    is.read(reinterpret_cast<char *>(&f.infoHeader), sizeof(f.infoHeader));
    int diff = f.infoHeader.size - sizeof(BmpInfoHeader);
    if (diff > 0) {
        is.seekg(diff, std::ios_base::cur);
    }

    uint32_t pixelSize = f.fileHeader.fileSizeInBytes - f.fileHeader.pixelOffset;
    f.pixels = reinterpret_cast<uint8_t *>(std::malloc(pixelSize));
    is.read(reinterpret_cast<char *>(f.pixels), pixelSize);
    return is;
}

bool loadBmp(Image &image) {
    BmpFile bmpFile = {};

    std::ifstream file(image.fileName, std::ios::binary);
    file >> bmpFile;

    image.width = bmpFile.infoHeader.width;
    image.height = bmpFile.infoHeader.height;

    int pixelSize = image.width * image.height * 3;
    image.pixels = std::vector<uint8_t>(bmpFile.pixels, bmpFile.pixels + pixelSize);

    return true;
}
