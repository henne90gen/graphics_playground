#include <gtest/gtest.h>
#include <vector>

#include "RubiksCube.h"

void assertWholeFace(rubiks::RubiksCube cube, Face face) {
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(face, i), face);
    }
}

TEST(RubiksCubeTest, face_does_not_change_without_a_rotation) {
    auto cube = rubiks::RubiksCube();
    assertWholeFace(cube, Face::FRONT);
    assertWholeFace(cube, Face::BACK);
    assertWholeFace(cube, Face::LEFT);
    assertWholeFace(cube, Face::RIGHT);
    assertWholeFace(cube, Face::UP);
    assertWholeFace(cube, Face::DOWN);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_one_rotation) {
    auto cube = rubiks::RubiksCube({R_R});
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 2), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 5), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 8), Face::DOWN);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 2), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 5), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 8), Face::FRONT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 0), Face::UP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 3), Face::UP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 6), Face::UP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 8), Face::BACK);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_two_rotations_1) {
    auto cube = rubiks::RubiksCube({R_R, R_U});
    cube.rotate(2.0f);
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 0), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 1), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 2), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 5), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 8), Face::DOWN);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 6), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 7), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 8), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 2), Face::UP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::UP, 5), Face::UP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 0), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 1), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 2), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 3), Face::UP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 6), Face::UP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::DOWN, 8), Face::BACK);
}
