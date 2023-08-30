#include <gtest/gtest.h>
#include <random>
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

TEST(getCurrentFace, Face_is_calculated_correctly_for_R) {
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

TEST(getCurrentFace, Face_is_calculated_correctly_for_R_U) {
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

TEST(getCurrentFace, Face_is_calculated_correctly_for_F_L) {
    auto cube = rubiks::CoreRubiksCube({R_F, R_L});

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 0), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 3), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 6), rubiks::Face::LEFT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 0), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 3), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 6), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 7), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 8), rubiks::Face::LEFT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 2), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 5), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 8), rubiks::Face::RIGHT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 6), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 7), rubiks::Face::DOWN);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 8), rubiks::Face::DOWN);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 0), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 1), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 2), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 3), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, 6), rubiks::Face::FRONT);
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

void testSolving(const std::vector<rubiks::RotationCommand> &initialCommands) {
    auto cube = rubiks::CoreRubiksCube(initialCommands);
    auto commands = cube.solve();
    for (const auto &cmd : commands) {
        ASSERT_NE(cmd.side, rubiks::Face::NONE);
    }

    // bottom side
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(cube.getCurrentFace(rubiks::Face::DOWN, i), rubiks::Face::DOWN);
    }

    // bottom layer
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 6), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 7), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 8), rubiks::Face::FRONT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 6), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 7), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 8), rubiks::Face::RIGHT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 6), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 7), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 8), rubiks::Face::BACK);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 6), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 7), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 8), rubiks::Face::LEFT);

    // middle layer
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 3), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 5), rubiks::Face::FRONT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 3), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 5), rubiks::Face::LEFT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 3), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 5), rubiks::Face::RIGHT);

    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 3), rubiks::Face::BACK);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 5), rubiks::Face::BACK);

    // top cross
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 1), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 3), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 5), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 7), rubiks::Face::UP);

    // sides of top cross
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::FRONT, 1), rubiks::Face::FRONT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::LEFT, 1), rubiks::Face::LEFT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::RIGHT, 1), rubiks::Face::RIGHT);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::BACK, 1), rubiks::Face::BACK);

    // top corners
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 0), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 2), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 6), rubiks::Face::UP);
    ASSERT_EQ(cube.getCurrentFace(rubiks::Face::UP, 8), rubiks::Face::UP);

    // TODO find out why the final result is not equal to the initial cube
    // assertCubeIsInitialCube(cube);
}

TEST(solve, BottomLayer_FrontRotations) {
    testSolving({R_F});
    testSolving({R_F, R_F});
    testSolving({R_FI});
}

TEST(solve, BottomLayer_LeftRotations) {
    testSolving({R_L});
    testSolving({R_L, R_L});
    testSolving({R_LI});
}

TEST(solve, BottomLayer_BackRotations) {
    testSolving({R_B});
    testSolving({R_B, R_B});
    testSolving({R_BI});
}

TEST(solve, BottomLayer_RightRotations) {
    testSolving({R_R});
    testSolving({R_R, R_R});
    testSolving({R_RI});
}

TEST(solve, BottomLayer_FrontAndUpRotations) {
    testSolving({R_F, R_F, R_U});
    testSolving({R_F, R_F, R_U, R_U});
    testSolving({R_F, R_F, R_UI});
}

TEST(solve, BottomLayer_LeftBackAndRightRotations) {
    testSolving({R_F, R_F, R_U, R_L});
    testSolving({R_F, R_F, R_U, R_LI});

    testSolving({R_F, R_F, R_U, R_U, R_B});
    testSolving({R_F, R_F, R_U, R_U, R_BI});

    testSolving({R_F, R_F, R_UI, R_R});
    testSolving({R_F, R_F, R_UI, R_RI});
}

TEST(solve, BottomLayer_PieceAtBottomLayerButWrongPosition) {
    testSolving({R_F, R_L});
    testSolving({R_F, R_F, R_U, R_L, R_L});
}

TEST(solve, RegressionTests) {
    testSolving({R_D,  R_D,  R_L, R_DI, R_FI, R_LI, R_UI, R_UI, R_BI, R_R,
                 R_FI, R_FI, R_R, R_D,  R_F,  R_L,  R_D,  R_FI, R_UI, R_R});
    testSolving({R_B,  R_FI, R_B, R_RI, R_RI, R_RI, R_RI, R_UI, R_BI, R_UI,
                 R_RI, R_U,  R_L, R_LI, R_FI, R_F,  R_UI, R_L,  R_U,  R_UI});
    testSolving({R_R,  R_LI, R_D,  R_BI, R_RI, R_F, R_RI, R_FI, R_R,  R_UI,
                 R_BI, R_F,  R_UI, R_B,  R_FI, R_U, R_L,  R_FI, R_DI, R_R});
}

TEST(solve, DISABLED_RandomMoves) {
    constexpr auto iterationCount = 1000;
    constexpr int shuffleCount = 20;

    for (int i = 0; i < iterationCount; i++) {
        std::vector<rubiks::RotationCommand> commands = {};
        static constexpr auto rotationCommandCount = 12;
        std::array<rubiks::RotationCommand, rotationCommandCount> rotations = {
              rubiks::RotationCommand(R_R), R_RI, R_F, R_FI, R_D, R_DI, R_L, R_LI, R_U, R_UI, R_B, R_BI,
        };

        const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distribution(0, rotations.size() - 1);

        for (unsigned int i = 0; i < shuffleCount; i++) {
            unsigned int randomIndex = distribution(generator);
            rubiks::RotationCommand rotation = rotations[randomIndex];
            commands.push_back(rotation);

            std::cout << to_string(rotation, true) << ", ";
        }
        std::cout << std::endl;

        testSolving(commands);
    }
}

namespace rubiks {
std::array<std::pair<rubiks::Face, unsigned int>, 3> getCornerPartners(rubiks::Face side, unsigned int localIndex);
}
TEST(getCornerPartners, works) {
    auto result = rubiks::getCornerPartners(rubiks::Face::FRONT, 0);
    ASSERT_EQ(result[0].first, rubiks::Face::FRONT);
    ASSERT_EQ(result[0].second, 0);
    ASSERT_EQ(result[1].first, rubiks::Face::LEFT);
    ASSERT_EQ(result[1].second, 2);
    ASSERT_EQ(result[2].first, rubiks::Face::UP);
    ASSERT_EQ(result[2].second, 6);

    result = rubiks::getCornerPartners(rubiks::Face::LEFT, 2);
    ASSERT_EQ(result[0].first, rubiks::Face::LEFT);
    ASSERT_EQ(result[0].second, 2);
    ASSERT_EQ(result[1].first, rubiks::Face::UP);
    ASSERT_EQ(result[1].second, 6);
    ASSERT_EQ(result[2].first, rubiks::Face::FRONT);
    ASSERT_EQ(result[2].second, 0);

    result = rubiks::getCornerPartners(rubiks::Face::UP, 6);
    ASSERT_EQ(result[0].first, rubiks::Face::UP);
    ASSERT_EQ(result[0].second, 6);
    ASSERT_EQ(result[1].first, rubiks::Face::FRONT);
    ASSERT_EQ(result[1].second, 0);
    ASSERT_EQ(result[2].first, rubiks::Face::LEFT);
    ASSERT_EQ(result[2].second, 2);
}
