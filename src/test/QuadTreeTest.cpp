#include <catch.hpp>
#include <glm/glm.hpp>

#include "quad_tree/QuadTree.h"

TEST_CASE("Can add one element") {
    auto tree = QuadTree<unsigned int>();

    auto point = glm::vec3(1.0F);
    unsigned int elem = 123;
    tree.insert(point, elem);

    REQUIRE(tree.elements.size() == 1);
    REQUIRE(tree.root->parity == 0);
    REQUIRE(tree.root->left == nullptr);
    REQUIRE(tree.root->right == nullptr);
    REQUIRE(tree.root->begin->first == point);
    REQUIRE(tree.root->begin->second == elem);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(1.0F));
}

TEST_CASE("Can add multiple elements") {
    auto tree = QuadTree<unsigned int>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0F), 3)  //
    };
    tree.insert(elements);

    REQUIRE(tree.elements.size() == 3);
    REQUIRE(tree.root->parity == 0);
    REQUIRE(tree.root->left == nullptr);
    REQUIRE(tree.root->right == nullptr);
    REQUIRE(tree.root->begin->first == glm::vec3(1.0));
    REQUIRE(tree.root->begin->second == 1);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(3.0F));
}

TEST_CASE("Can add multiple unordered elements") {
    auto tree = QuadTree<unsigned int>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(3.0f), 3)  //
    };
    tree.insert(elements);

    REQUIRE(tree.elements.size() == 3);
    REQUIRE(tree.root->parity == 0);
    REQUIRE(tree.root->left == nullptr);
    REQUIRE(tree.root->right == nullptr);
    REQUIRE(tree.root->begin->first == glm::vec3(1.0));
    REQUIRE(tree.root->begin->second == 1);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(3.0F));
}

TEST_CASE("Can retrieve nearest element") {
    auto tree = QuadTree<unsigned int>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0F), 3)  //
    };
    tree.insert(elements);

    auto result = tree.get(glm::vec3(1.75F));
    REQUIRE(result == 2);
}
