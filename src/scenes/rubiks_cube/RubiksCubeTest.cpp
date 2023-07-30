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
    assertWholeFace(cube, Face::TOP);
    assertWholeFace(cube, Face::BOTTOM);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_one_rotation) {
    auto cube = rubiks::RubiksCube({R_R});
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 2), Face::BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 5), Face::BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 8), Face::BOTTOM);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 2), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 5), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 8), Face::FRONT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 0), Face::TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 3), Face::TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 6), Face::TOP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 8), Face::BACK);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_two_rotations) {
    auto cube = rubiks::RubiksCube({R_R, R_T});
    cube.rotate(2.0f);
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 2), Face::BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 5), Face::BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::FRONT, 8), Face::RIGHT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 2), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 5), Face::TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::TOP, 8), Face::TOP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 0), Face::TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 3), Face::TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BACK, 6), Face::LEFT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(Face::BOTTOM, 8), Face::BACK);
}
