#include "FileUtils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

#ifndef WIN32

#include <unistd.h>

#endif

#ifdef WIN32
#include <sys/stat.h>
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

int64_t getLastModifiedTimeNano(const std::string &filePath) {
#if defined(_WIN32)
    {
        std::filesystem::path filePath_ = filePath;
        struct _stat64 fileInfo = {};
        if (_wstati64(filePath_.wstring().c_str(), &fileInfo) != 0) {
            throw std::runtime_error("Failed to get last write time.");
        }
        return std::chrono::time_point_cast<std::chrono::nanoseconds>(
                     std::chrono::system_clock::from_time_t(fileInfo.st_mtime))
              .time_since_epoch()
              .count();
    }
#else
    {
        auto fsTime = std::filesystem::last_write_time(filePath);
        return fsTime.time_since_epoch().count();
    }
#endif
}
