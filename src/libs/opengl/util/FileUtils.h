#pragma once

#include <string>
#include <vector>

void getFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &filePaths);

std::vector<std::string> getFilesInDirectory(const std::string &directoryPath);

int64_t getLastModifiedTimeNano(const std::string &filePath);
