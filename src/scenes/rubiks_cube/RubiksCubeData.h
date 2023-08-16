#pragma once

#include <glm/ext.hpp>
#include <string>
#include <vector>

/*
Global Indices:
           UP
            6  7  8
           15 16 17
           24 25 26
          ----------
LEFT     | FRONT    | RIGHT    | BACK
 6 15 24 | 24 25 26 | 26 17  8 | 8  7  6
 3 12 21 | 21 22 23 | 23 14  5 | 5  4  3
 0  9 18 | 18 19 20 | 20 11  2 | 2  1  0
          ----------
           DOWN
           18 19 20
            9 10 11
            0  1  2

Local Indices:
       UP
       0 1 2
       3 4 5
       6 7 8
LEFT   FRONT  RIGHT  BACK
0 1 2  0 1 2  0 1 2  0 1 2
3 4 5  3 4 5  3 4 5  3 4 5
6 7 8  6 7 8  6 7 8  6 7 8
       DOWN
       0 1 2
       3 4 5
       6 7 8
*/

#define FRONT_CUBES                                                                                                    \
    { 24, 25, 26, 21, 22, 23, 18, 19, 20 }
#define BACK_CUBES                                                                                                     \
    { 8, 7, 6, 5, 4, 3, 2, 1, 0 }
#define RIGHT_CUBES                                                                                                    \
    { 26, 17, 8, 23, 14, 5, 20, 11, 2 }
#define LEFT_CUBES                                                                                                     \
    { 6, 15, 24, 3, 12, 21, 0, 9, 18 }
#define UP_CUBES                                                                                                       \
    { 6, 7, 8, 15, 16, 17, 24, 25, 26 }
#define DOWN_CUBES                                                                                                     \
    { 18, 19, 20, 9, 10, 11, 0, 1, 2 }

// macros for cube rotations (I = inverse)
#define R_R                                                                                                            \
    { Face::RIGHT, Direction::CLOCKWISE }
#define R_RI                                                                                                           \
    { Face::RIGHT, Direction::COUNTER_CLOCKWISE }
#define R_F                                                                                                            \
    { Face::FRONT, Direction::CLOCKWISE }
#define R_FI                                                                                                           \
    { Face::FRONT, Direction::COUNTER_CLOCKWISE }
#define R_D                                                                                                            \
    { Face::DOWN, Direction::CLOCKWISE }
#define R_DI                                                                                                           \
    { Face::DOWN, Direction::COUNTER_CLOCKWISE }
#define R_L                                                                                                            \
    { Face::LEFT, Direction::CLOCKWISE }
#define R_LI                                                                                                           \
    { Face::LEFT, Direction::COUNTER_CLOCKWISE }
#define R_U                                                                                                            \
    { Face::UP, Direction::CLOCKWISE }
#define R_UI                                                                                                           \
    { Face::UP, Direction::COUNTER_CLOCKWISE }
#define R_B                                                                                                            \
    { Face::BACK, Direction::CLOCKWISE }
#define R_BI                                                                                                           \
    { Face::BACK, Direction::COUNTER_CLOCKWISE }

// NOTE Do not change the numeric values of these enum constants. They are being used to index into arrays.
enum class Face {
    NONE = 0,
    FRONT = 1,
    BACK = 2,
    LEFT = 3,
    RIGHT = 4,
    UP = 5,
    DOWN = 6,
};

enum class Direction { CLOCKWISE, COUNTER_CLOCKWISE };

struct RotationCommand {
    Face side = {};
    Direction direction = {};
};

struct SmallCube {
    std::vector<glm::vec3> rotations;
    glm::mat4 rotationMatrix;
};

std::string to_string(const Face &face, bool simple = false);

std::string to_string(const Direction &dir, bool simple = false);

std::string to_string(const RotationCommand &cmd, bool simple = false);
