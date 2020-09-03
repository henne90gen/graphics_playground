#include <catch.hpp>
#include <glm/glm.hpp>

#include "quad_tree/QuadTree.h"

TEST_CASE("Can add one element") {
    auto tree = QuadTree<unsigned int, 10>();

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
    auto tree = QuadTree<unsigned int, 10>();

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
    REQUIRE(tree.root->begin->first == glm::vec3(1.0F));
    REQUIRE(tree.root->begin->second == 1);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(3.0F));
}

TEST_CASE("Can add multiple unordered elements") {
    auto tree = QuadTree<unsigned int, 10>();

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
    REQUIRE(tree.root->begin->first == glm::vec3(1.0F));
    REQUIRE(tree.root->begin->second == 1);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(3.0F));
}

TEST_CASE("Branches correctly") {
    auto tree = QuadTree<unsigned int, 2>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(4.0f), 4), //
          std::make_pair(glm::vec3(3.0f), 3)  //
    };
    tree.insert(elements);

    REQUIRE(tree.elements.size() == 4);
    REQUIRE(tree.root->parity == 0);
    REQUIRE(tree.root->begin->first == glm::vec3(1.0F));
    REQUIRE(tree.root->begin->second == 1);
    REQUIRE(tree.root->bb.min == glm::vec3(1.0F));
    REQUIRE(tree.root->bb.max == glm::vec3(4.0F));

    REQUIRE(tree.root->left != nullptr);
    REQUIRE(tree.root->left->begin->first == glm::vec3(1.0F));
    REQUIRE((tree.root->left->begin + 1)->first == glm::vec3(2.0F));
    REQUIRE((tree.root->left->begin + 2) == tree.root->left->end);

    REQUIRE(tree.root->right != nullptr);
    REQUIRE(tree.root->right->begin->first == glm::vec3(3.0F));
    REQUIRE((tree.root->right->begin + 1)->first == glm::vec3(4.0F));
    REQUIRE((tree.root->right->begin + 2) == tree.root->right->end);
}

TEST_CASE("Fails, if there are no elements") {
    auto tree = QuadTree<unsigned int, 10>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {};
    tree.insert(elements);

    unsigned int result = 0;
    REQUIRE(!tree.get(glm::vec3(1.75F), result));
    REQUIRE(result == 0);
}

TEST_CASE("Can retrieve nearest element") {
    auto tree = QuadTree<unsigned int, 10>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0F), 3)  //
    };
    tree.insert(elements);

    unsigned int result = 0;
    REQUIRE(tree.get(glm::vec3(1.75F), result));
    REQUIRE(result == 2);
}

TEST_CASE("Can retrieve nearest element from branched tree") {
    auto tree = QuadTree<unsigned int, 2>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    unsigned int result = 0;
    REQUIRE(tree.get(glm::vec3(1.75F), result));
    REQUIRE(result == 2);
}

TEST_CASE("Can retrieve nearest element from large branched tree") {
    auto tree = QuadTree<unsigned int, 2>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {};
    const unsigned int width = 100;
    const unsigned int height = 100;
    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            elements.push_back(std::make_pair(glm::vec3(x, 0, z), x * height + z));
        }
    }
    tree.insert(elements);

    unsigned int result = 0;
    REQUIRE(tree.get(glm::vec3(width / 2.0F, 0, height / 2.0F), result));
    REQUIRE(result == 5050);
}

TEST_CASE("Can retrieve nearest elements from branched tree") {
    auto tree = QuadTree<unsigned int, 2>();

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    std::vector<unsigned int> result = {};
    REQUIRE(tree.get(glm::vec3(0.0F), 2, result));
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 2);
}
