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
