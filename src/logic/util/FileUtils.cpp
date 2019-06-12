#include "FileUtils.h"

#include <filesystem>

void getFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &filePaths) {
    for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
        std::string fileName = entry.path().u8string();
        filePaths.push_back(fileName);
    }
}


std::vector<std::string> getFilesInDirectory(const std::string &directoryPath) {
    std::vector<std::string> result = {};
    getFilesInDirectory(directoryPath, result);
    return result;
}


