#include "zip.h"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace zip {

#pragma pack(push, 1)
struct GeneralPurposeBitFlag {
    bool is_encrypted : 1;
    bool bit_1 : 1;
    bool bit_2 : 1;
    bool is_data_descriptor_present : 1;
    bool enhanced_deflating : 1;
    bool is_compressed_patched_data : 1;
    bool strong_encryption : 1;
    bool bit_7 : 1;
    bool bit_8 : 1;
    bool bit_9 : 1;
    bool bit_10 : 1;
    bool bit_11 : 1;
    bool enhanced_compression : 1;
    bool bit_13 : 1;
    bool bit_14 : 1;
    bool bit_15 : 1;
};

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

    GeneralPurposeBitFlag general_purpose_bit_flag = {};

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

struct DataDescriptor {
    uint32_t data_descriptor_signature = 0;
    uint32_t crc32 = 0;
    uint32_t compressed_size = 0;
    uint32_t uncompressed_size = 0;
};
#pragma pack(pop)

struct File {
    LocalFileHeader local_file_header = {};
    char *file_data = nullptr;
    DataDescriptor data_descriptor = {};
};

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

    // TODO find out when to move forward by uncompressed_size and when to move forward by compressed_size
    fs.seekg(fileHeader.uncompressed_size, std::ios_base::cur);

    if (fileHeader.general_purpose_bit_flag.is_data_descriptor_present) {
        DataDescriptor dataDescriptor = {};
        auto dataDescriptorSize = sizeof(dataDescriptor);
        fs.read((char *)&dataDescriptor, dataDescriptorSize);
        if (!fs.good()) {
            std::cerr << "Failed to read data descriptor" << std::endl;
            return {};
        }

        if (dataDescriptor.data_descriptor_signature != 0x08074b50) {
            std::cerr << "Signature of data descriptor is not 0x08074b50" << std::endl;
            return {};
        }
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
        auto fileHeaderOpt = readLocalFileHeader(fs);
        if (!fileHeaderOpt.has_value()) {
            return {};
        }

        auto fileHeader = fileHeaderOpt.value();
        result.files.push_back(std::string(fileHeader.file_name, fileHeader.file_name_length));
    }

    return result;
}

} // namespace zip
