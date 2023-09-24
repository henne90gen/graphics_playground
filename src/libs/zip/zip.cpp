#include "zip.h"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace zip {

#pragma pack(push, 1)
struct LocalFileHeader {
    // local file header signature     4 bytes  (0x04034b50) 67324752
    // version needed to extract       2 bytes
    // general purpose bit flag        2 bytes
    // compression method              2 bytes
    // last mod file time              2 bytes
    // last mod file date              2 bytes
    // crc-32                          4 bytes
    // compressed size                 4 bytes
    // uncompressed size               4 bytes
    // file name length                2 bytes
    // extra field length              2 bytes
    // file name (variable size)
    // extra field (variable size)

    uint32_t local_file_header_signature = 0;
    uint16_t version_needed_to_extract = 0;
    uint16_t general_purpose_bit_flag = 0;
    uint16_t compression_method = 0;
    uint16_t last_mod_file_time = 0;
    uint16_t last_mod_file_data = 0;
    uint32_t crc32 = 0;
    uint32_t compressed_size = 0;
    uint32_t uncompressed_size = 0;
    uint16_t file_name_length = 0;
    uint16_t extra_field_length = 0;
    char *file_name = nullptr;
    char *extra_field = nullptr;
};
#pragma pack(pop)

std::optional<LocalFileHeader> readLocalFileHeader(std::ifstream &fs) {
    LocalFileHeader fileHeader = {};
    auto sizeOfFileHeaderWithoutVariableLengthFields =
          sizeof(fileHeader) - (sizeof(fileHeader.file_name) + sizeof(fileHeader.extra_field));

    fs.read((char *)&fileHeader, sizeOfFileHeaderWithoutVariableLengthFields);
    if (!fs.good()) {
        std::cerr << "Failed to read local file header" << std::endl;
        return {};
    }

    if (fileHeader.local_file_header_signature != 67324752) {
        std::cerr << "Signature of local file header is not 0x04034b50" << std::endl;
        return {};
    }

    fileHeader.file_name = (char *)malloc(fileHeader.file_name_length);
    fs.read(fileHeader.file_name, fileHeader.file_name_length);
    if (!fs.good()) {
        std::cerr << "Failed to read file name in local file header" << std::endl;
        return {};
    }

    fileHeader.extra_field = (char *)malloc(fileHeader.extra_field_length);
    fs.read(fileHeader.extra_field, fileHeader.extra_field_length);
    if (!fs.good()) {
        std::cerr << "Failed to read extra field in local file header" << std::endl;
        return {};
    }

    fs.seekg(fileHeader.compressed_size, std::ios_base::cur);

    if (fileHeader.general_purpose_bit_flag & 0b100) {
        fs.seekg(16, std::ios_base::cur);
    }

    return fileHeader;
}

std::optional<Container> open(const std::string &filepath) {
    Container result = {};
    result.filepath = filepath;

    auto fs = std::ifstream(result.filepath);
    if (!fs.is_open()) {
        return {};
    }
    for (int i = 0; i < 2; i++) {
        auto fileHeader = readLocalFileHeader(fs);
        int j = 0;
    }

    return result;
}

std::vector<std::string> Container::files() { return {}; }

} // namespace zip
