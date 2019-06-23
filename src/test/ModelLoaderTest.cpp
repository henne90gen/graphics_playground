#include <catch.hpp>
#include "model_loading/ModelLoader.h"

TEST_CASE("Not existing file is handled correctly") {
    std::shared_ptr<ModelLoader::RawModel> model = std::make_shared<ModelLoader::RawModel>();
    auto result = ModelLoader::fromFile("not-existing.obj", model);
    REQUIRE(result == 1);
    REQUIRE(model->meshes.empty());
    REQUIRE(model->materials.empty());
}

TEST_CASE("Empty file is handled correctly") {
    std::shared_ptr<ModelLoader::RawModel> model = std::make_shared<ModelLoader::RawModel>();
    auto result = ModelLoader::fromFile("../../../src/test/empty.obj", model);
    REQUIRE(result == 1);
    REQUIRE(model->meshes.empty());
    REQUIRE(model->materials.empty());
}

static void
assertListEquals(const std::string &message, std::vector<glm::vec3> &expected, std::vector<glm::vec3> &actual);

static void
assertListEquals(const std::string &message, std::vector<glm::ivec3> &expected, std::vector<glm::ivec3> &actual);

TEST_CASE("Simple cube model is loaded correctly") {
    std::shared_ptr<ModelLoader::RawModel> model = std::make_shared<ModelLoader::RawModel>();
    auto error = ModelLoader::fromFile("../../../src/test/cube.obj", model);
    REQUIRE(error == 0);
    REQUIRE(model->meshes.size() == 1);
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
            {1.000000,  -1.000000, 1.000000},
            {-1.000000, -1.000000, 1.000000},
            {-1.000000, -1.000000, -1.000000},
            {-1.000000, 1.000000,  -1.000000},
            {-1.000000, 1.000000,  1.000000},
            {1.000000,  1.000000,  1.000000},
            {1.000000,  1.000000,  -1.000000},
            {1.000000,  1.000000,  1.000000},
            {1.000000,  -1.000000, 1.000000},
            {1.000000,  1.000000,  1.000000},
            {-1.000000, 1.000000,  1.000000},
            {-1.000000, -1.000000, 1.000000},
            {-1.000000, -1.000000, 1.000000},
            {-1.000000, 1.000000,  1.000000},
            {-1.000000, 1.000000,  -1.000000},
            {1.000000,  -1.000000, -1.000000},
            {-1.000000, -1.000000, -1.000000},
            {-1.000000, 1.000000,  -1.000000},
    };
    assertListEquals("Vertices", vertices, model->meshes[0].vertices);

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
    };
    assertListEquals("Normals", normals, model->meshes[0].normals);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2},
            {3, 4, 5},
            {6, 7, 8},
            {9, 10, 11},
            {12, 13, 14},
            {15, 16, 17},
            {18, 19, 20},
            {21, 22, 23},
            {24, 25, 26},
            {27, 28, 29},
            {30, 31, 32},
            {33, 34, 35},
    };
    assertListEquals("Indices", indices, model->meshes[0].indices);

    REQUIRE(model->meshes[0].textureCoordinates.empty());
}

#include <iostream>

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
