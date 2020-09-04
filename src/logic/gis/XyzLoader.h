#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "util/BoundingBox.h"

typedef std::function<void(unsigned int, const std::vector<glm::vec3> &)> TakePointsFunc;

bool loadXyzDir(const std::string &dirName, BoundingBox3 &bb, std::vector<glm::vec3> &result);
bool loadXyzDir(const std::string &dirName, const TakePointsFunc &takePointsFunc);
bool loadXyzDir(const std::vector<std::string> &files, const TakePointsFunc &takePointsFunc);

unsigned long countLinesInDir(const std::string &dirName);
inline unsigned int getBatchName(const std::string &fileName);
