#include "WavLoader.h"

#include <fstream>
#include <iostream>

#include "util/DataReadUtils.h"

std::istream &operator>>(std::istream &is, WavHeader &header) {
    is.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (is.fail()) {
        std::cout << "Failed to read wav header" << std::endl;
        return is;
    }

    bigToLittleEndian(header.format);
    bigToLittleEndian(header.subChunkId);

    return is;
}

std::istream &operator>>(std::istream &is, WavData &data) {
    is.read(reinterpret_cast<char *>(&data), sizeof(data.subChunkId) + sizeof(data.subChunkSize));
    if (is.fail()) {
        std::cout << "Failed to read wav data header" << std::endl;
        return is;
    }

    bigToLittleEndian(data.subChunkId);

    data.data = static_cast<uint8_t *>(std::malloc(data.subChunkSize));
    is.read(reinterpret_cast<char *>(data.data), data.subChunkSize);
    if (is.fail()) {
        std::cout << "Failed to read wav data" << std::endl;
        return is;
    }

    return is;
}

std::istream &operator>>(std::istream &is, WavFile &wav) {
    is >> wav.header;
    is >> wav.data;
    return is;
}

bool loadWavFile(const std::string &fileName, WavFile &wav) {
    std::ifstream wavFile;
    wavFile.open(fileName, std::ios::in | std::ios::binary);
    if (!wavFile.is_open()) {
        std::cout << "Failed to open wav file: " << fileName << std::endl;
        return false;
    }

    wavFile >> wav;
    if (wavFile.fail()) {
        wavFile.close();
        return false;
    }

    wavFile.close();
    return true;
}
