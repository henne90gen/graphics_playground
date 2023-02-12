#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include "quad_tree/QuadTree.h"

TEST(QuadTreeTest, can_add_one_element) {
    auto tree = QuadTree<unsigned int>(10);

    auto point = glm::vec3(1.0F);
    unsigned int elem = 123;
    tree.insert(point, elem);

    ASSERT_EQ(tree.elements.size(), 1);
    ASSERT_EQ(tree.root->parity, 0);
    ASSERT_EQ(tree.root->left, nullptr);
    ASSERT_EQ(tree.root->right, nullptr);
    ASSERT_EQ(tree.root->begin->first, point);
    ASSERT_EQ(tree.root->begin->second, elem);
    ASSERT_EQ(tree.root->bb.min, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->bb.max, glm::vec3(1.0F));
}

TEST(QuadTreeTest, can_add_multiple_elements) {
    auto tree = QuadTree<unsigned int>(10);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0F), 3)  //
    };
    tree.insert(elements);

    ASSERT_EQ(tree.elements.size(), 3);
    ASSERT_EQ(tree.root->parity, 0);
    ASSERT_EQ(tree.root->left, nullptr);
    ASSERT_EQ(tree.root->right, nullptr);
    ASSERT_EQ(tree.root->begin->first, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->begin->second, 1);
    ASSERT_EQ(tree.root->bb.min, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->bb.max, glm::vec3(3.0F));
}

TEST(QuadTreeTest, can_add_multiple_unordered_elements) {
    auto tree = QuadTree<unsigned int>(10);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(3.0f), 3)  //
    };
    tree.insert(elements);

    ASSERT_EQ(tree.elements.size(), 3);
    ASSERT_EQ(tree.root->parity, 0);
    ASSERT_EQ(tree.root->left, nullptr);
    ASSERT_EQ(tree.root->right, nullptr);
    ASSERT_EQ(tree.root->begin->first, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->begin->second, 1);
    ASSERT_EQ(tree.root->bb.min, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->bb.max, glm::vec3(3.0F));
}

TEST(QuadTreeTest, branches_correctly) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(4.0f), 4), //
          std::make_pair(glm::vec3(3.0f), 3)  //
    };
    tree.insert(elements);

    ASSERT_EQ(tree.elements.size(), 4);
    ASSERT_EQ(tree.root->parity, 0);
    ASSERT_EQ(tree.root->begin->first, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->begin->second, 1);
    ASSERT_EQ(tree.root->bb.min, glm::vec3(1.0F));
    ASSERT_EQ(tree.root->bb.max, glm::vec3(4.0F));

    ASSERT_NE(tree.root->left, nullptr);
    ASSERT_EQ(tree.root->left->begin->first , glm::vec3(1.0F));
    ASSERT_EQ((tree.root->left->begin + 1)->first , glm::vec3(2.0F));
    ASSERT_EQ((tree.root->left->begin + 2) , tree.root->left->end);

    ASSERT_NE(tree.root->right, nullptr);
    ASSERT_EQ(tree.root->right->begin->first , glm::vec3(3.0F));
    ASSERT_EQ((tree.root->right->begin + 1)->first , glm::vec3(4.0F));
    ASSERT_EQ((tree.root->right->begin + 2) , tree.root->right->end);
}

TEST(QuadTreeTest, fails_if_there_are_no_elements) {
    auto tree = QuadTree<unsigned int>(10);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {};
    tree.insert(elements);

    unsigned int result = 0;
    ASSERT_TRUE(!tree.get(glm::vec3(1.75F), result));
    ASSERT_EQ(result , 0);
}

TEST(QuadTreeTest, can_retrieve_nearest_element) {
    auto tree = QuadTree<unsigned int>(10);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0F), 3)  //
    };
    tree.insert(elements);

    unsigned int result = 0;
    ASSERT_TRUE(tree.get(glm::vec3(1.75F), result));
    ASSERT_EQ(result , 2);
}

TEST(QuadTreeTest, can_retrieve_nearest_element_from_branched_tree) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    unsigned int result = 0;
    ASSERT_TRUE(tree.get(glm::vec3(1.75F), result));
    ASSERT_EQ(result , 2);
}

TEST(QuadTreeTest, can_retrieve_nearest_element_from_large_branched_tree) {
    auto tree = QuadTree<unsigned int>(2);

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
    ASSERT_TRUE(tree.get(glm::vec3(width / 2.0F, 0, height / 2.0F), result));
    ASSERT_EQ(result , 5050);
}

TEST(QuadTreeTest, can_retrieve_nearest_elements_from_branched_tree) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    std::vector<unsigned int> result = {};
    ASSERT_TRUE(tree.get(glm::vec3(0.0F), 2, result));
    ASSERT_EQ(result.size() , 2);
    ASSERT_EQ(result[0] , 1);
    ASSERT_EQ(result[1] , 2);
}

TEST(QuadTreeTest, can_travers_branched_tree_in_pre_order) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    int count = 0;
    tree.traversPreOrder([&count](QuadTree<unsigned int>::Node *node) {
        if (count == 0) {
            ASSERT_TRUE(!node->isLeaf());
        } else if (count == 1) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count == 2) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count > 2) {
            FAIL();
        }
        count++;
    });
    ASSERT_EQ(count , 3);
}

TEST(QuadTreeTest, can_travers_branched_tree_in_post_order) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    int count = 0;
    tree.traversPostOrder([&count](QuadTree<unsigned int>::Node *node) {
        if (count == 0) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count == 1) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count == 2) {
            ASSERT_TRUE(!node->isLeaf());
        } else if (count > 2) {
            FAIL();
        }
        count++;
    });
    ASSERT_EQ(count , 3);
}

TEST(QuadTreeTest, can_travers_branched_tree_in_inOrder) {
    auto tree = QuadTree<unsigned int>(2);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {
          std::make_pair(glm::vec3(1.0F), 1), //
          std::make_pair(glm::vec3(2.0F), 2), //
          std::make_pair(glm::vec3(3.0f), 3), //
          std::make_pair(glm::vec3(4.0f), 4)  //
    };
    tree.insert(elements);

    int count = 0;
    tree.traversInOrder([&count](QuadTree<unsigned int>::Node *node) {
        if (count == 0) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count == 1) {
            ASSERT_TRUE(!node->isLeaf());
        } else if (count == 2) {
            ASSERT_TRUE(node->isLeaf());
        } else if (count > 2) {
            FAIL();
        }
        count++;
    });
    ASSERT_EQ(count , 3);
}
