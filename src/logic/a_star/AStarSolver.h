#pragma once

#include <functional>
#include <glm/ext.hpp>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include "util/TimeUtils.h"

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

    double getTotalTime() {
        const auto &itr = perfCounter.dataPoints.find("TotalTime");
        if (itr == perfCounter.dataPoints.end()) {
            return 0.0;
        }
        return itr->second._sum;
    }

    unsigned int getTotalSteps() {
        const auto &itr = perfCounter.dataPoints.find("TotalTime");
        if (itr == perfCounter.dataPoints.end()) {
            return 0.0;
        }
        return itr->second.timerCount;
    }

    void drawFinalPath(Board &board);
  private:
    glm::ivec2 goal = {-1, -1};
    PerformanceCounter perfCounter = {};

    static glm::ivec2 findGoal(const Board &board);
    Node *findStart(const Board &board);
    float h(const glm::ivec2 &pos1, const glm::ivec2 &pos2);
    static std::vector<Node *> getNeighbors(const Board &board, Node *pNode);
    Node *getNodeFromWorkingSet(Node *pNode);
    inline PerformanceCounter *getPerformanceCounter() { return &perfCounter; }
};
