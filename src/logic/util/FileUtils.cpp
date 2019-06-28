#include "FileUtils.h"

#include <algorithm>
#include <filesystem>

#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32

#include <unistd.h>

#endif

#ifdef WIN32
#define stat _stat
#endif


void getFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &filePaths) {
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

long getLastModifiedTime(const std::string &filePath) {
    struct stat result = {};
    if (stat(filePath.c_str(), &result) == 0) {
        auto modTime = result.st_mtime;
        return modTime;
    }
    return -1L;
}


