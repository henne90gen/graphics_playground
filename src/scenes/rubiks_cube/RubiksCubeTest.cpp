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
    assertWholeFace(cube, FRONT);
    assertWholeFace(cube, BACK);
    assertWholeFace(cube, LEFT);
    assertWholeFace(cube, RIGHT);
    assertWholeFace(cube, TOP);
    assertWholeFace(cube, BOTTOM);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_one_rotation) {
    auto cube = rubiks::RubiksCube({R_R});
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 2), BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 5), BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 8), BOTTOM);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 2), FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 5), FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 8), FRONT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 0), TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 3), TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 6), TOP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 2), BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 5), BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 8), BACK);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_two_rotations) {
    auto cube = rubiks::RubiksCube({R_R, R_T});
    cube.rotate(2.0f);
    cube.rotate(2.0f);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 2), BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 5), BOTTOM);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(FRONT, 8), RIGHT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 2), FRONT);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 5), TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(TOP, 8), TOP);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 0), TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 3), TOP);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BACK, 6), LEFT);

    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 2), BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 5), BACK);
    ASSERT_EQ(cube.getCurrentFaceAtLocalIndex(BOTTOM, 8), BACK);
}
