#include "catch.hpp"
#include "rubiks_cube/RubiksCube.h"
#include <vector>
#include <iostream>

void assertWholeFace(RubiksCube cube, Face face) {
    for (int i = 0; i < 9; i++) {
        REQUIRE(cube.getCurrentFace(face, i) == face);
    }
}

TEST_CASE("Face does not change without a rotation") {
    RubiksCube cube = RubiksCube();
    assertWholeFace(cube, FRONT);
    assertWholeFace(cube, BACK);
    assertWholeFace(cube, LEFT);
    assertWholeFace(cube, RIGHT);
    assertWholeFace(cube, TOP);
    assertWholeFace(cube, BOTTOM);
}

TEST_CASE("Face is calculated correctly with one rotation") {
    RubiksCube cube = RubiksCube({R_R});
    cube.rotate(2.0f);

    REQUIRE(cube.getCurrentFace(FRONT, 2) == BOTTOM);
    REQUIRE(cube.getCurrentFace(FRONT, 5) == BOTTOM);
    REQUIRE(cube.getCurrentFace(FRONT, 8) == BOTTOM);

    REQUIRE(cube.getCurrentFace(TOP, 2) == FRONT);
    REQUIRE(cube.getCurrentFace(TOP, 5) == FRONT);
    REQUIRE(cube.getCurrentFace(TOP, 8) == FRONT);

    REQUIRE(cube.getCurrentFace(BACK, 0) == TOP);
    REQUIRE(cube.getCurrentFace(BACK, 3) == TOP);
    REQUIRE(cube.getCurrentFace(BACK, 6) == TOP);

    REQUIRE(cube.getCurrentFace(BOTTOM, 2) == BACK);
    REQUIRE(cube.getCurrentFace(BOTTOM, 5) == BACK);
    REQUIRE(cube.getCurrentFace(BOTTOM, 8) == BACK);
}

TEST_CASE("Face is calculated correctly with two rotations") {
    RubiksCube cube = RubiksCube({R_R, R_T});
    cube.rotate(2.0f);
    cube.rotate(2.0f);

    REQUIRE(cube.getCurrentFace(FRONT, 2) == BOTTOM);
    REQUIRE(cube.getCurrentFace(FRONT, 5) == BOTTOM);
    REQUIRE(cube.getCurrentFace(FRONT, 8) == RIGHT);

    REQUIRE(cube.getCurrentFace(TOP, 2) == FRONT);
    REQUIRE(cube.getCurrentFace(TOP, 5) == TOP);
    REQUIRE(cube.getCurrentFace(TOP, 8) == TOP);

    REQUIRE(cube.getCurrentFace(BACK, 0) == TOP);
    REQUIRE(cube.getCurrentFace(BACK, 3) == TOP);
    REQUIRE(cube.getCurrentFace(BACK, 6) == LEFT);

    REQUIRE(cube.getCurrentFace(BOTTOM, 2) == BACK);
    REQUIRE(cube.getCurrentFace(BOTTOM, 5) == BACK);
    REQUIRE(cube.getCurrentFace(BOTTOM, 8) == BACK);
}
