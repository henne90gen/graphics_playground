#include <gtest/gtest.h>
#include <vector>

#include "CoreRubiksCube.h"

void assertCubeIsInitialCube(const rubiks::CoreRubiksCube &actual) {
    const auto expected = rubiks::CoreRubiksCube();
    for (int i = 0; i < expected.globalIndexToCubeletIndex.size(); i++) {
        ASSERT_EQ(expected.globalIndexToCubeletIndex[i], actual.globalIndexToCubeletIndex[i]) << "where i=" << i;
    }

    for (int i = 0; i < expected.sideAndLocalIndexToFace.size(); i++) {
        for (int j = 0; j < expected.sideAndLocalIndexToFace[i].size(); j++) {
            ASSERT_EQ(expected.sideAndLocalIndexToFace[i][j], actual.sideAndLocalIndexToFace[i][j])
                  << "where i=" << i << " and j=" << j;
        }
    }
}

TEST(CoreRubiksCube, rotate_works_when_reversed) {
    std::array<Face, rubiks::SIDE_COUNT> faces = {
          Face::FRONT, Face::BACK, Face::LEFT, Face::RIGHT, Face::UP, Face::DOWN,
    };

    // test all rotations and their back rotation
    for (int i = 0; i < faces.size(); i++) {
        std::vector<RotationCommand> commands = {
              {faces[i], Direction::CLOCKWISE},
              {faces[i], Direction::COUNTER_CLOCKWISE},
        };
        assertCubeIsInitialCube(rubiks::CoreRubiksCube(commands));
    }

    // test all combinations of two rotations and their back rotation
    for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j < faces.size(); j++) {
            std::vector<RotationCommand> commands = {
                  {faces[i], Direction::CLOCKWISE},
                  {faces[j], Direction::CLOCKWISE},
                  {faces[j], Direction::COUNTER_CLOCKWISE},
                  {faces[i], Direction::COUNTER_CLOCKWISE},
            };
            assertCubeIsInitialCube(rubiks::CoreRubiksCube(commands));
        }
    }
}

TEST(CoreRubiksCube, Face_is_calculated_correctly_with_one_rotation) {
    auto cube = rubiks::CoreRubiksCube({R_R});

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

TEST(CoreRubiksCube, Face_is_calculated_correctly_with_two_rotations_1) {
    auto cube = rubiks::CoreRubiksCube({R_R, R_U});

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
    auto cube = rubiks::CoreRubiksCube(initialCommands);
    cube.solve();
    ASSERT_EQ(cube.getCurrentFace(side, localIndex), expectedFace);
}

TEST(CoreRubiksCube, BottomLayer_FrontTurnedUp) {
    testSolving({R_F, R_F}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_U}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_U, R_U}, Face::DOWN, 1, Face::DOWN);
    testSolving({R_F, R_F, R_UI}, Face::DOWN, 1, Face::DOWN);
}
