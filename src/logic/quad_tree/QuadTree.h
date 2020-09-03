#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "util/BoundingBox.h"

/*
 * Running the benchmark revealed that K=256 is a pretty optimal size for the tree.
 */

inline float distanceSq(const glm::vec3 &p1, const glm::vec3 &p2) {
    auto vec = p1 - p2;
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

template <typename T, unsigned int S> struct QuadTree {
    using Element = typename std::pair<glm::vec3, T>;
    using Iterator = typename std::vector<Element>::iterator;

    struct Node {
        Iterator begin;
        Iterator end;
        BoundingBox3 bb = {};
        bool parity = false;
        Node *left = nullptr;
        Node *right = nullptr;

        Node(Iterator begin, Iterator end) : begin(begin), end(end) { init(); }
        Node(Iterator begin, Iterator end, bool parity) : begin(begin), end(end), parity(parity) { init(); }
        ~Node() {
            delete left;
            delete right;
        }

        void init() {
            std::function<bool(const Element &, const Element &)> comp;
            if (parity) {
                comp = [](const Element &l, const Element &r) { return l.first.x < r.first.x; };
            } else {
                comp = [](const Element &l, const Element &r) { return l.first.z < r.first.z; };
            }
            std::sort(begin, end, comp);

            long numElements = end - begin;
            if (numElements <= S) {
                for (Iterator itr = begin; itr != end; itr++) {
                    bb.update(itr->first);
                }
            } else {
                long numElementsHalf = numElements / 2;
                left = new Node(begin, begin + numElementsHalf, !parity);
                bb.update(left->bb);

                right = new Node(begin + numElementsHalf, end, !parity);
                bb.update(right->bb);
            }
        }

        inline bool isLeaf() { return left == nullptr; }
    };

    std::vector<Element> elements = {};
    Node *root = new Node(elements.begin(), elements.end());

    ~QuadTree() { delete root; }

    void insert(const glm::vec3 &point, T data) {
        delete root;
        elements.push_back(std::make_pair(point, data));
        root = new Node(elements.begin(), elements.end());
    }

    void insert(const std::vector<Element> &newElements) {
        delete root;
        elements.reserve(elements.size() + newElements.size());
        for (const auto &element : newElements) {
            elements.push_back(element);
        }
        root = new Node(elements.begin(), elements.end());
    }

    // TODO use limit parameter
    // TODO add parameter k for knn lookup
    bool get(const glm::vec3 &query, T &closestElement, const float limit) {
        if (elements.empty()) {
            return false;
        }

        // std::deque<std::pair<float, T>> closestElements = {};
        std::vector<Node *> nodeQueue = {root};
        float limitSq = limit * limit;
        float minDistSq = std::numeric_limits<float>::max();
        while (!nodeQueue.empty()) {
            Node *currentNode = nodeQueue.back();
            nodeQueue.pop_back();

            auto closestPoint = currentNode->bb.closestPointOnSurface(query);
            auto distSq = distanceSq(closestPoint, query);
            if (distSq >= minDistSq) {
                continue;
            }

            if (!currentNode->isLeaf()) {
                glm::vec3 closestPointLeft = currentNode->left->bb.closestPointOnSurface(query);
                glm::vec3 closestPointRight = currentNode->right->bb.closestPointOnSurface(query);
                float distLeftSq = distanceSq(closestPointLeft, query);
                float distRightSq = distanceSq(closestPointRight, query);
                if (distLeftSq < distRightSq) {
                    nodeQueue.push_back(currentNode->right);
                    nodeQueue.push_back(currentNode->left);
                } else {
                    nodeQueue.push_back(currentNode->left);
                    nodeQueue.push_back(currentNode->right);
                }
            } else {
                for (Iterator itr = currentNode->begin; itr != currentNode->end; itr++) {
                    float distSq = distanceSq(itr->first, query);
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        closestElement = itr->second;
                    }
                }
            }
        }
        return true;
    }

    bool get(const glm::vec3 &query, T &closestElement) {
        return get(query, closestElement, std::numeric_limits<float>::max());
    }
};
