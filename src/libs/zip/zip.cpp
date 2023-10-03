#include "zip.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <zlib.h>

namespace zip {

#define ENSURE_FS_IS_GOOD(fs, msg)                                                                                     \
    if (!(fs).good()) {                                                                                                \
        std::cerr << msg << std::endl;                                                                                 \
        return {};                                                                                                     \
    }

template <typename T> std::optional<LocalFileHeader> readLocalFileHeader(T &fs) {
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

template <typename T> std::optional<CentralDirectorySignature> readCentralDirectorySignature(T &fs) {
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

template <typename T>
std::optional<CentralDirectory> readCentralDirectory(T &fs,
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

template <typename T> std::optional<EndOfCentralDirectoryRecord> readEndOfCentralDirectoryRecord(T &fs) {
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

template <typename T> std::optional<File> readFile(T &fs, const CentralFileHeader &central_file_header) {
    fs.seekg(central_file_header.relative_offset_of_local_header, std::ios_base::beg);
    auto localFileHeaderOpt = readLocalFileHeader(fs);
    if (!localFileHeaderOpt.has_value()) {
        return {};
    }

    File result = {};
    result.local_file_header = localFileHeaderOpt.value();

    uint32_t file_size = 0;
    if (result.local_file_header.general_purpose_bit_flag.is_data_descriptor_present) {
        file_size = central_file_header.compressed_size;
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

template <typename T> std::optional<Container> open(T &fs) {
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

    for (const auto &central_file_header : result.central_directory.file_headers) {
        auto fileOpt = readFile(fs, central_file_header);
        if (!fileOpt.has_value()) {
            std::cerr << "Failed to load file " << central_file_header.get_file_name() << std::endl;
            return {};
        }

        result.files.push_back(fileOpt.value());

        // fixing up local file header with information from the central file header
        result.files.back().local_file_header.compressed_size = central_file_header.compressed_size;
        result.files.back().local_file_header.uncompressed_size = central_file_header.uncompressed_size;
    }

    return result;
}

std::optional<Container> open_from_file(const std::string &filepath) {
    auto fs = std::ifstream(filepath, std::ios::in | std::ios::binary);
    if (!fs.is_open()) {
        std::cerr << "Failed to open zip file for reading: " << filepath << std::endl;
        return {};
    }

    return open(fs);
}

std::optional<Container> open_from_memory(char *data, uint64_t size) {
    auto s = std::string(data, size);
    auto ss = std::stringstream(s, std::ios::in | std::ios::binary);
    return open(ss);
}

std::optional<std::string_view> File::get_content() {
    auto compressionMethod = local_file_header.get_compression_method();
    if (compressionMethod == CompressionMethod::NO_COMPRESSION) {
        return std::string_view(file_data, local_file_header.uncompressed_size);
    }

    if (compressionMethod != CompressionMethod::DEFLATED) {
        std::cerr << "Compression methods other than DEFLATED are not supported" << std::endl;
        return {};
    }

    if (uncompressed_file_data != nullptr) {
        return std::string_view(uncompressed_file_data, local_file_header.uncompressed_size);
    }

    auto *input = file_data;
    size_t inputSize = local_file_header.compressed_size;

    auto *output = (char *)malloc(local_file_header.uncompressed_size);
    size_t outputSize = local_file_header.uncompressed_size;

    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = (uInt)inputSize;   // size of input
    infstream.next_in = (Bytef *)input;     // input char array
    infstream.avail_out = (uInt)outputSize; // size of output
    infstream.next_out = (Bytef *)output;   // output char array

    auto err = inflateInit2(&infstream, -MAX_WBITS);
    if (err < 0) {
        std::cerr << "Failed to inflate file data: " << infstream.msg << std::endl;
        return {};
    }

    err = inflate(&infstream, Z_NO_FLUSH);
    if (err < 0) {
        std::cerr << "Failed to inflate file data: " << infstream.msg << std::endl;
        return {};
    }

    if (infstream.avail_out != 0) {
        std::cerr << "Failed to inflate file data in a single call" << std::endl;
        return {};
    }

    err = inflateEnd(&infstream);
    if (err < 0) {
        std::cerr << "Failed to inflate file data: " << infstream.msg << std::endl;
        return {};
    }

    uncompressed_file_data = output;
    return std::string_view(uncompressed_file_data, local_file_header.uncompressed_size);
}

void Container::extract_to_directory(const std::string &directoryPath) {
    std::filesystem::create_directories(directoryPath);

    for (auto &file : files) {
        const auto destinationFilepath = directoryPath + "/" + std::string(file.get_file_name());
        auto os = std::ofstream(destinationFilepath, std::ios::out | std::ios::binary | std::ios::trunc);

        auto contentOpt = file.get_content();
        if (!contentOpt) {
            std::cerr << "Failed to decompress " << file.get_file_name() << std::endl;
            return;
        }

        auto content = contentOpt.value();
        os.write(content.data(), content.size());
    }
}

} // namespace zip
