#pragma once

#include <glm/ext.hpp>
#include <vector>

const glm::vec3 startColor = {0.0, 1.0, 0.0};
const glm::vec3 finishColor = {1.0, 0.0, 0.0};
const glm::vec3 obstacleColor = {0.0, 0.0, 0.0};
const glm::vec3 pathColor = {0.0, 0.0, 1.0};
const glm::vec3 visitedColor = {0.8, 0.8, 0.8};
const glm::vec3 backgroundColor = {1.0, 0.0, 1.0};
const glm::vec3 workingSetColor = {0.0, 1.0, 1.0};

struct Board {
    unsigned int width;
    unsigned int height;
    std::vector<glm::vec3> pixels;
};

struct Node {
    float traveledDistance;
    float estimatedTotalDistance;
    glm::ivec2 position;
    glm::ivec2 predecessor;
};

class AStarSolver {
public:
    void nextStep(Board &board);

    std::vector<Node> workingSet = {};
    bool solved = false;
private:
    glm::ivec2 goal = {-1, -1};
    std::vector<glm::ivec2> visited = {};

    glm::ivec2 findGoal(const Board &board) const;

    static Node findStart(const Board &board, const glm::ivec2 &finish);

    static float calculateDistance(const glm::ivec2 &pos1, const glm::ivec2 &pos2);

    bool isValidNeighbor(Board &board, glm::ivec2 &pos);

    void addNeighborsToWorkingSet(Board &board, Node node);

    void addNeighbor(Board &board, Node &node);
};
