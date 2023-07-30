#pragma once

#include <vector>
#include <string>
#include <glm/ext.hpp>

#define FRONT_CUBES {18, 19, 20, 21, 22, 23, 24, 25, 26}
#define BACK_CUBES {2, 1, 0, 5, 4, 3, 8, 7, 6}
#define RIGHT_CUBES {20, 11, 2, 23, 14, 5, 26, 17, 8}
#define LEFT_CUBES {0, 9, 18, 3, 12, 21, 6, 15, 24}
#define TOP_CUBES {24, 25, 26, 15, 16, 17, 6, 7, 8}
#define BOTTOM_CUBES {0, 1, 2, 9, 10, 11, 18, 19, 20}

// macros for cube rotations (I - inverse)
#define R_R {Face::RIGHT, Direction::CLOCKWISE}
#define R_RI {Face::RIGHT, Direction::COUNTER_CLOCKWISE}
#define R_F {Face::FRONT, Direction::CLOCKWISE}
#define R_FI {Face::FRONT, Direction::COUNTER_CLOCKWISE}
#define R_BO {Face::BOTTOM, Direction::CLOCKWISE}
#define R_BOI {Face::BOTTOM, Direction::COUNTER_CLOCKWISE}
#define R_L {Face::LEFT, Direction::CLOCKWISE}
#define R_LI {Face::LEFT, Direction::COUNTER_CLOCKWISE}
#define R_T {Face::TOP, Direction::CLOCKWISE}
#define R_TI {Face::TOP, Direction::COUNTER_CLOCKWISE}
#define R_BA {Face::BACK, Direction::CLOCKWISE}
#define R_BAI {Face::BACK, Direction::COUNTER_CLOCKWISE}

enum class Face {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

enum class Direction {
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

std::string to_string(const Face &face, bool simple = false);

std::string to_string(const Direction &dir, bool simple = false);

std::string to_string(const RotationCommand &cmd, bool simple = false);
