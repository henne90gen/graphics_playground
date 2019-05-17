#pragma once

#include <vector>
#include <glm/ext.hpp>

#define FRONT_CUBES {18, 19, 20, 21, 22, 23, 24, 25, 26}
#define BACK_CUBES {2, 1, 0, 5, 4, 3, 8, 7, 6}
#define RIGHT_CUBES {20, 11, 2, 23, 14, 5, 26, 17, 8}
#define LEFT_CUBES {0, 9, 18, 3, 12, 21, 6, 15, 24}
#define TOP_CUBES {24, 25, 26, 15, 16, 17, 6, 7, 8}
#define BOTTOM_CUBES {0, 1, 2, 9, 10, 11, 18, 19, 20}

// macros for cube rotations (I - inverse)
#define R_R {RIGHT, CLOCKWISE}
#define R_RI {RIGHT, COUNTER_CLOCKWISE}
#define R_F {FRONT, CLOCKWISE}
#define R_FI {FRONT, COUNTER_CLOCKWISE}
#define R_BO {BOTTOM, CLOCKWISE}
#define R_BOI {BOTTOM, COUNTER_CLOCKWISE}
#define R_L {LEFT, CLOCKWISE}
#define R_LI {LEFT, COUNTER_CLOCKWISE}
#define R_T {TOP, CLOCKWISE}
#define R_TI {TOP, COUNTER_CLOCKWISE}
#define R_BA {BACK, CLOCKWISE}
#define R_BAI {BACK, COUNTER_CLOCKWISE}

enum Face {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

enum Direction {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

struct RotationCommand {
    Face face;
    Direction direction;
};

struct SmallCube {
    std::vector<glm::vec3> rotations;
    glm::mat4 rotationMatrix;
};
