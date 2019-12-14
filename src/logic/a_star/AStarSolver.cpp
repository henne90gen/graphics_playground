#include <cmath>

#include "AStarSolver.h"

#include <algorithm>
#include <functional>
#include <iostream>

inline void setPixelValue(Board &board, glm::ivec2 &pos, glm::vec3 color) {
    board.pixels[pos.x + pos.y * board.height] = color;
}

inline glm::vec3 getPixelValue(const Board &board, glm::ivec2 &pos) {
    return board.pixels[pos.x + pos.y * board.height];
}

bool compareNodes(Node *n1, Node *n2) { return n1->f > n2->f; }

void AStarSolver::nextStep(Board &board) {

    if (solved) {
        drawFinalPath(board);
        return;
    }

    if (goal.x == -1 && goal.y == -1) {
        goal = findGoal(board);
        if (goal.x == -1 && goal.y == -1) {
            return;
        }
    }

    if (workingSet.empty()) {
        Node *start = findStart(board);
        if (start == nullptr) {
            return;
        }
        workingSet.push_back(start);
        std::sort(workingSet.begin(), workingSet.end(), compareNodes);
    }

    Node *node = workingSet.back();
    workingSet.pop_back();
    if (node->position == goal) {
        solved = true;
        finalNode = node;
        return;
    }

    visitedSet.push_back(node);
    if (getPixelValue(board, node->position) != startColor) {
        setPixelValue(board, node->position, visitedColor);
    }

    for (auto neighbor : getNeighbors(board, node)) {
        neighbor->g = neighbor->predecessor->g + 1;
        neighbor->f = neighbor->g + h(neighbor->position, goal);
        Node *existingNode = getNodeFromWorkingSet(neighbor);
        if (existingNode != nullptr) {
            // THIS HAS TO BE '>=' FOR THE ALGORITHM TO WORK!
            if (existingNode->f >= neighbor->f) {
                existingNode->g = neighbor->g;
                existingNode->f = neighbor->f;
                existingNode->predecessor = neighbor->predecessor;
            }
        } else {
            workingSet.push_back(neighbor);
            std::sort(workingSet.begin(), workingSet.end(), compareNodes);
            setPixelValue(board, neighbor->position, workingSetColor);
        }
    }
}

glm::ivec2 AStarSolver::findGoal(const Board &board) {
    for (unsigned int y = 0; y < board.height; y++) {
        for (unsigned int x = 0; x < board.width; x++) {
            if (board.pixels[x + y * board.width] == finishColor) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

Node *AStarSolver::findStart(const Board &board) {
    for (unsigned int y = 0; y < board.height; y++) {
        for (unsigned int x = 0; x < board.width; x++) {
            if (board.pixels[x + y * board.width] == startColor) {
                glm::ivec2 pos = {x, y};
                float distanceToFinish = h(pos, goal);
                Node *result = new Node();
                result->g = 0;
                result->f = distanceToFinish;
                result->predecessor = nullptr;
                result->position = pos;
                return result;
            }
        }
    }
    return nullptr;
}

float manhattenDistance(const glm::ivec2 &pos1, const glm::ivec2 &pos2) {
    float xDiff = abs(pos1.x - pos2.x);
    float yDiff = abs(pos1.y - pos2.y);
    return xDiff + yDiff;
}

float absoluteDistance(const glm::ivec2 &pos1, const glm::ivec2 &pos2) {
    float xDiff = pos1.x - pos2.x;
    float yDiff = pos1.y - pos2.y;
    return std::sqrt(xDiff * xDiff + yDiff * yDiff);
}

float AStarSolver::h(const glm::ivec2 &pos1, const glm::ivec2 &pos2) {
    if (useManhattenDistance) {
        return manhattenDistance(pos1, pos2);
    }
    return absoluteDistance(pos1, pos2);
}

bool isValidNeighbor(const Board &board, glm::ivec2 &pos) {
    if (pos.x < 0 || pos.x >= board.width || pos.y < 0 || pos.y >= board.height) {
        return false;
    }

    glm::vec3 color = getPixelValue(board, pos);
    return color != obstacleColor && color != startColor && color != visitedColor;
}

void AStarSolver::drawFinalPath(Board &board) {
    Node *currentNode = finalNode;
    while (currentNode->predecessor != nullptr) {
        setPixelValue(board, currentNode->position, pathColor);
        currentNode = currentNode->predecessor;
    }
}

void addNeighbor(const Board &board, std::vector<Node *> &neighbors, Node *node, glm::ivec2 offset) {
    glm::ivec2 topPos = node->position + offset;
    if (!isValidNeighbor(board, topPos)) {
        return;
    }

    Node *top = new Node();
    top->position = topPos;
    top->predecessor = node;

    neighbors.push_back(top);
}

std::vector<Node *> AStarSolver::getNeighbors(const Board &board, Node *node) {
    auto result = std::vector<Node *>();

    addNeighbor(board, result, node, {0, 1});
    addNeighbor(board, result, node, {0, -1});
    addNeighbor(board, result, node, {1, 0});
    addNeighbor(board, result, node, {-1, 0});

    addNeighbor(board, result, node, {1, 1});
    addNeighbor(board, result, node, {-1, 1});
    addNeighbor(board, result, node, {-1, -1});
    addNeighbor(board, result, node, {1, -1});

    return result;
}

Node *AStarSolver::getNodeFromWorkingSet(Node *node) {
    for (auto other : workingSet) {
        if (node->position == other->position) {
            return other;
        }
    }
    return nullptr;
}
