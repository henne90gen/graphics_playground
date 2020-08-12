#include "XyzLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "util/FileUtils.h"

/*
TODO
This is way too slow!
Make sure all files have the same format (line length, position of spaces and punctuation), then we can just memory map
them and hopefully convert them faster.
*/
bool loadXyzDir(const std::string &dirName, std::vector<glm::vec3> &result) {
    result.clear();

    auto files = getFilesInDirectory(dirName);
    // std::cout << "Found " << files.size() << " xyz files" << std::endl;
#pragma omp parallel for
    for (int i = 0; i < 10; i++) {
        const auto &fileName = files[i];

        std::ifstream file;
        file.open(fileName, std::ios::in | std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Could not open file: " << fileName << std::endl;
            continue;
        }

        /*
                std::string line;
                std::stringstream lineStream;
                while (std::getline(file, line)) {
                    lineStream = std::stringstream(line);
                    glm::vec3 vec;
                    lineStream >> vec.x >> vec.y >> vec.z;
                    result.push_back(vec);
                }*/
        file.close();
    }

    return true;
}
