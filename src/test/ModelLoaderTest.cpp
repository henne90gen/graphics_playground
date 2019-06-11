#include <catch.hpp>
#include "model_loading/ModelLoader.h"

TEST_CASE("Not existing file is handled correctly") {
    auto result = ModelLoader::fromFile("not-existing.obj");
    REQUIRE(result.vertices.empty());
    REQUIRE(result.normals.empty());
    REQUIRE(result.textureCoordinates.empty());
    REQUIRE(result.indices.empty());
}

TEST_CASE("Empty file is handled correctly") {
    auto result = ModelLoader::fromFile("../../../src/test/empty.obj");
    REQUIRE(result.vertices.empty());
    REQUIRE(result.normals.empty());
    REQUIRE(result.textureCoordinates.empty());
    REQUIRE(result.indices.empty());
}

static void
assertListEquals(const std::string &message, std::vector<glm::vec3> &expected, std::vector<glm::vec3> &actual);

static void
assertListEquals(const std::string &message, std::vector<glm::ivec3> &expected, std::vector<glm::ivec3> &actual);

TEST_CASE("Simple cube model is loaded correctly") {
    auto model = ModelLoader::fromFile("../../../src/test/cube.obj");
    std::vector<glm::vec3> vertices = {
            {1.000000,  -1.000000, 1.000000},
            {-1.000000, -1.000000, -1.000000},
            {1.000000,  -1.000000, -1.000000},
            {-1.000000, 1.000000,  -1.000000},
            {1.000000,  1.000000,  1.000000},
            {1.000000,  1.000000,  -1.000000},
            {1.000000,  1.000000,  -1.000000},
            {1.000000,  -1.000000, 1.000000},
            {1.000000,  -1.000000, -1.000000},
            {1.000000,  1.000000,  1.000000},
            {-1.000000, -1.000000, 1.000000},
            {1.000000,  -1.000000, 1.000000},
            {-1.000000, -1.000000, 1.000000},
            {-1.000000, 1.000000,  -1.000000},
            {-1.000000, -1.000000, -1.000000},
            {1.000000,  -1.000000, -1.000000},
            {-1.000000, 1.000000,  -1.000000},
            {1.000000,  1.000000,  -1.000000},
            {-1.000000, -1.000000, 1.000000},
            {-1.000000, 1.000000,  1.000000},
            {1.000000,  1.000000,  1.000000},
            {-1.000000, 1.000000,  1.000000},
            {-1.000000, 1.000000,  1.000000},
            {-1.000000, -1.000000, -1.000000},
    };
    assertListEquals("Vertices", vertices, model.vertices);

    std::vector<glm::vec3> normals = {
            {0.0000,  -1.0000, 0.0000},
            {0.0000,  -1.0000, 0.0000},
            {0.0000,  -1.0000, 0.0000},
            {0.0000,  1.0000,  0.0000},
            {0.0000,  1.0000,  0.0000},
            {0.0000,  1.0000,  0.0000},
            {1.0000,  -0.0000, 0.0000},
            {1.0000,  -0.0000, 0.0000},
            {1.0000,  -0.0000, 0.0000},
            {0.0000,  -0.0000, 1.0000},
            {0.0000,  -0.0000, 1.0000},
            {0.0000,  -0.0000, 1.0000},
            {-1.0000, -0.0000, -0.0000},
            {-1.0000, -0.0000, -0.0000},
            {-1.0000, -0.0000, -0.0000},
            {0.0000,  0.0000,  -1.0000},
            {0.0000,  0.0000,  -1.0000},
            {0.0000,  0.0000,  -1.0000},
            {0.0000,  -1.0000, 0.0000},
            {0.0000,  1.0000,  0.0000},
            {1.0000,  -0.0000, 0.0000},
            {0.0000,  -0.0000, 1.0000},
            {-1.0000, -0.0000, -0.0000},
            {0.0000,  0.0000,  -1.0000},
    };
    assertListEquals("Normals", normals, model.normals);

    std::vector<glm::ivec3> indices = {
            {0,  1,  2},
            {3,  4,  5},
            {6,  7,  8},
            {9,  10, 11},
            {12, 13, 14},
            {15, 16, 17},
            {0,  18, 1},
            {3,  19, 4},
            {6,  20, 7},
            {9,  21, 10},
            {12, 22, 13},
            {15, 23, 16},
    };
    assertListEquals("Indices", indices, model.indices);

    REQUIRE(model.textureCoordinates.empty());
}

static void
assertListEquals(const std::string &message, std::vector<glm::vec3> &expected, std::vector<glm::vec3> &actual) {
    INFO(message);
    REQUIRE(expected.size() == actual.size());
    float margin = std::numeric_limits<float>::epsilon();
    for (unsigned int i = 0; i < expected.size(); i++) {
        INFO("i = " << i);
        REQUIRE(expected[i].x == Approx(actual[i].x).margin(margin));
        REQUIRE(expected[i].y == Approx(actual[i].y).margin(margin));
        REQUIRE(expected[i].z == Approx(actual[i].z).margin(margin));
    }
}

static void
assertListEquals(const std::string &message, std::vector<glm::ivec3> &expected, std::vector<glm::ivec3> &actual) {
    INFO(message);
    REQUIRE(expected.size() == actual.size());
    for (unsigned int i = 0; i < expected.size(); i++) {
        INFO("i = " << i);
        REQUIRE(expected[i].x == actual[i].x);
        REQUIRE(expected[i].y == actual[i].y);
        REQUIRE(expected[i].z == actual[i].z);
    }
}
