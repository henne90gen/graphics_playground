#include "catch.hpp"
#include "RubiksCubeLogic.h"

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


TEST_CASE("Indices are adjusted correctly for clockwise bottom and clockwise right rotation") {
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
