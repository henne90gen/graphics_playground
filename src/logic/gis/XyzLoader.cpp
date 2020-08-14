#include "XyzLoader.h"

#define USE_STRING_STREAM 0

#include <fstream>
#include <iostream>
#if USE_STRING_STREAM
#include <sstream>
#endif

#include "util/FileUtils.h"

// 10
// Stream-NoOmp BM_Load       52450609 ns      52368904 ns           11
// Custom-NoOmp BM_Load       16374400 ns      16350382 ns           41
// Stream-Omp   BM_Load       46006784 ns      37812791 ns           18
// Custom-Omp   BM_Load       16467553 ns      16050949 ns           40

// all
// Stream-NoOmp BM_Load    56988497077 ns   56825585013 ns            1
// Custom-NoOmp BM_Load    11108723440 ns   11095611956 ns            1
// Stream-Omp   BM_Load    18314706058 ns   15897039255 ns            1
// Custom-Omp   BM_Load     4150112235 ns    3929744017 ns            1

bool loadXyzDir(const std::string &dirName, std::vector<glm::vec3> &result, BoundingBox3 &bb) {
    bb = {{std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()},
          {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}};
    result.clear();

    auto files = getFilesInDirectory(dirName);
    if (files.empty()) {
        return false;
    }

    unsigned long fileCount = files.size();
    fileCount = 1000;

    result.reserve(files.size() * 10000);
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
            vec.y = strtof(last, &last);
            vec.z = strtof(last, &last);
            temp.push_back(vec);

            workingBuffer = last;
        }
        std::free(buffer);

#define UPDATE(left, op, right)                                                                                        \
    if ((left)op(right)) {                                                                                             \
        (right) = (left);                                                                                              \
    }

#pragma omp critical
        {
            for (unsigned int i = 0; i < temp.size(); i++) {
                UPDATE(temp[i].x, <, bb.min.x)
                UPDATE(temp[i].y, <, bb.min.y)
                UPDATE(temp[i].z, <, bb.min.z)

                UPDATE(temp[i].x, >, bb.max.x)
                UPDATE(temp[i].y, >, bb.max.y)
                UPDATE(temp[i].z, >, bb.max.z)

                result.push_back(temp[i]);
            }
        }

        file.close();
    }

    return true;
}
