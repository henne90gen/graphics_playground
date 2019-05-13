#include "RubiksCubeLogic.h"
#include <iostream>

bool rotate(CubeRotation *cubeRotations, unsigned int *cubePositions, Rotation &rot, float rotationSpeed) {
    float direction = 1.0f;
    if (rot.direction == CLOCKWISE && rot.face != LEFT && rot.face != BOTTOM) {
        direction = -1.0f;
    }
    rot.currentAngle += rotationSpeed;

    bool isDoneRotating = false;
    auto piHalf = glm::pi<float>() / 2.0f;
    if (rot.currentAngle >= piHalf) {
        isDoneRotating = true;
        rot.currentAngle = piHalf;
    }

    std::vector<unsigned int> cubes(9);
    glm::vec3 rotationVector;
    switch (rot.face) {
        case FRONT:
            cubes = FRONT_CUBES;
            rotationVector = glm::vec3(0, 0, direction * rot.currentAngle);
            break;
        case BACK:
            cubes = BACK_CUBES;
            rotationVector = glm::vec3(0, 0, direction * rot.currentAngle);
            break;
        case LEFT:
            cubes = LEFT_CUBES;
            rotationVector = glm::vec3(direction * rot.currentAngle, 0, 0);
            break;
        case RIGHT:
            cubes = RIGHT_CUBES;
            rotationVector = glm::vec3(direction * rot.currentAngle, 0, 0);
            break;
        case TOP:
            cubes = TOP_CUBES;
            rotationVector = glm::vec3(0, direction * rot.currentAngle, 0);
            break;
        case BOTTOM:
            cubes = BOTTOM_CUBES;
            rotationVector = glm::vec3(0, direction * rot.currentAngle, 0);
            break;
    }

    for (unsigned int i = 0; i < 9; i++) {
        unsigned int cubeIndex = cubePositions[cubes[i]];
        cubeRotations[cubeIndex].rotations[cubeRotations[cubeIndex].rotations.size() - 1] = rotationVector;
        if (!isDoneRotating) {
            continue;
        }

        updateRotationMatrix(cubeRotations[cubeIndex]);
    }

    if (isDoneRotating) {
        if (rot.direction == CLOCKWISE) {
            adjustIndicesClockwise(cubePositions, cubes);
        } else {
            adjustIndicesCounterClockwise(cubePositions, cubes);
        }
    }

    return isDoneRotating;
}

void updateRotationMatrix(CubeRotation &cubeRotation) {
    glm::mat4 cubeMatrix = glm::mat4(1.0f);
    for (auto &rotation : cubeRotation.rotations) {
        const glm::vec3 &normalizedRotation = glm::normalize(rotation);
        float rotationAngle;
        if (normalizedRotation.x >= 1.0f) {
            rotationAngle = rotation.x;
        } else if (normalizedRotation.y >= 1.0f) {
            rotationAngle = rotation.y;
        } else {
            rotationAngle = rotation.z;
        }
        cubeMatrix = glm::rotate(cubeMatrix, rotationAngle, normalizedRotation);
    }
    cubeRotation.rotationMatrix = cubeMatrix;
    cubeRotation.rotations.emplace_back();
}

void adjustIndicesCounterClockwise(unsigned int positions[27], std::vector<unsigned int> &selectedCubes) {
    // move the corners
    unsigned int tmp1 = positions[selectedCubes[2]];
    positions[selectedCubes[2]] = positions[selectedCubes[0]];
    unsigned int tmp2 = positions[selectedCubes[8]];
    positions[selectedCubes[8]] = tmp1;
    tmp1 = positions[selectedCubes[6]];
    positions[selectedCubes[6]] = tmp2;
    positions[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = positions[selectedCubes[5]];
    positions[selectedCubes[5]] = positions[selectedCubes[1]];
    tmp2 = positions[selectedCubes[7]];
    positions[selectedCubes[7]] = tmp1;
    tmp1 = positions[selectedCubes[3]];
    positions[selectedCubes[3]] = tmp2;
    positions[selectedCubes[1]] = tmp1;
}

void adjustIndicesClockwise(unsigned int positions[27], std::vector<unsigned int> &selectedCubes) {
    // move the corners
    unsigned int tmp1 = positions[selectedCubes[6]];
    positions[selectedCubes[6]] = positions[selectedCubes[0]];
    unsigned int tmp2 = positions[selectedCubes[8]];
    positions[selectedCubes[8]] = tmp1;
    tmp1 = positions[selectedCubes[2]];
    positions[selectedCubes[2]] = tmp2;
    positions[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = positions[selectedCubes[3]];
    positions[selectedCubes[3]] = positions[selectedCubes[1]];
    tmp2 = positions[selectedCubes[7]];
    positions[selectedCubes[7]] = tmp1;
    tmp1 = positions[selectedCubes[5]];
    positions[selectedCubes[5]] = tmp2;
    positions[selectedCubes[1]] = tmp1;
}

