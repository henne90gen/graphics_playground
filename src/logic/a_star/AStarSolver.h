#pragma once

#include <functional>
#include <glm/ext.hpp>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include "util/TimeUtils.h"

const glm::vec3 startColor = {53.0 / 255.0, 210.0 / 255.0, 141.0 / 255.0};
const glm::vec3 finishColor = {235.0 / 255.0, 51.0 / 255.0, 37.0 / 255.0};
const glm::vec3 backgroundColor = {156.0 / 255.0, 140.0 / 255.0, 135.0 / 255.0};

const glm::vec3 obstacleColor = {45.0 / 255.0, 57.0 / 255.0, 66.0 / 255.0};
const glm::vec3 pathColor = {74.0 / 255.0, 102.0 / 255.0, 248.0 / 255.0};

const glm::vec3 visitedColor = {85.0 / 255.0, 191.0 / 255.0, 240.0 / 255.0};
const glm::vec3 workingSetColor = {65.0 / 255.0, 209.0 / 255.0, 217.0 / 255.0};

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
