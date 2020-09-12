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

    bool get(const glm::vec3 &query, const unsigned int k, std::vector<T> &result) {
        if (elements.empty()) {
            return false;
        }

        std::vector<std::pair<float, T>> closestElements = {};
        std::vector<Node *> nodeQueue = {root};
        while (!nodeQueue.empty()) {
            Node *currentNode = nodeQueue.back();
            nodeQueue.pop_back();

            const auto closestPoint = currentNode->bb.closestPointOnSurface(query);
            const auto distToBBSq = distanceSq(closestPoint, query);
            if (!closestElements.empty() && distToBBSq >= closestElements.back().first) {
                continue;
            }

            if (!currentNode->isLeaf()) {
                glm::vec3 closestPointLeft = currentNode->left->bb.closestPointOnSurface(query);
                glm::vec3 closestPointRight = currentNode->right->bb.closestPointOnSurface(query);
                const float distLeftSq = distanceSq(closestPointLeft, query);
                const float distRightSq = distanceSq(closestPointRight, query);
                if (distLeftSq < distRightSq) {
                    nodeQueue.push_back(currentNode->right);
                    nodeQueue.push_back(currentNode->left);
                } else {
                    nodeQueue.push_back(currentNode->left);
                    nodeQueue.push_back(currentNode->right);
                }
                continue;
            }

            for (Iterator itr = currentNode->begin; itr != currentNode->end; itr++) {
                const float distSq = distanceSq(itr->first, query);
                if (closestElements.empty() || closestElements.size() < k || distSq < closestElements.back().first) {
                    closestElements.push_back(std::make_pair(distSq, itr->second));
                    std::sort(closestElements.begin(), closestElements.end(),
                              [](const std::pair<float, T> &e1, const std::pair<float, T> &e2) {
                                  return e1.first < e2.first;
                              });
                    while (closestElements.size() > k) {
                        closestElements.pop_back();
                    }
                }
            }
        }

        for (const auto &elem : closestElements) {
            result.push_back(elem.second);
        }

        return true;
    }

    bool get(const glm::vec3 &query, T &closestElement) {
        std::vector<T> result = {};
        bool success = get(query, 1, result);
        if (!success || result.empty()) {
            return false;
        }
        closestElement = result[0];
        return true;
    }

  private:
    std::vector<Element> elements = {};
    Node *root = new Node(elements.begin(), elements.end());
};
