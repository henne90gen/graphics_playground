#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "util/BoundingBox.h"

using TakePointsFunc = std::function<void(const std::string &, const std::vector<glm::vec3> &)>;

bool isXyzFile(const std::string &fileName);
bool loadXyzDir(const std::string &dirName, BoundingBox3 &bb, std::vector<glm::vec3> &result);
bool loadXyzDir(const std::string &dirName, const TakePointsFunc &takePointsFunc);
bool loadXyzDir(const std::vector<std::string> &files, const TakePointsFunc &takePointsFunc);

std::vector<glm::vec3> loadXyzFile(const std::string &fileName);

unsigned long countLinesInDir(const std::string &dirName);
