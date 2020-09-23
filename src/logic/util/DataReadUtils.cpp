#include "DataReadUtils.h"

#include <array>

void bigToLittleEndian(int32_t &in) {
    std::array<unsigned char, 4> buffer = {};
    for (int i = 0; i < 4; i++) {
        buffer[3 - i] = (in >> (i * 8));
    }
    in = (int)buffer[0] | (int)buffer[1] << 8 | (int)buffer[2] << 16 | (int)buffer[3] << 24;
}

void bigToLittleEndian(uint32_t &in) {
    std::array<unsigned char, 4> buffer = {};
    for (int i = 0; i < 4; i++) {
        buffer[3 - i] = (in >> (i * 8));
    }
    in = (int)buffer[0] | (int)buffer[1] << 8 | (int)buffer[2] << 16 | (int)buffer[3] << 24;
}
