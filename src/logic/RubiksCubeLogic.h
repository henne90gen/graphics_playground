#pragma once

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#define FRONT_CUBES {18, 19, 20, 21, 22, 23, 24, 25, 26}
#define BACK_CUBES {0, 1, 2, 3, 4, 5, 6, 7, 8}
#define RIGHT_CUBES {2, 5, 8, 11, 14, 17, 20, 23, 26}
#define LEFT_CUBES {0, 3, 6, 9, 12, 15, 18, 21, 24}
#define TOP_CUBES {6, 7, 8, 15, 16, 17, 24, 25, 26}
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

struct Rotation {
    Face face;
    Direction direction;
    float currentAngle = 0;
};

struct CubeRotation {
    std::vector<glm::vec3> rotations;
    glm::mat4 rotationMatrix;
};

bool rotate(std::vector<CubeRotation> &cubeRotations, std::vector<unsigned int> &cubePositions, Rotation &rot,
            float rotationSpeed = 0.1f);

int getDirection(Rotation &rot);

void adjustIndicesClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes);

void adjustIndicesCounterClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes);

void updateCubeRotation(CubeRotation &cubeRotation, glm::vec3 rotationVector, bool isDoneRotating);
