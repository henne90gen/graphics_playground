#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct BoundingBox3 {
    glm::vec3 min;
    glm::vec3 max;
};

bool loadXyzDir(const std::string &dirName, BoundingBox3 &bb, std::vector<glm::vec3> &result);
bool loadXyzDir(const std::string &dirName, const std::function<void(const std::vector<glm::vec3> &)> &takePointsFunc);
bool loadXyzDir(const std::vector<std::string> &files,
                const std::function<void(const std::vector<glm::vec3> &)> &takePointsFunc);
unsigned long countLinesInDir(const std::string &dirName);
