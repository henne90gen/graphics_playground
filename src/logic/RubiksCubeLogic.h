#pragma once

#include <glm/ext.hpp>
#include <vector>

#define FRONT_CUBES {18, 19, 20, 21, 22, 23, 24, 25, 26}
#define BACK_CUBES {0, 1, 2, 3, 4, 5, 6, 7, 8}
#define RIGHT_CUBES {2, 5, 8, 11, 14, 17, 20, 23, 26}
#define LEFT_CUBES {0, 3, 6, 9, 12, 15, 18, 21, 24}
#define TOP_CUBES {6, 7, 8, 15, 16, 17, 24, 25, 26}
#define BOTTOM_CUBES {0, 1, 2, 9, 10, 11, 18, 19, 20}

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
    float currentAngle;
};

struct CubeRotation {
    glm::vec3 finalRotation;
    glm::vec3 currentRotation;
};

bool rotate(CubeRotation *cubeRotations, unsigned int *cubePositions, Rotation &rot, float rotationSpeed = 0.1f);

void adjustIndicesClockwise(unsigned int positions[27], std::vector<unsigned int> &selectedCubes);

void adjustIndicesCounterClockwise(unsigned int positions[27], std::vector<unsigned int> &selectedCubes);
