#include "zip.h"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace zip {

#define ENSURE_FS_IS_GOOD(fs, msg)                                                                                     \
    if (!(fs).good()) {                                                                                                \
        std::cerr << msg << std::endl;                                                                                 \
        return {};                                                                                                     \
    }

std::optional<LocalFileHeader> readLocalFileHeader(std::ifstream &fs) {
    LocalFileHeader fileHeader = {};
    auto fileHeaderSize = fileHeader.get_struct_size();

    fs.read((char *)&fileHeader, fileHeaderSize);
    ENSURE_FS_IS_GOOD(fs, "Failed to read local file header");

    if (fileHeader.local_file_header_signature != 0x04034b50) {
        std::cerr << "Signature of local file header is not 0x04034b50" << std::endl;
        return {};
    }

    fileHeader.file_name = (char *)malloc(fileHeader.file_name_length);
    fs.read(fileHeader.file_name, fileHeader.file_name_length);
    ENSURE_FS_IS_GOOD(fs, "Failed to read file name in local file header");

    fileHeader.extra_field = (char *)malloc(fileHeader.extra_field_length);
    fs.read(fileHeader.extra_field, fileHeader.extra_field_length);
    ENSURE_FS_IS_GOOD(fs, "Failed to read extra field in local file header");

    return fileHeader;
}

std::optional<CentralDirectorySignature> readCentralDirectorySignature(std::ifstream &fs) {
    CentralDirectorySignature result = {};
    fs.read((char *)&result, result.get_struct_size());
    ENSURE_FS_IS_GOOD(fs, "Failed to read central directory digital signature");

    if (result.header_signature != 0x05054b50) {
        // the signature is not always present
        return CentralDirectorySignature();
    }

    result.signature_data = (char *)malloc(result.size_of_data);
    fs.read(result.signature_data, result.size_of_data);
    ENSURE_FS_IS_GOOD(fs, "Failed to read signature data in central directory signature");

    return result;
}

std::optional<CentralDirectory> readCentralDirectory(std::ifstream &fs,
                                                     const EndOfCentralDirectoryRecord &endOfCentralDirectoryRecord) {
    if (endOfCentralDirectoryRecord.number_of_the_disk_with_the_start_if_the_central_directory !=
        endOfCentralDirectoryRecord.number_of_this_disk) {
        std::cerr << "Multiple disks are not supported" << std::endl;
        return {};
    }

    fs.seekg(endOfCentralDirectoryRecord.offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number,
             std::ios_base::beg);

    CentralDirectory centralDirectory = {};
    for (int i = 0; i < endOfCentralDirectoryRecord.total_number_of_entries_in_the_central_directory_on_this_disk;
         i++) {
        auto &fileHeader = centralDirectory.file_headers.emplace_back();
        fs.read((char *)&fileHeader, fileHeader.get_struct_size());
        ENSURE_FS_IS_GOOD(fs, "Failed to read central file header " << i);

        if (fileHeader.central_file_header_signature != 0x02014b50) {
            std::cerr << "Signature of central file header " << i << " is not 0x02014b50" << std::endl;
            return {};
        }

        fileHeader.file_name = (char *)malloc(fileHeader.file_name_length);
        fs.read(fileHeader.file_name, fileHeader.file_name_length);
        ENSURE_FS_IS_GOOD(fs, "Failed to read file name in central file header");

        fileHeader.extra_field = (char *)malloc(fileHeader.extra_field_length);
        fs.read(fileHeader.extra_field, fileHeader.extra_field_length);
        ENSURE_FS_IS_GOOD(fs, "Failed to read extra field in central file header");

        fileHeader.file_comment = (char *)malloc(fileHeader.file_comment_length);
        fs.read(fileHeader.file_comment, fileHeader.file_comment_length);
        ENSURE_FS_IS_GOOD(fs, "Failed to read file comment in central file header");
    }

    auto digitalSignatureOpt = readCentralDirectorySignature(fs);
    if (!digitalSignatureOpt.has_value()) {
        std::cerr << "Failed to read digital signature in central directory" << std::endl;
        return {};
    }

    return centralDirectory;
}

std::optional<EndOfCentralDirectoryRecord> readEndOfCentralDirectoryRecord(std::ifstream &fs) {
    EndOfCentralDirectoryRecord result = {};
    auto endOfCentralDirectoryRecordSize = result.get_struct_size();

    // seek backwards from the end to find the signature
    fs.seekg(-1 * endOfCentralDirectoryRecordSize, std::ios_base::end);

    while (true) {
        uint32_t signature = 0;
        fs.read((char *)&signature, sizeof(signature));
        ENSURE_FS_IS_GOOD(fs, "Failed to read end of central directory signature");

        if (signature == 0x06054b50) {
            break;
        }

        fs.seekg(-1 * (sizeof(signature) + 1), std::ios_base::cur);
    }

    fs.seekg(-1 * sizeof(result.end_of_central_directory_signature), std::ios_base::cur);
    fs.read((char *)&result, endOfCentralDirectoryRecordSize);
    ENSURE_FS_IS_GOOD(fs, "Failed to read end of central directory record");

    return result;
}

std::optional<File> readFile(std::ifstream &fs, const CentralFileHeader &file_header) {
    fs.seekg(file_header.relative_offset_of_local_header, std::ios_base::beg);
    auto localFileHeaderOpt = readLocalFileHeader(fs);
    if (!localFileHeaderOpt.has_value()) {
        return {};
    }

    File result = {};
    result.local_file_header = localFileHeaderOpt.value();

    uint32_t file_size = 0;
    if (result.local_file_header.general_purpose_bit_flag.is_data_descriptor_present) {
        file_size = file_header.compressed_size;
    } else {
        file_size = result.local_file_header.compressed_size;
    }

    result.file_data = (char *)malloc(file_size);
    fs.read(result.file_data, file_size);
    ENSURE_FS_IS_GOOD(fs, "Failed to read file data");

    if (result.local_file_header.general_purpose_bit_flag.is_data_descriptor_present) {
        fs.read((char *)&result.data_descriptor, result.data_descriptor.get_struct_size());
        ENSURE_FS_IS_GOOD(fs, "Failed to read data descriptor");

        if (result.data_descriptor.data_descriptor_signature != 0x08074b50) {
            std::cerr << "Signature of data descriptor is not 0x08074b50" << std::endl;
            return {};
        }
    }

    return result;
}

std::optional<Container> open(const std::string &filepath) {
    auto fs = std::ifstream(filepath);
    if (!fs.is_open()) {
        return {};
    }

    auto endOfCentralDirectoryRecordOpt = readEndOfCentralDirectoryRecord(fs);
    if (!endOfCentralDirectoryRecordOpt.has_value()) {
        return {};
    }

    Container result = {};
    result.end_of_central_directory_record = endOfCentralDirectoryRecordOpt.value();

    auto centralDirectoryOpt = readCentralDirectory(fs, result.end_of_central_directory_record);
    if (!centralDirectoryOpt.has_value()) {
        return {};
    }
    result.central_directory = centralDirectoryOpt.value();

    for (const auto &file_header : result.central_directory.file_headers) {
        auto fileOpt = readFile(fs, file_header);
        if (!fileOpt.has_value()) {
            std::cerr << "Failed to load file " << file_header.get_file_name() << std::endl;
            return {};
        }

        result.files.push_back(fileOpt.value());
    }

    return result;
}

} // namespace zip
