#include <gtest/gtest.h>
#include <vector>

#include "RubiksCube.h"

void assertWholeFace(rubiks::RubiksCube cube, Face face) {
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(cube.getCurrentFace(face, i), face);
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
    cube.rotateAll();

    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 2), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 5), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 8), Face::DOWN);

    ASSERT_EQ(cube.getCurrentFace(Face::UP, 2), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 5), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 8), Face::FRONT);

    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 0), Face::UP);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 3), Face::UP);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 6), Face::UP);

    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 8), Face::BACK);
}

TEST(RubiksCubeTest, Face_is_calculated_correctly_with_two_rotations_1) {
    auto cube = rubiks::RubiksCube({R_R, R_U});
    cube.rotateAll();

    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 0), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 1), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 2), Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 5), Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(Face::FRONT, 8), Face::DOWN);

    ASSERT_EQ(cube.getCurrentFace(Face::UP, 6), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 7), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 8), Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 2), Face::UP);
    ASSERT_EQ(cube.getCurrentFace(Face::UP, 5), Face::UP);

    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 0), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 1), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 2), Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 3), Face::UP);
    ASSERT_EQ(cube.getCurrentFace(Face::BACK, 6), Face::UP);

    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 2), Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 5), Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(Face::DOWN, 8), Face::BACK);
}

void testSolving(const std::vector<RotationCommand> &initialCommands, Face side, int localIndex, Face expectedFace) {
    auto cube = rubiks::RubiksCube(initialCommands);
    cube.rotateAll();

    cube.startSolving();
    cube.solve();

    cube.rotateAll();

    ASSERT_EQ(cube.getCurrentFace(side, localIndex), expectedFace);
}

TEST(RubiksCubeTest_Solve, BottomLayer_FrontTurnedUp) {
    testSolving({R_F, R_F}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_U}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_U, R_U}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_UI}, Face::DOWN, 1, Face::DOWN);
}

TEST(RubiksCubeTest_SolveBottomLayer, BottomLayer_RightTurnedUp) {
    testSolving({R_R, R_R}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_R, R_R, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_R, R_R, R_U, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_R, R_R, R_UI}, Face::DOWN, 5, Face::DOWN);
}

TEST(RubiksCubeTest_SolveBottomLayer, BottomLayer_BackTurnedUp) {
    testSolving({R_B, R_B}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_B, R_B, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_B, R_B, R_U, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_B, R_B, R_UI}, Face::DOWN, 5, Face::DOWN);
}

TEST(RubiksCubeTest_SolveBottomLayer, BottomLayer_LeftTurnedUp) {
    testSolving({R_L, R_L}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_L, R_L, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_L, R_L, R_U, R_U}, Face::DOWN, 5, Face::DOWN);
    testSolving({R_L, R_L, R_UI}, Face::DOWN, 5, Face::DOWN);
}
