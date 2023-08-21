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

TEST(rotate, works_when_reversed) {
    std::array<rubiks::Face, rubiks::SIDE_COUNT> faces = {
          rubiks::Face::FRONT, rubiks::Face::BACK, rubiks::Face::LEFT,
          rubiks::Face::RIGHT, rubiks::Face::UP,   rubiks::Face::DOWN,
    };

    // test all rotations and their back rotation
    for (int i = 0; i < faces.size(); i++) {
        std::vector<rubiks::RotationCommand> commands = {
              {faces[i], rubiks::Direction::CLOCKWISE},
              {faces[i], rubiks::Direction::COUNTER_CLOCKWISE},
        };
        assertCubeIsInitialCube(rubiks::CoreRubiksCube(commands));
    }

    // test all combinations of two rotations and their back rotation
    for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j < faces.size(); j++) {
            std::vector<rubiks::RotationCommand> commands = {
                  {faces[i], rubiks::Direction::CLOCKWISE},
                  {faces[j], rubiks::Direction::CLOCKWISE},
                  {faces[j], rubiks::Direction::COUNTER_CLOCKWISE},
                  {faces[i], rubiks::Direction::COUNTER_CLOCKWISE},
            };
            assertCubeIsInitialCube(rubiks::CoreRubiksCube(commands));
        }
    }
}

void assertWholeFace(rubiks::CoreRubiksCube cube, rubiks::Face face) {
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(cube.getCurrentFace(face, i), face);
    }
}

TEST(getCurrentFace, Face_does_not_change_without_a_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    assertWholeFace(cube, rubiks::Face::FRONT);
    assertWholeFace(cube, rubiks::Face::BACK);
    assertWholeFace(cube, rubiks::Face::LEFT);
    assertWholeFace(cube, rubiks::Face::RIGHT);
    assertWholeFace(cube, rubiks::Face::UP);
    assertWholeFace(cube, rubiks::Face::DOWN);
}

TEST(getCurrentFace, Face_is_calculated_correctly_with_one_rotation) {
    auto cube = rubiks::CoreRubiksCube({R_R});

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 2), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 5), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 8), rubiks::Face::DOWN);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 2), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 5), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 8), rubiks::Face::FRONT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 0), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 3), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 6), rubiks::Face::UP);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 2), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 5), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 8), rubiks::Face::BACK);
}

TEST(getCurrentFace, Face_is_calculated_correctly_with_two_rotations_1) {
    auto cube = rubiks::CoreRubiksCube({R_R, R_U});

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 0), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 1), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 2), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 5), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 8), rubiks::Face::DOWN);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 6), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 7), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 8), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 2), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 5), rubiks::Face::UP);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 0), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 1), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 2), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 3), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 6), rubiks::Face::UP);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 2), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 5), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 8), rubiks::Face::BACK);
}

TEST(rotate, indices_are_adjusted_correctly_for_counter_clockwise_front_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_FI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[18], 24);
    ASSERT_EQ(indices[19], 21);
    ASSERT_EQ(indices[20], 18);
    ASSERT_EQ(indices[21], 25);
    ASSERT_EQ(indices[23], 19);
    ASSERT_EQ(indices[24], 26);
    ASSERT_EQ(indices[25], 23);
    ASSERT_EQ(indices[26], 20);
}

TEST(rotate, Indices_are_adjusted_correctly_for_counter_clockwise_back_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_BI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[0], 2);
    ASSERT_EQ(indices[1], 5);
    ASSERT_EQ(indices[2], 8);
    ASSERT_EQ(indices[3], 1);
    ASSERT_EQ(indices[5], 7);
    ASSERT_EQ(indices[6], 0);
    ASSERT_EQ(indices[7], 3);
    ASSERT_EQ(indices[8], 6);
}

TEST(rotate, Indices_are_adjusted_correctly_for_counter_clockwise_left_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_LI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[18], 0);
    ASSERT_EQ(indices[9], 3);
    ASSERT_EQ(indices[0], 6);
    ASSERT_EQ(indices[21], 9);
    ASSERT_EQ(indices[3], 15);
    ASSERT_EQ(indices[24], 18);
    ASSERT_EQ(indices[15], 21);
    ASSERT_EQ(indices[6], 24);
}

TEST(rotate, Indices_are_adjusted_correctly_for_counter_clockwise_right_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_RI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[26], 8);
    ASSERT_EQ(indices[17], 5);
    ASSERT_EQ(indices[8], 2);
    ASSERT_EQ(indices[23], 17);
    ASSERT_EQ(indices[5], 11);
    ASSERT_EQ(indices[20], 26);
    ASSERT_EQ(indices[11], 23);
    ASSERT_EQ(indices[2], 20);
}

TEST(rotate, Indices_are_adjusted_correctly_for_counter_clockwise_up_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_UI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[6], 8);
    ASSERT_EQ(indices[7], 17);
    ASSERT_EQ(indices[8], 26);
    ASSERT_EQ(indices[15], 7);
    ASSERT_EQ(indices[17], 25);
    ASSERT_EQ(indices[24], 6);
    ASSERT_EQ(indices[25], 15);
    ASSERT_EQ(indices[26], 24);
}

TEST(rotate, Indices_are_adjusted_correctly_for_two_counter_clockwise_down_rotations) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_DI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[0], 18);
    ASSERT_EQ(indices[1], 9);
    ASSERT_EQ(indices[2], 0);
    ASSERT_EQ(indices[11], 1);
    ASSERT_EQ(indices[20], 2);
    ASSERT_EQ(indices[19], 11);
    ASSERT_EQ(indices[18], 20);
    ASSERT_EQ(indices[9], 19);

    cube.rotate(R_DI);

    ASSERT_EQ(indices[0], 20);
    ASSERT_EQ(indices[1], 19);
    ASSERT_EQ(indices[2], 18);
    ASSERT_EQ(indices[11], 9);
    ASSERT_EQ(indices[20], 0);
    ASSERT_EQ(indices[19], 1);
    ASSERT_EQ(indices[18], 2);
    ASSERT_EQ(indices[9], 11);
}

TEST(rotate, Indices_are_adjusted_correctly_for_two_clockwise_down_rotations) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_D);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[0], 2);
    ASSERT_EQ(indices[1], 11);
    ASSERT_EQ(indices[2], 20);
    ASSERT_EQ(indices[11], 19);
    ASSERT_EQ(indices[20], 18);
    ASSERT_EQ(indices[19], 9);
    ASSERT_EQ(indices[18], 0);
    ASSERT_EQ(indices[9], 1);

    cube.rotate(R_D);

    ASSERT_EQ(indices[0], 20);
    ASSERT_EQ(indices[1], 19);
    ASSERT_EQ(indices[2], 18);
    ASSERT_EQ(indices[11], 9);
    ASSERT_EQ(indices[20], 0);
    ASSERT_EQ(indices[19], 1);
    ASSERT_EQ(indices[18], 2);
    ASSERT_EQ(indices[9], 11);
}

TEST(rotate, IndicesAdjustment_RI) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_RI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[20], 26);
    ASSERT_EQ(indices[2], 20);
    ASSERT_EQ(indices[8], 2);
    ASSERT_EQ(indices[26], 8);
    ASSERT_EQ(indices[11], 23);
    ASSERT_EQ(indices[5], 11);
    ASSERT_EQ(indices[17], 5);
    ASSERT_EQ(indices[23], 17);
}

TEST(rotate, IndicesAdjustment_RI_DI) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_RI);
    cube.rotate(R_DI);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[18], 26);
    ASSERT_EQ(indices[19], 23);
    ASSERT_EQ(indices[20], 20);
    ASSERT_EQ(indices[11], 1);
    ASSERT_EQ(indices[2], 0);
    ASSERT_EQ(indices[9], 19);
    ASSERT_EQ(indices[0], 18);
    ASSERT_EQ(indices[1], 9);
}

TEST(rotate, IndicesAdjustment_RI_DI_R) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_RI);
    cube.rotate(R_DI);
    cube.rotate(R_R);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[23], 1);
    ASSERT_EQ(indices[20], 0);
    ASSERT_EQ(indices[11], 11);
    ASSERT_EQ(indices[2], 2);
    ASSERT_EQ(indices[5], 5);
    ASSERT_EQ(indices[8], 8);
    ASSERT_EQ(indices[17], 17);
    ASSERT_EQ(indices[26], 20);
}

TEST(rotate, Indices_are_adjusted_correctly_for_cw_down_and_cw_right_rotation) {
    auto cube = rubiks::CoreRubiksCube();
    cube.rotate(R_D);

    auto &indices = cube.globalIndexToCubeletIndex;
    ASSERT_EQ(indices[18], 0);
    ASSERT_EQ(indices[19], 9);
    ASSERT_EQ(indices[20], 18);
    ASSERT_EQ(indices[9], 1);
    ASSERT_EQ(indices[10], 10);
    ASSERT_EQ(indices[11], 19);
    ASSERT_EQ(indices[0], 2);
    ASSERT_EQ(indices[1], 11);
    ASSERT_EQ(indices[2], 20);

    cube.rotate(R_R);

    ASSERT_EQ(indices[20], 20);
    ASSERT_EQ(indices[23], 19);
    ASSERT_EQ(indices[26], 18);
    ASSERT_EQ(indices[11], 5);
    ASSERT_EQ(indices[2], 8);
    ASSERT_EQ(indices[5], 17);
    ASSERT_EQ(indices[8], 26);
    ASSERT_EQ(indices[17], 23);
}

void testSolving(const std::vector<rubiks::RotationCommand> &initialCommands, rubiks::Face side, int localIndex,
                 rubiks::Face expectedFace) {
    auto cube = rubiks::CoreRubiksCube(initialCommands);
    auto commands = cube.solve();
    for (const auto &cmd : commands) {
        ASSERT_NE(cmd.side, rubiks::Face::NONE);
    }
    ASSERT_EQ(cube.getCurrentFace(side, localIndex), expectedFace);
}

TEST(solve, BottomLayer_FrontRotations) {
    testSolving({R_F}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_FI}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_LeftRotations) {
    testSolving({R_L}, rubiks::Face::DOWN, 3, rubiks::Face::DOWN);
    testSolving({R_L, R_L}, rubiks::Face::DOWN, 3, rubiks::Face::DOWN);
    testSolving({R_LI}, rubiks::Face::DOWN, 3, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_BackRotations) {
    testSolving({R_B}, rubiks::Face::DOWN, 7, rubiks::Face::DOWN);
    testSolving({R_B, R_B}, rubiks::Face::DOWN, 7, rubiks::Face::DOWN);
    testSolving({R_BI}, rubiks::Face::DOWN, 7, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_RightRotations) {
    testSolving({R_R}, rubiks::Face::DOWN, 5, rubiks::Face::DOWN);
    testSolving({R_R, R_R}, rubiks::Face::DOWN, 5, rubiks::Face::DOWN);
    testSolving({R_RI}, rubiks::Face::DOWN, 5, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_FrontAndUpRotations) {
    testSolving({R_F, R_F, R_U}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_U, R_U}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_UI}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_LeftBackAndRightRotations) {
    testSolving({R_F, R_F, R_U, R_L}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_U, R_LI}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);

    testSolving({R_F, R_F, R_U, R_U, R_B}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_U, R_U, R_BI}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);

    testSolving({R_F, R_F, R_UI, R_R}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_UI, R_RI}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
}

TEST(solve, BottomLayer_PieceAtBottomLayerButWrongPosition) {
    testSolving({R_F, R_L}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
    testSolving({R_F, R_F, R_U, R_L, R_L}, rubiks::Face::DOWN, 1, rubiks::Face::DOWN);
}
