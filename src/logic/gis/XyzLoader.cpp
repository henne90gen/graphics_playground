#include "XyzLoader.h"

#include <fstream>
#include <iostream>

#include "util/FileUtils.h"

#define UPDATE_BB(left, op, right)                                                                                     \
    if ((left)op(right)) {                                                                                             \
        (right) = (left);                                                                                              \
    }

bool loadXyzDir(const std::string &dirName, BoundingBox3 &bb, std::vector<glm::vec3> &result) {
    auto files = getFilesInDirectory(dirName);
    if (files.empty()) {
        return false;
    }

    bb = {{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
          {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()}};

    result.clear();
    result.reserve(files.size() * 10000);
    return loadXyzDir(files, [&bb, &result](const std::vector<glm::vec3> &temp) {
        for (unsigned int i = 0; i < temp.size(); i++) {
            UPDATE_BB(temp[i].x, <, bb.min.x)
            UPDATE_BB(temp[i].y, <, bb.min.y)
            UPDATE_BB(temp[i].z, <, bb.min.z)

            UPDATE_BB(temp[i].x, >, bb.max.x)
            UPDATE_BB(temp[i].y, >, bb.max.y)
            UPDATE_BB(temp[i].z, >, bb.max.z)

            result.push_back(temp[i]);
        }
    });
}

bool loadXyzDir(const std::string &dirName, const std::function<void(const std::vector<glm::vec3> &)> &takePointsFunc) {
    auto files = getFilesInDirectory(dirName);
    if (files.empty()) {
        return false;
    }

    return loadXyzDir(files, takePointsFunc);
}

bool loadXyzDir(const std::vector<std::string> &files,
                const std::function<void(const std::vector<glm::vec3> &)> &takePointsFunc) {
    unsigned long fileCount = files.size();
    if (fileCount > 100) {
        fileCount = 100;
    }

#pragma omp parallel for
    for (int i = 0; i < fileCount; i++) {
        const auto &fileName = files[i];
        if (fileName[fileName.size() - 4] != '.' || fileName[fileName.size() - 3] != 'x' ||
            fileName[fileName.size() - 2] != 'y' || fileName[fileName.size() - 1] != 'z') {
            continue;
        }

        std::ifstream file;
        file.open(fileName, std::ios::in | std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Could not open file: " << fileName << std::endl;
            continue;
        }

        size_t fileSize = file.tellg();
        file.seekg(0);
        char *buffer = reinterpret_cast<char *>(std::malloc(fileSize));
        file.read(buffer, fileSize);

        std::vector<glm::vec3> temp = {};
        char *workingBuffer = buffer;
        while (workingBuffer - buffer < fileSize - 3) {
            glm::vec3 vec;
            char *last = nullptr;
            vec.x = strtof(workingBuffer, &last);
            vec.z = strtof(last, &last);
            vec.y = strtof(last, &last);
            temp.push_back(vec);

            workingBuffer = last;
        }
        std::free(buffer);

#pragma omp critical
        { takePointsFunc(temp); }

        file.close();
    }

    return true;
}

unsigned long countLinesInDir(const std::string &dirName) {
    auto files = getFilesInDirectory(dirName);
    if (files.empty()) {
        return -1;
    }

    unsigned long totalLineCount = 0;

#pragma omp parallel for
    for (unsigned int i = 0; i < files.size(); i++) {
        const auto &fileName = files[i];
        if (fileName[fileName.size() - 4] != '.' || fileName[fileName.size() - 3] != 'x' ||
            fileName[fileName.size() - 2] != 'y' || fileName[fileName.size() - 1] != 'z') {
            continue;
        }

        std::ifstream file;
        file.open(fileName, std::ios::in | std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Could not open file: " << fileName << std::endl;
            continue;
        }

        size_t fileSize = file.tellg();
        file.seekg(0);

        char *buffer = reinterpret_cast<char *>(std::malloc(fileSize));
        file.read(buffer, fileSize);

        unsigned long lineCount = 0;
        char *workingBuffer = buffer;
        while (workingBuffer - buffer < fileSize) {
            if (*workingBuffer == '\n') {
                lineCount++;
            }
            workingBuffer++;
        }
        std::free(buffer);
        file.close();

#pragma omp critical
        { totalLineCount += lineCount; }
    }

    return totalLineCount;
}
