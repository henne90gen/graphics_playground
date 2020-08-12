#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

bool loadXyzDir(const std::string &dirName, std::vector<glm::vec3> &result);
