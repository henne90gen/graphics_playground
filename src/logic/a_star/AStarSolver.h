#pragma once

#include <glm/ext.hpp>
#include <vector>

const glm::vec3 startColor = {0.0, 1.0, 0.0};
const glm::vec3 finishColor = {1.0, 0.0, 0.0};
const glm::vec3 pathColor = {0.0, 0.0, 1.0};

class AStarSolver {
public:
    void nextStep(std::vector<glm::vec3> &board, unsigned int width, unsigned int height);
};
