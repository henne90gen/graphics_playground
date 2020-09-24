#include <catch.hpp>

#include <vector>

#include "rubiks_cube/RubiksCubeLogic.h"

void initCubePositions(std::vector<unsigned int> &cubePosition) {
    for (int i = 0; i < 27; i++) {
        cubePosition.push_back(i);
    }
}

TEST_CASE("Indices are adjusted correctly for counter clockwise front rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = FRONT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[18] == 24);
    REQUIRE(cubePositions[19] == 21);
    REQUIRE(cubePositions[20] == 18);
    REQUIRE(cubePositions[21] == 25);
    REQUIRE(cubePositions[23] == 19);
    REQUIRE(cubePositions[24] == 26);
    REQUIRE(cubePositions[25] == 23);
    REQUIRE(cubePositions[26] == 20);
}

TEST_CASE("Indices are adjusted correctly for counter clockwise back rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = BACK_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 2);
    REQUIRE(cubePositions[1] == 5);
    REQUIRE(cubePositions[2] == 8);
    REQUIRE(cubePositions[3] == 1);
    REQUIRE(cubePositions[5] == 7);
    REQUIRE(cubePositions[6] == 0);
    REQUIRE(cubePositions[7] == 3);
    REQUIRE(cubePositions[8] == 6);
}

TEST_CASE("Indices are adjusted correctly for counter clockwise left rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = LEFT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[18] == 0);
    REQUIRE(cubePositions[9] == 3);
    REQUIRE(cubePositions[0] == 6);
    REQUIRE(cubePositions[21] == 9);
    REQUIRE(cubePositions[3] == 15);
    REQUIRE(cubePositions[24] == 18);
    REQUIRE(cubePositions[15] == 21);
    REQUIRE(cubePositions[6] == 24);
}

TEST_CASE("Indices are adjusted correctly for counter clockwise right rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[26] == 8);
    REQUIRE(cubePositions[17] == 5);
    REQUIRE(cubePositions[8] == 2);
    REQUIRE(cubePositions[23] == 17);
    REQUIRE(cubePositions[5] == 11);
    REQUIRE(cubePositions[20] == 26);
    REQUIRE(cubePositions[11] == 23);
    REQUIRE(cubePositions[2] == 20);
}

TEST_CASE("Indices are adjusted correctly for counter clockwise top rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = TOP_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[6] == 8);
    REQUIRE(cubePositions[7] == 17);
    REQUIRE(cubePositions[8] == 26);
    REQUIRE(cubePositions[15] == 7);
    REQUIRE(cubePositions[17] == 25);
    REQUIRE(cubePositions[24] == 6);
    REQUIRE(cubePositions[25] == 15);
    REQUIRE(cubePositions[26] == 24);
}

TEST_CASE("Indices are adjusted correctly for two counter clockwise bottom rotations") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = BOTTOM_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 18);
    REQUIRE(cubePositions[1] == 9);
    REQUIRE(cubePositions[2] == 0);
    REQUIRE(cubePositions[11] == 1);
    REQUIRE(cubePositions[20] == 2);
    REQUIRE(cubePositions[19] == 11);
    REQUIRE(cubePositions[18] == 20);
    REQUIRE(cubePositions[9] == 19);

    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 20);
    REQUIRE(cubePositions[1] == 19);
    REQUIRE(cubePositions[2] == 18);
    REQUIRE(cubePositions[11] == 9);
    REQUIRE(cubePositions[20] == 0);
    REQUIRE(cubePositions[19] == 1);
    REQUIRE(cubePositions[18] == 2);
    REQUIRE(cubePositions[9] == 11);
}

TEST_CASE("Indices are adjusted correctly for two clockwise bottom rotations") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = BOTTOM_CUBES;
    adjustIndicesClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 2);
    REQUIRE(cubePositions[1] == 11);
    REQUIRE(cubePositions[2] == 20);
    REQUIRE(cubePositions[11] == 19);
    REQUIRE(cubePositions[20] == 18);
    REQUIRE(cubePositions[19] == 9);
    REQUIRE(cubePositions[18] == 0);
    REQUIRE(cubePositions[9] == 1);

    adjustIndicesClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 20);
    REQUIRE(cubePositions[1] == 19);
    REQUIRE(cubePositions[2] == 18);
    REQUIRE(cubePositions[11] == 9);
    REQUIRE(cubePositions[20] == 0);
    REQUIRE(cubePositions[19] == 1);
    REQUIRE(cubePositions[18] == 2);
    REQUIRE(cubePositions[9] == 11);
}

TEST_CASE("IndicesAdjustement: RI") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[20] == 26);
    REQUIRE(cubePositions[2] == 20);
    REQUIRE(cubePositions[8] == 2);
    REQUIRE(cubePositions[26] == 8);
    REQUIRE(cubePositions[11] == 23);
    REQUIRE(cubePositions[5] == 11);
    REQUIRE(cubePositions[17] == 5);
    REQUIRE(cubePositions[23] == 17);
}

TEST_CASE("IndicesAdjustment: RI, BI") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = BOTTOM_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[18] == 26);
    REQUIRE(cubePositions[19] == 23);
    REQUIRE(cubePositions[20] == 20);
    REQUIRE(cubePositions[11] == 1);
    REQUIRE(cubePositions[2] == 0);
    REQUIRE(cubePositions[9] == 19);
    REQUIRE(cubePositions[0] == 18);
    REQUIRE(cubePositions[1] == 9);
}

TEST_CASE("IndicesAdjustment: RI, BI, R") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = RIGHT_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = BOTTOM_CUBES;
    adjustIndicesCounterClockwise(cubePositions, cubes);

    cubes = RIGHT_CUBES;
    adjustIndicesClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[23] == 1);
    REQUIRE(cubePositions[20] == 0);
    REQUIRE(cubePositions[11] == 11);
    REQUIRE(cubePositions[2] == 2);
    REQUIRE(cubePositions[5] == 5);
    REQUIRE(cubePositions[8] == 8);
    REQUIRE(cubePositions[17] == 17);
    REQUIRE(cubePositions[26] == 20);
}

TEST_CASE("Indices are adjusted correctly for cw bottom and cw right rotation") {
    std::vector<unsigned int> cubePositions;
    initCubePositions(cubePositions);
    std::vector<unsigned int> cubes = BOTTOM_CUBES;
    adjustIndicesClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[0] == 2);
    REQUIRE(cubePositions[1] == 11);
    REQUIRE(cubePositions[2] == 20);
    REQUIRE(cubePositions[11] == 19);
    REQUIRE(cubePositions[20] == 18);
    REQUIRE(cubePositions[19] == 9);
    REQUIRE(cubePositions[18] == 0);
    REQUIRE(cubePositions[9] == 1);

    cubes = RIGHT_CUBES;
    adjustIndicesClockwise(cubePositions, cubes);

    REQUIRE(cubePositions[20] == 20);
    REQUIRE(cubePositions[23] == 19);
    REQUIRE(cubePositions[26] == 18);
    REQUIRE(cubePositions[11] == 5);
    REQUIRE(cubePositions[2] == 8);
    REQUIRE(cubePositions[5] == 17);
    REQUIRE(cubePositions[8] == 26);
    REQUIRE(cubePositions[17] == 23);
}

TEST_CASE("Rotation matrix is updated correctly for one rotation vector") {
    SmallCube cubeRotation = {
            std::vector<glm::vec3>(),
            glm::mat4()
    };
    float piHalf = glm::pi<float>() / 2.0f;
    const glm::vec3 rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);

    updateCubeRotation(cubeRotation, rotationVector, false);

    glm::mat4 &matrix = cubeRotation.rotationMatrix;
    REQUIRE(matrix[0][0] == Approx(1.0f));
    REQUIRE(matrix[1][0] == Approx(0.0f));
    REQUIRE(matrix[2][0] == Approx(0.0f));
    REQUIRE(matrix[0][1] == Approx(0.0f));
    REQUIRE(matrix[0][2] == Approx(0.0f));

    float margin = std::numeric_limits<float>::epsilon() * 100;
    REQUIRE(matrix[1][1] == Approx(-0.0f).margin(margin)); // cos(a)
    REQUIRE(matrix[1][2] == Approx(1.0f).margin(margin)); // -sin(a)
    REQUIRE(matrix[2][1] == Approx(-1.0f)); // sin(a)
    REQUIRE(matrix[2][2] == Approx(0.0f).margin(margin)); // cos(a)
}

TEST_CASE("Rotation matrix is updated correctly for two opposing rotation vectors") {
    SmallCube cubeRotation = {
            std::vector<glm::vec3>(),
            glm::mat4()
    };
    float piHalf = glm::pi<float>() / 2.0f;
    const glm::vec3 rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);
    glm::vec3 negativeRotationVector = glm::vec3(-1.0f * piHalf, 0, 0);
    cubeRotation.rotations.push_back(negativeRotationVector);

    updateCubeRotation(cubeRotation, negativeRotationVector, false);

    glm::mat4 &matrix = cubeRotation.rotationMatrix;
    REQUIRE(matrix[0][0] == Approx(1.0f));
    REQUIRE(matrix[1][0] == Approx(0.0f));
    REQUIRE(matrix[2][0] == Approx(0.0f));
    REQUIRE(matrix[0][1] == Approx(0.0f));
    REQUIRE(matrix[0][2] == Approx(0.0f));

    REQUIRE(matrix[1][1] == Approx(1.0f));
    REQUIRE(matrix[1][2] == Approx(0.0f));
    REQUIRE(matrix[2][1] == Approx(0.0f));
    REQUIRE(matrix[2][2] == Approx(1.0f));
}

TEST_CASE("Rotations list is updated correctly") {
    SmallCube cubeRotation = {
            std::vector<glm::vec3>(),
            glm::mat4()
    };
    float piHalf = glm::pi<float>() / 2.0f;
    const glm::vec3 rotationVector = glm::vec3(piHalf, 0, 0);
    cubeRotation.rotations.push_back(rotationVector);

    updateCubeRotation(cubeRotation, rotationVector, false);
    REQUIRE(cubeRotation.rotations.size() == 1);
    REQUIRE(cubeRotation.rotations[0] == rotationVector);

    updateCubeRotation(cubeRotation, rotationVector, true);
    REQUIRE(cubeRotation.rotations.size() == 2);
    REQUIRE(cubeRotation.rotations[0] == rotationVector);
    REQUIRE(cubeRotation.rotations[1] == glm::vec3());
}

TEST_CASE("Rotation matrix is calculated correctly for two rotations") {
    SmallCube cubeRotation = {
            std::vector<glm::vec3>(),
            glm::mat4()
    };
    float piHalf = glm::pi<float>() / 2.0f;
    cubeRotation.rotations.emplace_back(piHalf, 0, 0);
    cubeRotation.rotations.emplace_back(0, piHalf, 0);

    updateCubeRotation(cubeRotation, glm::vec3(0, piHalf, 0), false);

    glm::mat4 &matrix = cubeRotation.rotationMatrix;
    float margin = std::numeric_limits<float>::epsilon() * 100;
    REQUIRE(matrix[0][0] == Approx(-0.0f).margin(margin));
    REQUIRE(matrix[0][1] == Approx(0.0f));
    REQUIRE(matrix[0][2] == Approx(-1.0f));
    REQUIRE(matrix[1][0] == Approx(1.0f));
    REQUIRE(matrix[1][1] == Approx(-0.0f).margin(margin));
    REQUIRE(matrix[1][2] == Approx(-0.0f).margin(margin));

    REQUIRE(matrix[2][0] == Approx(-0.0f).margin(margin));
    REQUIRE(matrix[2][1] == Approx(-1.0f));
    REQUIRE(matrix[2][2] == Approx(0.0f).margin(margin));
}

void assertDirection(Face face, int direction) {
    RotationCommand rot = {face, CLOCKWISE};
    REQUIRE(getDirection(rot) == direction);
    rot = {face, COUNTER_CLOCKWISE};
    REQUIRE(getDirection(rot) == -1 * direction);
}

TEST_CASE("Direction is calculated correctly") {
    assertDirection(RIGHT, -1);
    assertDirection(LEFT, 1);

    assertDirection(TOP, -1);
    assertDirection(BOTTOM, 1);

    assertDirection(FRONT, -1);
    assertDirection(BACK, 1);
}

void assertFaceRotation(Face face, std::vector<Face> results) {
    REQUIRE(rotateFaceBack(face, glm::vec3(1, 0, 0)) == results[0]);
    REQUIRE(rotateFaceBack(face, glm::vec3(-1, 0, 0)) == results[1]);
    REQUIRE(rotateFaceBack(face, glm::vec3(0, 1, 0)) == results[2]);
    REQUIRE(rotateFaceBack(face, glm::vec3(0, -1, 0)) == results[3]);
    REQUIRE(rotateFaceBack(face, glm::vec3(0, 0, 1)) == results[4]);
    REQUIRE(rotateFaceBack(face, glm::vec3(0, 0, -1)) == results[5]);
}

TEST_CASE("Single face is rotated correctly") {
    std::vector<Face> results = {TOP, BOTTOM, LEFT, RIGHT, FRONT, FRONT};
    assertFaceRotation(FRONT, results);

    results = {BOTTOM, TOP, RIGHT, LEFT, BACK, BACK};
    assertFaceRotation(BACK, results);

    results = {LEFT, LEFT, BACK, FRONT, TOP, BOTTOM};
    assertFaceRotation(LEFT, results);

    results = {RIGHT, RIGHT, FRONT, BACK, BOTTOM, TOP};
    assertFaceRotation(RIGHT, results);

    results = {BACK, FRONT, TOP, TOP, RIGHT, LEFT};
    assertFaceRotation(TOP, results);

    results = {FRONT, BACK, BOTTOM, BOTTOM, LEFT, RIGHT};
    assertFaceRotation(BOTTOM, results);
}

TEST_CASE("Rotations are squashed correctly") {
    auto cubes = std::vector<SmallCube>();
    auto rotations = std::vector<glm::vec3>();
    rotations.emplace_back(1.0f, 0, 0);
    rotations.emplace_back(-1.0f, 0, 0);
    cubes.push_back({rotations, glm::mat4()});
    unsigned int squashedRotations = squashRotations(cubes);
    REQUIRE(cubes[0].rotations.empty());
    REQUIRE(squashedRotations == 2);
}
