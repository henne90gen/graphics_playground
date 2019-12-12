#pragma once

#include <glm/ext.hpp>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <functional>

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
    unsigned int g;
    float f;
    glm::ivec2 position;
    Node *predecessor;
};


class AStarSolver {
public:
    void nextStep(Board &board);

    std::vector<Node *> workingSet;
    std::vector<Node *> visitedSet;

    bool solved = false;
    Node *finalNode = nullptr;
    bool useManhattenDistance = false;
private:
    glm::ivec2 goal = {-1, -1};

    static glm::ivec2 findGoal(const Board &board) ;

    Node *findStart(const Board &board);

    float h(const glm::ivec2 &pos1, const glm::ivec2 &pos2);

    bool isValidNeighbor(Board &board, glm::ivec2 &pos);

    void drawFinalPath(Board &board);

    static std::vector<Node *> getNeighbors(Board &board, Node *pNode);

    Node *getNodeFromWorkingSet(Node *pNode);
};
