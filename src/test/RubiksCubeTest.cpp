#include "catch.hpp"
#include "rubiks_cube/RubiksCube.h"
#include <vector>
#include <iostream>

void assertWholeFace(RubiksCube cube, Face face) {
    for (int i = 0; i < 9; i++) {
//        REQUIRE(cube.getCurrentFace(face, 0) == face);
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

//    REQUIRE(cube.getCurrentFace(FRONT, 2) == BOTTOM);
//    REQUIRE(cube.getCurrentFace(FRONT, 5) == BOTTOM);
//    REQUIRE(cube.getCurrentFace(FRONT, 8) == BOTTOM);
}
