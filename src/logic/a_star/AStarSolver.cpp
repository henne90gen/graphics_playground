#include "AStarSolver.h"

#include <algorithm>
#include <functional>

void AStarSolver::nextStep(Board &board) {
    if (solved) {
        return;
    }

    if (goal.x == -1 && goal.y == -1) {
        goal = findGoal(board);
        if (goal.x == -1 && goal.y == -1) {
            return;
        }
    }

    if (workingSet.empty()) {
        Node start = findStart(board, goal);
        if (start.position.x == -1 && start.position.y == -1) {
            return;
        }

        addNeighborsToWorkingSet(board, start);
        if (workingSet.empty()) {
            return;
        }
    }

    std::function<bool(Node &, Node &)> compare = [](Node &node1, Node &node2) -> bool {
        return node1.estimatedTotalDistance > node2.estimatedTotalDistance;
    };
    std::sort(workingSet.begin(), workingSet.end(), compare);

    Node &node = workingSet.back();
    workingSet.pop_back();
    if (node.position == goal) {
        solved = true;
        return;
    }

    addNeighborsToWorkingSet(board, node);
}

glm::ivec2 AStarSolver::findGoal(const Board &board) const {
    for (unsigned int y = 0; y < board.height; y++) {
        for (unsigned int x = 0; x < board.width; x++) {
            if (board.pixels[x + y * board.width] == finishColor) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

Node
AStarSolver::findStart(const Board &board, const glm::ivec2 &finish) {
    for (unsigned int y = 0; y < board.height; y++) {
        for (unsigned int x = 0; x < board.width; x++) {
            if (board.pixels[x + y * board.width] == startColor) {
                glm::ivec2 pos = {x, y};
                float distanceToFinish = calculateDistance(pos, finish);
                return {
                        0,
                        distanceToFinish,
                        pos,
                        {-1, -1}
                };
            }
        }
    }
    return {0, 0, {-1, -1}, {-1, -1}};
}

float AStarSolver::calculateDistance(const glm::ivec2 &pos1, const glm::ivec2 &pos2) {
    return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}

inline glm::vec3 getPixelValue(Board &board, glm::ivec2 &pos) {
    return board.pixels[pos.x + pos.y * board.height];
}

inline void setPixelValue(Board &board, glm::ivec2 &pos, glm::vec3 color) {
    board.pixels[pos.x + pos.y * board.height] = color;
}

bool AStarSolver::isValidNeighbor(Board &board, glm::ivec2 &pos) {
    if (pos.x < 0 || pos.x >= board.width || pos.y < 0 || pos.y >= board.height) {
        return false;
    }

    glm::vec3 color = getPixelValue(board, pos);
    return color != obstacleColor && color != visitedColor && color != startColor && color != workingSetColor;
}

void AStarSolver::addNeighborsToWorkingSet(Board &board, Node node) {
    glm::vec3 pixel = getPixelValue(board, node.position);
    if (pixel != startColor) {
        setPixelValue(board, node.position, visitedColor);
    }

    float traveledDistance = node.traveledDistance + 1;
    glm::ivec2 topPos = node.position + glm::ivec2(0, 1);
    Node top = {traveledDistance, calculateDistance(topPos, goal), topPos, node.position};

    glm::ivec2 bottomPos = node.position + glm::ivec2(0, -1);
    Node bottom = {traveledDistance, calculateDistance(bottomPos, goal), bottomPos, node.position};

    glm::ivec2 leftPos = node.position + glm::ivec2(-1, 0);
    Node left = {traveledDistance, calculateDistance(leftPos, goal), leftPos, node.position};

    glm::ivec2 rightPos = node.position + glm::ivec2(1, 0);
    Node right = {traveledDistance, calculateDistance(rightPos, goal), rightPos, node.position};

    addNeighbor(board, top);
    addNeighbor(board, bottom);
    addNeighbor(board, left);
    addNeighbor(board, right);
}

void AStarSolver::addNeighbor(Board &board, Node &node) {
    if (!isValidNeighbor(board, node.position)) {
        return;
    }

    glm::vec3 pixel = getPixelValue(board, node.position);
    if (pixel != startColor && pixel != finishColor) {
        setPixelValue(board, node.position, workingSetColor);
    }

    workingSet.push_back(node);
}
