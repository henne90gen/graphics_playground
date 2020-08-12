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

bool loadXyzDir(const std::string &dirName, std::vector<glm::vec3> &result) {
    result.clear();

    auto files = getFilesInDirectory(dirName);
    result.reserve(files.size() * 10000);

#pragma omp parallel for
    for (int i = 0; i < files.size(); i++) {
        const auto &fileName = files[i];

#if USE_STRING_STREAM
        std::ifstream file;
        file.open(fileName, std::ios::in);
        if (!file.is_open()) {
            std::cout << "Could not open file: " << fileName << std::endl;
            continue;
        }

        std::vector<glm::vec3> temp = {};
        std::string line;
        std::stringstream lineStream;
        while (std::getline(file, line)) {
            lineStream = std::stringstream(line);
            glm::vec3 vec;
            lineStream >> vec.x >> vec.y >> vec.z;
            temp.push_back(vec);
        }

#pragma omp critical
        { result.insert(result.end(), temp.begin(), temp.end()); }
#else
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
        while (workingBuffer - buffer < fileSize) {
            size_t lineLength = 0;
            while (true) {
                if (workingBuffer[lineLength] == '\n') {
                    lineLength++;
                    break;
                }
                lineLength++;
            }

            glm::vec3 vec;
            char *last = nullptr;
            vec.x = strtof(workingBuffer, &last);
            vec.y = strtof(last, &last);
            vec.z = strtof(last, &last);
            temp.push_back(vec);

            workingBuffer = workingBuffer + lineLength;
        }
        std::free(buffer);

#pragma omp critical
        { result.insert(result.end(), temp.begin(), temp.end()); }
#endif
        file.close();
    }

    return true;
}
