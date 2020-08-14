#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct BoundingBox3 {
    glm::vec3 min;
    glm::vec3 max;
};

bool loadXyzDir(const std::string &dirName, std::vector<glm::vec3> &result, BoundingBox3 &bb);
