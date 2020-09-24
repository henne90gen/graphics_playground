#pragma once

#include <cstdint>
#include <string>

#pragma pack(push, 1)

struct WavHeader {
    uint32_t chunkId;
    uint32_t chunkSize;
    uint32_t format;
    uint32_t subChunkId;
    uint32_t subChunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct WavData {
    uint32_t subChunkId;
    uint32_t subChunkSize;
    union {
        uint8_t *data8 = nullptr;
        int16_t *data16;
        int32_t *data32;
    };
};

struct WavFile {
    WavHeader header;
    WavData data;
};

#pragma pack(pop)

bool loadWavFile(const std::string &fileName, WavFile &wav);
