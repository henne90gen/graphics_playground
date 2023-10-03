#pragma once

#include <functional>
#include <string>
#include <vector>

void getFilesInDirectory(const std::string &directoryPath, std::function<bool(const std::string &)> filterFunc,
                         std::vector<std::string> &filePaths);

std::vector<std::string> getFilesInDirectory(const std::string &directoryPath,
                                             std::function<bool(const std::string &)> filterFunc);

int64_t getLastModifiedTimeNano(const std::string &filePath);
