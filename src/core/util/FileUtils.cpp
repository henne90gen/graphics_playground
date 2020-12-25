#include "FileUtils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

#include <sys/stat.h>

#ifndef WIN32

#include <unistd.h>

#endif

#ifdef WIN32
#define stat _stat
#endif

void getFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &filePaths) {
    if (!std::filesystem::exists(directoryPath)) {
        std::cerr << "Directory '" << directoryPath << "' does not exist" << std::endl;
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
        std::string fileName = entry.path().u8string();
        filePaths.push_back(fileName);
    }

    std::sort(filePaths.begin(), filePaths.end());
}

std::vector<std::string> getFilesInDirectory(const std::string &directoryPath) {
    std::vector<std::string> result = {};
    getFilesInDirectory(directoryPath, result);
    return result;
}
