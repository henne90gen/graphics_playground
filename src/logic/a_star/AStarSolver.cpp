#include <algorithm>
#include "AStarSolver.h"

bool AStarSolver::nextStep(Board &board) {
    if (goal.x == -1 && goal.y == -1) {
        goal = findGoal(board);
        if (goal.x == -1 && goal.y == -1) {
            return false;
        }
    }

    if (workingSet.empty()) {
        Node start = findStart(board, goal);
        if (start.position.x == -1 && start.position.y == -1) {
            return false;
        }

        addNeighborsToWorkingSet(board, start);
        if (workingSet.empty()) {
            return false;
        }
    }

    std::function<bool(Node &, Node &)> compare = [](Node &node1, Node &node2) -> bool {
        return node1.estimatedTotalDistance < node2.estimatedTotalDistance;
    };
    std::sort(workingSet.begin(), workingSet.end(), compare);

    Node &node = workingSet.front();
    if (node.position == goal) {
        return true;
    }

    addNeighborsToWorkingSet(board, node);

    return false;
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

void AStarSolver::addNeighborsToWorkingSet(Board &board, Node node) {
    for (auto &pos : visited) {
        if (node.position == pos) {
            return;
        }
    }
    visited.push_back(node.position);
    board.pixels[node.position.x + node.position.y * board.height] = visitedColor;

    float traveledDistance = node.traveledDistance + 1;
    glm::ivec2 topPos = node.position + glm::ivec2(0, 1);
    Node top = {traveledDistance, calculateDistance(topPos, goal), topPos, node.position};

    glm::ivec2 bottomPos = node.position + glm::ivec2(0, -1);
    Node bottom = {traveledDistance, calculateDistance(bottomPos, goal), bottomPos, node.position};

    glm::ivec2 leftPos = node.position + glm::ivec2(-1, 0);
    Node left = {traveledDistance, calculateDistance(leftPos, goal), leftPos, node.position};

    glm::ivec2 rightPos = node.position + glm::ivec2(1, 0);
    Node right = {traveledDistance, calculateDistance(rightPos, goal), rightPos, node.position};

    workingSet.push_back(top);
    workingSet.push_back(bottom);
    workingSet.push_back(left);
    workingSet.push_back(right);
}
