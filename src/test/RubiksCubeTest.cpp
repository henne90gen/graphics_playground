#include "catch.hpp"
#include "RubiksCubeLogic.h"
#include <vector>
#include <iostream>

void initCubePositions(unsigned int *cubePosition) {
    for (int i = 0; i < 27; i++) {
        cubePosition[i] = i;
    }
}

TEST_CASE("Indices are adjusted correctly for counter clockwise bottom rotation") {
    unsigned int cubePositions[27];
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

TEST_CASE("Indices are adjusted correctly for clockwise bottom rotation") {
    unsigned int cubePositions[27];
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

TEST_CASE("Indices are adjusted correctly for cw bottom and cw right rotation") {
    unsigned int cubePositions[27];
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
    CubeRotation cubeRotation = {
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
    CubeRotation cubeRotation = {
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
    CubeRotation cubeRotation = {
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
