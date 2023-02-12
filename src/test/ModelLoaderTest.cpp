#include <gtest/gtest.h>

#include "Model.h"

TEST(ModelLoaderTest, not_existing_file_is_handled_correctly) {
    RawModel model;
    auto result = Model::loadRawModelFromFile("not-existing.obj", model);
    ASSERT_EQ(result, 1);
    ASSERT_TRUE(model.meshes.empty());
    ASSERT_TRUE(model.materials.empty());
}

TEST(ModelLoaderTest, empty_file_is_handled_correctly) {
    RawModel model;
    auto result = Model::loadRawModelFromFile("../../src/test/empty.obj", model);
    ASSERT_EQ(result, 1);
    ASSERT_TRUE(model.meshes.empty());
    ASSERT_TRUE(model.materials.empty());
}

static void assertListEquals(const std::string &message, std::vector<glm::vec3> &expected,
                             std::vector<glm::vec3> &actual);

static void assertListEquals(const std::string &message, std::vector<glm::ivec3> &expected,
                             std::vector<glm::ivec3> &actual);

TEST(ModelLoaderTest, simple_cube_model_is_loaded_correctly) {
    RawModel model;
    auto error = Model::loadRawModelFromFile("../../../src/test/cube.obj", model);
    ASSERT_EQ(error, 0);
    ASSERT_EQ(model.meshes.size(), 1);
    std::vector<glm::vec3> vertices = {
          {1.000000, -1.000000, 1.000000},  {-1.000000, -1.000000, -1.000000}, {1.000000, -1.000000, -1.000000},
          {-1.000000, 1.000000, -1.000000}, {1.000000, 1.000000, 1.000000},    {1.000000, 1.000000, -1.000000},
          {1.000000, 1.000000, -1.000000},  {1.000000, -1.000000, 1.000000},   {1.000000, -1.000000, -1.000000},
          {1.000000, 1.000000, 1.000000},   {-1.000000, -1.000000, 1.000000},  {1.000000, -1.000000, 1.000000},
          {-1.000000, -1.000000, 1.000000}, {-1.000000, 1.000000, -1.000000},  {-1.000000, -1.000000, -1.000000},
          {1.000000, -1.000000, -1.000000}, {-1.000000, 1.000000, -1.000000},  {1.000000, 1.000000, -1.000000},
          {1.000000, -1.000000, 1.000000},  {-1.000000, -1.000000, 1.000000},  {-1.000000, -1.000000, -1.000000},
          {-1.000000, 1.000000, -1.000000}, {-1.000000, 1.000000, 1.000000},   {1.000000, 1.000000, 1.000000},
          {1.000000, 1.000000, -1.000000},  {1.000000, 1.000000, 1.000000},    {1.000000, -1.000000, 1.000000},
          {1.000000, 1.000000, 1.000000},   {-1.000000, 1.000000, 1.000000},   {-1.000000, -1.000000, 1.000000},
          {-1.000000, -1.000000, 1.000000}, {-1.000000, 1.000000, 1.000000},   {-1.000000, 1.000000, -1.000000},
          {1.000000, -1.000000, -1.000000}, {-1.000000, -1.000000, -1.000000}, {-1.000000, 1.000000, -1.000000},
    };
    assertListEquals("Vertices", vertices, model.meshes[0].vertices);

    std::vector<glm::vec3> normals = {
          {0.0000, -1.0000, 0.0000},   {0.0000, -1.0000, 0.0000},   {0.0000, -1.0000, 0.0000},
          {0.0000, 1.0000, 0.0000},    {0.0000, 1.0000, 0.0000},    {0.0000, 1.0000, 0.0000},
          {1.0000, -0.0000, 0.0000},   {1.0000, -0.0000, 0.0000},   {1.0000, -0.0000, 0.0000},
          {0.0000, -0.0000, 1.0000},   {0.0000, -0.0000, 1.0000},   {0.0000, -0.0000, 1.0000},
          {-1.0000, -0.0000, -0.0000}, {-1.0000, -0.0000, -0.0000}, {-1.0000, -0.0000, -0.0000},
          {0.0000, 0.0000, -1.0000},   {0.0000, 0.0000, -1.0000},   {0.0000, 0.0000, -1.0000},
          {0.0000, -1.0000, 0.0000},   {0.0000, -1.0000, 0.0000},   {0.0000, -1.0000, 0.0000},
          {0.0000, 1.0000, 0.0000},    {0.0000, 1.0000, 0.0000},    {0.0000, 1.0000, 0.0000},
          {1.0000, -0.0000, 0.0000},   {1.0000, -0.0000, 0.0000},   {1.0000, -0.0000, 0.0000},
          {0.0000, -0.0000, 1.0000},   {0.0000, -0.0000, 1.0000},   {0.0000, -0.0000, 1.0000},
          {-1.0000, -0.0000, -0.0000}, {-1.0000, -0.0000, -0.0000}, {-1.0000, -0.0000, -0.0000},
          {0.0000, 0.0000, -1.0000},   {0.0000, 0.0000, -1.0000},   {0.0000, 0.0000, -1.0000},
    };
    assertListEquals("Normals", normals, model.meshes[0].normals);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2},    {3, 4, 5},    {6, 7, 8},    {9, 10, 11},  {12, 13, 14}, {15, 16, 17},
          {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}, {30, 31, 32}, {33, 34, 35},
    };
    assertListEquals("Indices", indices, model.meshes[0].indices);

    ASSERT_TRUE(model.meshes[0].uvs.empty());
}

static void assertListEquals(const std::string &message, std::vector<glm::vec3> &expected,
                             std::vector<glm::vec3> &actual) {
    ASSERT_EQ(expected.size(), actual.size()) << message;
    for (unsigned long i = 0; i < expected.size(); i++) {
        ASSERT_FLOAT_EQ(expected[i].x, actual[i].x) << "i = " << i;
        ASSERT_FLOAT_EQ(expected[i].y, actual[i].y) << "i = " << i;
        ASSERT_FLOAT_EQ(expected[i].z, actual[i].z) << "i = " << i;
    }
}

static void assertListEquals(const std::string &message, std::vector<glm::ivec3> &expected,
                             std::vector<glm::ivec3> &actual) {
    ASSERT_EQ(expected.size(), actual.size()) << message;
    for (unsigned long i = 0; i < expected.size(); i++) {
        ASSERT_EQ(expected[i].x, actual[i].x) << "i = " << i;
        ASSERT_EQ(expected[i].y, actual[i].y) << "i = " << i;
        ASSERT_EQ(expected[i].z, actual[i].z) << "i = " << i;
    }
}
