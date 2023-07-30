#include <gtest/gtest.h>

#include <vector>

#include "RubiksCubeLogic.h"

void initCubePositions(std::vector<unsigned int> &cubePosition) {
    for (int i = 0; i < 27; i++) {
        cubePosition.push_back(i);
    }
}

TEST(RubiksCubeLogicTest, indices_are_adjusted_correctly_for_counter_clockwise_front_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = FRONT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[18], 24);
    ASSERT_EQ(cubePositions[19], 21);
    ASSERT_EQ(cubePositions[20], 18);
    ASSERT_EQ(cubePositions[21], 25);
    ASSERT_EQ(cubePositions[23], 19);
    ASSERT_EQ(cubePositions[24], 26);
    ASSERT_EQ(cubePositions[25], 23);
    ASSERT_EQ(cubePositions[26], 20);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_counter_clockwise_back_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = BACK_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[0], 2);
    ASSERT_EQ(cubePositions[1], 5);
    ASSERT_EQ(cubePositions[2], 8);
    ASSERT_EQ(cubePositions[3], 1);
    ASSERT_EQ(cubePositions[5], 7);
    ASSERT_EQ(cubePositions[6], 0);
    ASSERT_EQ(cubePositions[7], 3);
    ASSERT_EQ(cubePositions[8], 6);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_counter_clockwise_left_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = LEFT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[18], 0);
    ASSERT_EQ(cubePositions[9], 3);
    ASSERT_EQ(cubePositions[0], 6);
    ASSERT_EQ(cubePositions[21], 9);
    ASSERT_EQ(cubePositions[3], 15);
    ASSERT_EQ(cubePositions[24], 18);
    ASSERT_EQ(cubePositions[15], 21);
    ASSERT_EQ(cubePositions[6], 24);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_counter_clockwise_right_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[26], 8);
    ASSERT_EQ(cubePositions[17], 5);
    ASSERT_EQ(cubePositions[8], 2);
    ASSERT_EQ(cubePositions[23], 17);
    ASSERT_EQ(cubePositions[5], 11);
    ASSERT_EQ(cubePositions[20], 26);
    ASSERT_EQ(cubePositions[11], 23);
    ASSERT_EQ(cubePositions[2], 20);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_counter_clockwise_top_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = UP_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[6], 8);
    ASSERT_EQ(cubePositions[7], 17);
    ASSERT_EQ(cubePositions[8], 26);
    ASSERT_EQ(cubePositions[15], 7);
    ASSERT_EQ(cubePositions[17], 25);
    ASSERT_EQ(cubePositions[24], 6);
    ASSERT_EQ(cubePositions[25], 15);
    ASSERT_EQ(cubePositions[26], 24);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_two_counter_clockwise_down_rotations) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = DOWN_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[0], 18);
    ASSERT_EQ(cubePositions[1], 9);
    ASSERT_EQ(cubePositions[2], 0);
    ASSERT_EQ(cubePositions[11], 1);
    ASSERT_EQ(cubePositions[20], 2);
    ASSERT_EQ(cubePositions[19], 11);
    ASSERT_EQ(cubePositions[18], 20);
    ASSERT_EQ(cubePositions[9], 19);

    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[0], 20);
    ASSERT_EQ(cubePositions[1], 19);
    ASSERT_EQ(cubePositions[2], 18);
    ASSERT_EQ(cubePositions[11], 9);
    ASSERT_EQ(cubePositions[20], 0);
    ASSERT_EQ(cubePositions[19], 1);
    ASSERT_EQ(cubePositions[18], 2);
    ASSERT_EQ(cubePositions[9], 11);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_two_clockwise_down_rotations) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = DOWN_CUBES;
    rubiks::adjustIndicesClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[0], 2);
    ASSERT_EQ(cubePositions[1], 11);
    ASSERT_EQ(cubePositions[2], 20);
    ASSERT_EQ(cubePositions[11], 19);
    ASSERT_EQ(cubePositions[20], 18);
    ASSERT_EQ(cubePositions[19], 9);
    ASSERT_EQ(cubePositions[18], 0);
    ASSERT_EQ(cubePositions[9], 1);

    rubiks::adjustIndicesClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[0], 20);
    ASSERT_EQ(cubePositions[1], 19);
    ASSERT_EQ(cubePositions[2], 18);
    ASSERT_EQ(cubePositions[11], 9);
    ASSERT_EQ(cubePositions[20], 0);
    ASSERT_EQ(cubePositions[19], 1);
    ASSERT_EQ(cubePositions[18], 2);
    ASSERT_EQ(cubePositions[9], 11);
}

TEST(RubiksCubeLogicTest, IndicesAdjustment_RI) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[20], 26);
    ASSERT_EQ(cubePositions[2], 20);
    ASSERT_EQ(cubePositions[8], 2);
    ASSERT_EQ(cubePositions[26], 8);
    ASSERT_EQ(cubePositions[11], 23);
    ASSERT_EQ(cubePositions[5], 11);
    ASSERT_EQ(cubePositions[17], 5);
    ASSERT_EQ(cubePositions[23], 17);
}

TEST(RubiksCubeLogicTest, IndicesAdjustment_RI_BI) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = DOWN_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[18], 26);
    ASSERT_EQ(cubePositions[19], 23);
    ASSERT_EQ(cubePositions[20], 20);
    ASSERT_EQ(cubePositions[11], 1);
    ASSERT_EQ(cubePositions[2], 0);
    ASSERT_EQ(cubePositions[9], 19);
    ASSERT_EQ(cubePositions[0], 18);
    ASSERT_EQ(cubePositions[1], 9);
}

TEST(RubiksCubeLogicTest, IndicesAdjustment_RI_BI_R) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = DOWN_CUBES;
    rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = RIGHT_CUBES;
    rubiks::adjustIndicesClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[23], 1);
    ASSERT_EQ(cubePositions[20], 0);
    ASSERT_EQ(cubePositions[11], 11);
    ASSERT_EQ(cubePositions[2], 2);
    ASSERT_EQ(cubePositions[5], 5);
    ASSERT_EQ(cubePositions[8], 8);
    ASSERT_EQ(cubePositions[17], 17);
    ASSERT_EQ(cubePositions[26], 20);
}

TEST(RubiksCubeLogicTest, Indices_are_adjusted_correctly_for_cw_down_and_cw_right_rotation) {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = DOWN_CUBES;
    rubiks::adjustIndicesClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[18], 0);
    ASSERT_EQ(cubePositions[19], 9);
    ASSERT_EQ(cubePositions[20], 18);
    ASSERT_EQ(cubePositions[9], 1);
    ASSERT_EQ(cubePositions[10], 10);
    ASSERT_EQ(cubePositions[11], 19);
    ASSERT_EQ(cubePositions[0], 2);
    ASSERT_EQ(cubePositions[1], 11);
    ASSERT_EQ(cubePositions[2], 20);

    cubes = RIGHT_CUBES;
    rubiks::adjustIndicesClockwise(cubePositions, cubes);

    ASSERT_EQ(cubePositions[20], 20);
    ASSERT_EQ(cubePositions[23], 19);
    ASSERT_EQ(cubePositions[26], 18);
    ASSERT_EQ(cubePositions[11], 5);
    ASSERT_EQ(cubePositions[2], 8);
    ASSERT_EQ(cubePositions[5], 17);
    ASSERT_EQ(cubePositions[8], 26);
    ASSERT_EQ(cubePositions[17], 23);
}

TEST(RubiksCubeLogicTest, rotation_matrix_is_updated_correctly_for_one_rotation_vector) {
    SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4()};
    const auto piHalf = glm::pi<float>() / 2.0f;
    const glm::vec3 rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);

    rubiks::updateCubeRotation(cubeRotation, rotationVector, false);

    const auto &matrix = cubeRotation.rotationMatrix;
    ASSERT_FLOAT_EQ(matrix[0][0], 1.0f);
    ASSERT_FLOAT_EQ(matrix[1][0], 0.0f);
    ASSERT_FLOAT_EQ(matrix[2][0], 0.0f);
    ASSERT_FLOAT_EQ(matrix[0][1], 0.0f);
    ASSERT_FLOAT_EQ(matrix[0][2], 0.0f);

    const auto epsilon = std::numeric_limits<float>::epsilon() * 100;
    ASSERT_NEAR(matrix[1][1], -0.0f, epsilon); // cos(a)
    ASSERT_NEAR(matrix[1][2], 1.0f, epsilon);  // -sin(a)
    ASSERT_NEAR(matrix[2][1], -1.0f, epsilon); // sin(a)
    ASSERT_NEAR(matrix[2][2], 0.0f, epsilon);  // cos(a)
}

TEST(RubiksCubeLogicTest, rotation_matrix_is_updated_correctly_for_two_opposing_rotation_vectors) {
    SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4()};
    const auto piHalf = glm::pi<float>() / 2.0f;
    const auto rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);
    const auto negativeRotationVector = glm::vec3(-1.0f * piHalf, 0, 0);
    cubeRotation.rotations.push_back(negativeRotationVector);

    rubiks::updateCubeRotation(cubeRotation, negativeRotationVector, false);

    const auto &matrix = cubeRotation.rotationMatrix;
    ASSERT_FLOAT_EQ(matrix[0][0], 1.0f);
    ASSERT_FLOAT_EQ(matrix[1][0], 0.0f);
    ASSERT_FLOAT_EQ(matrix[2][0], 0.0f);
    ASSERT_FLOAT_EQ(matrix[0][1], 0.0f);
    ASSERT_FLOAT_EQ(matrix[0][2], 0.0f);

    ASSERT_FLOAT_EQ(matrix[1][1], 1.0f);
    ASSERT_FLOAT_EQ(matrix[1][2], 0.0f);
    ASSERT_FLOAT_EQ(matrix[2][1], 0.0f);
    ASSERT_FLOAT_EQ(matrix[2][2], 1.0f);
}

TEST(RubiksCubeLogicTest, rotations_list_is_updated_correctly) {
    SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4()};
    float piHalf = glm::pi<float>() / 2.0f;
    const glm::vec3 rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);

    rubiks::updateCubeRotation(cubeRotation, rotationVector, false);
    ASSERT_EQ(cubeRotation.rotations.size(), 1);
    ASSERT_EQ(cubeRotation.rotations[0], rotationVector);

    rubiks::updateCubeRotation(cubeRotation, rotationVector, true);
    ASSERT_EQ(cubeRotation.rotations.size(), 2);
    ASSERT_EQ(cubeRotation.rotations[0], rotationVector);
    ASSERT_EQ(cubeRotation.rotations[1], glm::vec3());
}

TEST(RubiksCubeLogicTest, Rotation_matrix_is_calculated_correctly_for_two_rotations) {
    SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4()};
    const auto piHalf = glm::pi<float>() / 2.0F;
    cubeRotation.rotations.emplace_back(piHalf, 0, 0);
    cubeRotation.rotations.emplace_back(0, piHalf, 0);

    rubiks::updateCubeRotation(cubeRotation, glm::vec3(0, piHalf, 0), false);

    const auto &matrix = cubeRotation.rotationMatrix;
    const auto epsilon = std::numeric_limits<float>::epsilon() * 100;
    ASSERT_NEAR(matrix[0][0], -0.0f, epsilon);
    ASSERT_NEAR(matrix[0][1], 0.0f, epsilon);
    ASSERT_NEAR(matrix[0][2], -1.0f, epsilon);
    ASSERT_NEAR(matrix[1][0], 1.0f, epsilon);
    ASSERT_NEAR(matrix[1][1], -0.0f, epsilon);
    ASSERT_NEAR(matrix[1][2], -0.0f, epsilon);

    ASSERT_NEAR(matrix[2][0], -0.0f, epsilon);
    ASSERT_NEAR(matrix[2][1], -1.0f, epsilon);
    ASSERT_NEAR(matrix[2][2], 0.0f, epsilon);
}

void assertDirection(Face face, int direction) {
    RotationCommand rot = {face, Direction::CLOCKWISE};
    ASSERT_EQ(rubiks::getDirection(rot), direction);
    rot = {face, Direction::COUNTER_CLOCKWISE};
    ASSERT_EQ(rubiks::getDirection(rot), -1 * direction);
}

TEST(RubiksCubeLogicTest, Direction_is_calculated_correctly) {
    assertDirection(Face::RIGHT, -1);
    assertDirection(Face::LEFT, 1);

    assertDirection(Face::UP, -1);
    assertDirection(Face::DOWN, 1);

    assertDirection(Face::FRONT, -1);
    assertDirection(Face::BACK, 1);
}

void assertFaceRotation(Face face, std::vector<Face> results) {
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(1, 0, 0)), results[0]);
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(-1, 0, 0)), results[1]);
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(0, 1, 0)), results[2]);
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(0, -1, 0)), results[3]);
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(0, 0, 1)), results[4]);
    ASSERT_EQ(rubiks::rotateFaceBack(face, glm::vec3(0, 0, -1)), results[5]);
}

TEST(RubiksCubeLogicTest, Single_face_is_rotated_correctly) {
    std::vector<Face> results = {Face::UP, Face::DOWN, Face::LEFT, Face::RIGHT, Face::FRONT, Face::FRONT};
    assertFaceRotation(Face::FRONT, results);

    results = {Face::DOWN, Face::UP, Face::RIGHT, Face::LEFT, Face::BACK, Face::BACK};
    assertFaceRotation(Face::BACK, results);

    results = {Face::LEFT, Face::LEFT, Face::BACK, Face::FRONT, Face::UP, Face::DOWN};
    assertFaceRotation(Face::LEFT, results);

    results = {Face::RIGHT, Face::RIGHT, Face::FRONT, Face::BACK, Face::DOWN, Face::UP};
    assertFaceRotation(Face::RIGHT, results);

    results = {Face::BACK, Face::FRONT, Face::UP, Face::UP, Face::RIGHT, Face::LEFT};
    assertFaceRotation(Face::UP, results);

    results = {Face::FRONT, Face::BACK, Face::DOWN, Face::DOWN, Face::LEFT, Face::RIGHT};
    assertFaceRotation(Face::DOWN, results);
}

TEST(RubiksCubeLogicTest, rotations_are_squashed_correctly) {
    auto cubes = std::vector<SmallCube>();
    auto rotations = std::vector<glm::vec3>();
    rotations.emplace_back(1.0f, 0, 0);
    rotations.emplace_back(-1.0f, 0, 0);
    cubes.push_back({rotations, glm::mat4()});
    unsigned int squashedRotations = rubiks::squashRotations(cubes);
    ASSERT_TRUE(cubes[0].rotations.empty());
    ASSERT_EQ(squashedRotations, 2);
}
