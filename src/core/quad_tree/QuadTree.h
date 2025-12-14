#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "util/BoundingBox.h"

/*
 * Running the benchmark revealed that K=256 is a pretty optimal size for the tree.
 */
constexpr unsigned int DEFAULT_MAX_ELEMENTS_PER_NODE = 256;

inline float distanceSq(const glm::vec3 &p1, const glm::vec3 &p2) {
    auto vec = p1 - p2;
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

template <typename T> struct QuadTree {
    using Element = typename std::pair<glm::vec3, T>;
    using Iterator = typename std::vector<Element>::iterator;

    struct Node {
        Iterator begin;
        Iterator end;
        BoundingBox3 bb = {};
        bool parity = false;
        std::shared_ptr<Node> left = nullptr;
        std::shared_ptr<Node> right = nullptr;

        Node(unsigned int maxElementsPerNode, Iterator begin, Iterator end) : begin(begin), end(end) {
            init(maxElementsPerNode);
        }
        Node(unsigned int maxElementsPerNode, Iterator begin, Iterator end, bool parity)
            : begin(begin), end(end), parity(parity) {
            init(maxElementsPerNode);
        }
        ~Node() = default;

        void init(unsigned int _maxElementsPerNode) {
            std::function<bool(const Element &, const Element &)> comp;
            if (parity) {
                comp = [](const Element &l, const Element &r) { return l.first.x < r.first.x; };
            } else {
                comp = [](const Element &l, const Element &r) { return l.first.z < r.first.z; };
            }
            std::sort(begin, end, comp);

            const auto numElements = end - begin;
            if (numElements <= _maxElementsPerNode) {
                for (Iterator itr = begin; itr != end; itr++) {
                    bb.update(itr->first);
                }
                return;
            }

            const auto numElementsHalf = numElements / 2;
            left = std::make_shared<Node>(_maxElementsPerNode, begin, begin + numElementsHalf, !parity);
            bb.update(left->bb);

            right = std::make_shared<Node>(_maxElementsPerNode, begin + numElementsHalf, end, !parity);
            bb.update(right->bb);
        }

        inline bool isLeaf() { return left == nullptr; }
    };

    QuadTree()
        : maxElementsPerNode(DEFAULT_MAX_ELEMENTS_PER_NODE), elements({}),
          root(std::make_shared<Node>(maxElementsPerNode, elements.begin(), elements.end())) {}
    explicit QuadTree(unsigned int maxElementsPerNode)
        : maxElementsPerNode(maxElementsPerNode), elements({}),
          root(std::make_shared<Node>(maxElementsPerNode, elements.begin(), elements.end())) {}
    ~QuadTree() = default;
    void insert(const glm::vec3 &point, T data) {
        elements.push_back(std::make_pair(point, data));
        root = std::make_shared<Node>(maxElementsPerNode, elements.begin(), elements.end());
    }

    void insert(const std::vector<Element> &newElements) {
        elements.reserve(elements.size() + newElements.size());
        for (const auto &element : newElements) {
            elements.push_back(element);
        }
        root = std::make_shared<Node>(maxElementsPerNode, elements.begin(), elements.end());
    }

    bool get(const glm::vec3 &query, const unsigned int k, std::vector<T> &result) const {
        if (elements.empty()) {
            return false;
        }

        std::vector<std::pair<float, T>> closestElements = {};
        std::vector<std::shared_ptr<Node>> nodeQueue = {root};
        while (!nodeQueue.empty()) {
            std::shared_ptr<Node> currentNode = nodeQueue.back();
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

    bool get(const glm::vec3 &query, T &closestElement) const {
        std::vector<T> result = {};
        bool success = get(query, 1, result);
        if (!success || result.empty()) {
            return false;
        }
        closestElement = result[0];
        return true;
    }

    void traversePreOrder(const std::function<void(std::shared_ptr<Node>)> &traversalFunc) const {
        std::vector<std::shared_ptr<Node>> stack = {root};
        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();
            if (current == nullptr) {
                continue;
            }

            traversalFunc(current);

            stack.push_back(current->left);
            stack.push_back(current->right);
        }
    }

    void traversePostOrder(const std::function<void(std::shared_ptr<Node>)> &traversalFunc) const {
        std::vector<std::shared_ptr<Node>> stack = {};
        std::shared_ptr<Node> current = root;
        while (true) {
            while (current != nullptr) {
                if (current->right != nullptr) {
                    stack.push_back(current->right);
                }
                stack.push_back(current);

                current = current->left;
            }

            current = stack.back();
            stack.pop_back();

            if (current->right != nullptr && (!stack.empty() && stack[stack.size() - 1] == current->right)) {
                stack.pop_back();
                stack.push_back(current);
                current = current->right;
            } else {
                traversalFunc(current);
                current = nullptr;
            }

            if (stack.empty()) {
                break;
            }
        }
    }

    void traverseInOrder(const std::function<void(std::shared_ptr<Node>)> &traversalFunc) const {
        std::vector<std::shared_ptr<Node>> stack = {};
        std::shared_ptr<Node> current = root;
        while (true) {
            if (current != nullptr) {
                stack.push_back(current);
                current = current->left;
            } else if (!stack.empty()) {
                current = stack.back();
                stack.pop_back();
                if (current == nullptr) {
                    continue;
                }

                traversalFunc(current);

                current = current->right;
            } else {
                break;
            }
        }
    }

    unsigned int maxElementsPerNode = DEFAULT_MAX_ELEMENTS_PER_NODE;
    std::vector<Element> elements = {};
    std::shared_ptr<Node> root = nullptr;
};
