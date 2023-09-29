#include "zip.h"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace zip {

enum class CompressionMethod {
    NO_COMPRESSION = 0,       // The file is stored (no compression)
    SHRUNK = 1,               // The file is Shrunk
    COMPRESSION_FACTOR_1 = 2, // The file is Reduced with compression factor 1
    COMPRESSION_FACTOR_2 = 3, // The file is Reduced with compression factor 2
    COMPRESSION_FACTOR_3 = 4, // The file is Reduced with compression factor 3
    COMPRESSION_FACTOR_4 = 5, // The file is Reduced with compression factor 4
    IMPLODED = 6,             // The file is Imploded
    TOKENIZED = 7,            // Reserved for Tokenizing compression algorithm
    DEFLATED = 8,             // The file is Deflated
    DEFLATE64 = 9,            // Enhanced Deflating using Deflate64(tm)
    PKWARE_IMPLODED = 10,     // PKWARE Data Compression Library Imploding
    RESERVED = 11,            // Reserved by PKWARE
    BZIP2 = 12,               // File is compressed using BZIP2 algorithm
};

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

    uint64_t get_struct_size() { return sizeof(*this) - (sizeof(this->file_name) + sizeof(this->extra_field)); }
    CompressionMethod get_compression_method() const { return (CompressionMethod)compression_method; }
};

struct DataDescriptor {
    uint32_t data_descriptor_signature = 0;
    uint32_t crc32 = 0;
    uint32_t compressed_size = 0;
    uint32_t uncompressed_size = 0;
};

struct CentralFileHeader {
    // central file header signature   4 bytes  (0x02014b50)
    // version made by                 2 bytes
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
    // file comment length             2 bytes
    // disk number start               2 bytes
    // internal file attributes        2 bytes
    // external file attributes        4 bytes
    // relative offset of local header 4 bytes
    // file name (variable size)
    // extra field (variable size)
    // file comment (variable size)
    uint32_t central_file_header_signature = 0;
    uint16_t version_made_by = 0;
    uint16_t version_needed_to_extract = 0;
    GeneralPurposeBitFlag general_purpose_bit_flag = {};
    uint16_t compression_method = 0;
    uint16_t last_mod_file_time = 0;
    uint16_t last_mod_file_date = 0;
    uint32_t crc_32 = 0;
    uint32_t compressed_size = 0;
    uint32_t uncompressed_size = 0;
    uint16_t file_name_length = 0;
    uint16_t extra_field_length = 0;
    uint16_t file_comment_length = 0;
    uint16_t disk_number_start = 0;
    uint16_t internal_file_attributes = 0;
    uint32_t external_file_attributes = 0;
    uint32_t relative_offset_of_local_header = 0;

    char *file_name = nullptr;
    char *extra_field = nullptr;
    char *file_comment = nullptr;
        uint64_t get_struct_size() { return sizeof(*this) - (sizeof(this->file_name) + sizeof(this->extra_field)); }
};

struct EndOfCentralDirectoryRecord {
    // end of central dir signature    4 bytes  (0x06054b50)
    // number of this disk             2 bytes
    // number of the disk with the
    // start of the central directory  2 bytes
    // total number of entries in the
    // central directory on this disk  2 bytes
    // total number of entries in
    // the central directory           2 bytes
    // size of the central directory   4 bytes
    // offset of start of central
    // directory with respect to
    // the starting disk number        4 bytes
    // .ZIP file comment length        2 bytes
    // .ZIP file comment       (variable size)

    uint32_t end_of_central_directory_signature = 0;
    uint16_t number_of_this_disk = 0;
    uint16_t number_of_the_disk_with_the_start_if_the_central_directory = 0;
    uint16_t total_number_of_entries_in_the_central_directory_on_this_disk = 0;
    uint16_t total_number_of_entries_in_the_central_directory = 0;
    uint32_t size_of_the_central_directory = 0;
    uint32_t offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number = 0;
    uint16_t zip_file_comment_length = 0;
    char *zip_file_comment = 0;

    uint64_t get_struct_size() { return sizeof(*this) - sizeof(zip_file_comment); }
};
#pragma pack(pop)

struct File {
    LocalFileHeader local_file_header = {};
    char *file_data = nullptr;
    DataDescriptor data_descriptor = {};
};

std::optional<LocalFileHeader> readLocalFileHeader(std::ifstream &fs) {
    LocalFileHeader fileHeader = {};
    auto sizeOfFileHeaderWithoutVariableLengthFields = fileHeader.get_struct_size();

    fs.read((char *)&fileHeader, sizeOfFileHeaderWithoutVariableLengthFields);
    if (!fs.good()) {
        std::cerr << "Failed to read local file header" << std::endl;
        return {};
    }

    if (fileHeader.local_file_header_signature != 0x04034b50) {
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

    return fileHeader;
}

std::optional<EndOfCentralDirectoryRecord> readEndOfCentralDirectoryRecord(std::ifstream &fs) {
    EndOfCentralDirectoryRecord result = {};
    auto s = result.get_struct_size();

    // seek backwards from the end to find the signature
    fs.seekg(-s, std::ios_base::end);

    while (true) {
        uint32_t signature = 0;
        fs.read((char *)&signature, sizeof(signature));
        if (!fs.good()) {
            std::cerr << "Failed to read end of central directory signature" << std::endl;
            return {};
        }

        if (signature == 0x06054b50) {
            break;
        }

        fs.seekg(-(sizeof(signature) + 1), std::ios_base::cur);
    }

    fs.seekg(-sizeof(result.end_of_central_directory_signature), std::ios_base::cur);
    fs.read((char *)&result, s);
    if (!fs.good()) {
        std::cerr << "Failed to read end of central directory record" << std::endl;
        return {};
    }

    return result;
}

std::optional<CentralDirectory> readCentralDirectory() { return {}; }

std::optional<Container> open(const std::string &filepath) {
    Container result = {};
    result.filepath = filepath;

    auto fs = std::ifstream(result.filepath);
    if (!fs.is_open()) {
        return {};
    }

    auto endOfCentralDirectoryRecordOpt = readEndOfCentralDirectoryRecord(fs);
    if (!endOfCentralDirectoryRecordOpt.has_value()) {
        return {};
    }

    auto centralDirectoryOpt = readCentralDirectory(fs);
    if (!centralDirectoryOpt.has_value()) {
        return {};
    }

    return result;
}

} // namespace zip
