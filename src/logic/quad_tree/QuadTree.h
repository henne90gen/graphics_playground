#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "util/BoundingBox.h"

template <typename T> struct Node {
    using Element = typename std::pair<glm::vec3, T>;
    using Iterator = typename std::vector<Element>::iterator;

    Iterator begin;
    Iterator end;
    BoundingBox3 bb;
    bool parity = false;
    Node<T> *left = nullptr;
    Node<T> *right = nullptr;

    Node(Iterator begin, Iterator end) : begin(begin), end(end) { init(); }
    Node(Iterator begin, Iterator end, bool parity) : begin(begin), end(end), parity(parity) { init(); }

    void init() {
        std::function<bool(const Element &, const Element &)> comp;
        if (parity) {
            comp = [](const Element &l, const Element &r) { return l.first.x < r.first.x; };
        } else {
            comp = [](const Element &l, const Element &r) { return l.first.z < r.first.z; };
        }
        std::sort(begin, end, comp);

        long numElements = end - begin;
        if (numElements <= 5) {
            for (Iterator itr = begin; itr != end; itr++) {
                bb.update(itr->first);
            }
        } else {
            long numElementsHalf = numElements / 2;
            left = new Node<T>(begin, begin + numElementsHalf, !parity);
            bb.update(left->bb);

            right = new Node<T>(begin + numElementsHalf, end, !parity);
            bb.update(right->bb);
        }
    }
};

template <typename T> struct QuadTree {
    using Element = typename std::pair<glm::vec3, T>;
    using Iterator = typename std::vector<Element>::iterator;

    std::vector<Element> elements = {};
    Node<T> *root = new Node<T>(elements.begin(), elements.end());

    void insert(const glm::vec3 &point, T data) {
        delete root;
        elements.push_back(std::make_pair(point, data));
        root = new Node<T>(elements.begin(), elements.end());
    }

    void insert(const std::vector<Element> &newElements) {
        delete root;
        elements.reserve(elements.size() + newElements.size());
        for (const auto &element : newElements) {
            elements.push_back(element);
        }
        root = new Node<T>(elements.begin(), elements.end());
    }

    T get(const glm::vec3 &query) {
        std::vector<Node<T> *> nodeQueue = {root};
        float minDistSq = std::numeric_limits<float>::max();
        T closestElement;
        while (!nodeQueue.empty()) {
            Node<T> *currentNode = nodeQueue.back();
            nodeQueue.pop_back();

            for (Iterator itr = currentNode->begin; itr != currentNode->end; itr++) {
                auto distVec = itr->first - query;
                float distSq = distVec.x * distVec.x + distVec.y * distVec.y + distVec.z * distVec.z;
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closestElement = itr->second;
                }
            }
        }
        return closestElement;
    }
};
