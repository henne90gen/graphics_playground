#include "RubiksCubeLogic.h"

#include <array>
#include <chrono>
#include <iostream>
#include <random>

namespace rubiks {
const unsigned int SIDES_COUNT = 6;
const unsigned int NEXT_FACES_PER_SIDE = 6;
std::array<std::array<Face, SIDES_COUNT>, NEXT_FACES_PER_SIDE> NEXT_FACE_MAP = {{
      // x,    -x,     y,      -y,     z,      -z
      {Face::UP, Face::DOWN, Face::LEFT, Face::RIGHT, Face::FRONT, Face::FRONT},  // FRONT
      {Face::DOWN, Face::UP, Face::RIGHT, Face::LEFT, Face::BACK, Face::BACK},    // BACK
      {Face::LEFT, Face::LEFT, Face::BACK, Face::FRONT, Face::UP, Face::DOWN},    // LEFT
      {Face::RIGHT, Face::RIGHT, Face::FRONT, Face::BACK, Face::DOWN, Face::UP},  // RIGHT
      {Face::BACK, Face::FRONT, Face::UP, Face::UP, Face::RIGHT, Face::LEFT},     // UP
      {Face::FRONT, Face::BACK, Face::DOWN, Face::DOWN, Face::LEFT, Face::RIGHT}, // DOWN
}};

void adjustCubeletIndicesClockwise(std::array<unsigned int, CUBELET_COUNT> &indices,
                                   std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */

    // move the corners
    unsigned int tmp1 = indices[selectedCubes[2]];
    indices[selectedCubes[2]] = indices[selectedCubes[0]];
    unsigned int tmp2 = indices[selectedCubes[8]];
    indices[selectedCubes[8]] = tmp1;
    tmp1 = indices[selectedCubes[6]];
    indices[selectedCubes[6]] = tmp2;
    indices[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = indices[selectedCubes[5]];
    indices[selectedCubes[5]] = indices[selectedCubes[1]];
    tmp2 = indices[selectedCubes[7]];
    indices[selectedCubes[7]] = tmp1;
    tmp1 = indices[selectedCubes[3]];
    indices[selectedCubes[3]] = tmp2;
    indices[selectedCubes[1]] = tmp1;
}

void adjustCubeletIndicesCounterClockwise(std::array<unsigned int, CUBELET_COUNT> &indices,
                                          std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */

    // move the corners
    unsigned int tmp1 = indices[selectedCubes[6]];
    indices[selectedCubes[6]] = indices[selectedCubes[0]];
    unsigned int tmp2 = indices[selectedCubes[8]];
    indices[selectedCubes[8]] = tmp1;
    tmp1 = indices[selectedCubes[2]];
    indices[selectedCubes[2]] = tmp2;
    indices[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = indices[selectedCubes[3]];
    indices[selectedCubes[3]] = indices[selectedCubes[1]];
    tmp2 = indices[selectedCubes[7]];
    indices[selectedCubes[7]] = tmp1;
    tmp1 = indices[selectedCubes[5]];
    indices[selectedCubes[5]] = tmp2;
    indices[selectedCubes[1]] = tmp1;
}

int getDirection(RotationCommand &rot) {
    if (rot.direction == Direction::CLOCKWISE) {
        if (rot.side == Face::DOWN || rot.side == Face::LEFT || rot.side == Face::BACK) {
            return 1;
        }
        return -1;
    }

    if (rot.side == Face::DOWN || rot.side == Face::LEFT || rot.side == Face::BACK) {
        return -1;
    }

    return 1;
}

bool rotate(std::array<SmallCube, CUBELET_COUNT> &cubeRotations, std::array<unsigned int, CUBELET_COUNT> &cubePositions,
            RotationCommand command, float &currentAngle) {
    bool isDoneRotating = false;
    const auto piHalf = glm::pi<float>() * 0.5F;
    if (currentAngle >= piHalf) {
        isDoneRotating = true;
        currentAngle = piHalf;
    }

    std::array<unsigned int, SMALL_FACE_COUNT> cubes;
    glm::vec3 rotationVector;
    auto direction = static_cast<float>(rubiks::getDirection(command));
    switch (command.side) {
    case Face::FRONT:
        cubes = FRONT_CUBES;
        rotationVector = glm::vec3(0, 0, direction * currentAngle);
        break;
    case Face::BACK:
        cubes = BACK_CUBES;
        rotationVector = glm::vec3(0, 0, direction * currentAngle);
        break;
    case Face::LEFT:
        cubes = LEFT_CUBES;
        rotationVector = glm::vec3(direction * currentAngle, 0, 0);
        break;
    case Face::RIGHT:
        cubes = RIGHT_CUBES;
        rotationVector = glm::vec3(direction * currentAngle, 0, 0);
        break;
    case Face::UP:
        cubes = UP_CUBES;
        rotationVector = glm::vec3(0, direction * currentAngle, 0);
        break;
    case Face::DOWN:
        cubes = DOWN_CUBES;
        rotationVector = glm::vec3(0, direction * currentAngle, 0);
        break;
    }

    for (const auto cube : cubes) {
        const auto cubeIndex = cubePositions[cube];
        rubiks::updateCubeRotation(cubeRotations[cubeIndex], rotationVector, isDoneRotating);
    }

    if (!isDoneRotating) {
        return isDoneRotating;
    }

    if (command.direction == Direction::CLOCKWISE) {
        rubiks::adjustCubeletIndicesClockwise(cubePositions, cubes);
    } else {
        rubiks::adjustCubeletIndicesCounterClockwise(cubePositions, cubes);
    }

    return isDoneRotating;
}

void printRotations(std::vector<glm::vec3> &rotations) {
    std::string result;
    bool isFirst = true;
    for (auto &rotation : rotations) {
        if (!isFirst) {
            result += "->";
        } else {
            isFirst = false;
        }
        result += glm::to_string(rotation);
    }

    std::cout << result << std::endl;
}

void updateCubeRotation(SmallCube &cubeRotation, glm::vec3 rotationVector, bool isDoneRotating) {
    cubeRotation.rotations[cubeRotation.rotations.size() - 1] = rotationVector;

    glm::mat4 cubeMatrix = glm::mat4(1.0F);
    for (auto &rotation : cubeRotation.rotations) {
        const auto normalizedRotation = glm::abs(glm::normalize(rotation));
        float rotationAngle = 0.0F;
        if (std::abs(normalizedRotation.x) > 0.0F) {
            rotationAngle = rotation.x;
        } else if (std::abs(normalizedRotation.y) > 0.0F) {
            rotationAngle = rotation.y;
        } else if (std::abs(normalizedRotation.z) > 0.0F) {
            rotationAngle = rotation.z;
        } else {
            std::cout << "Corrupted rotation vector! (" << rotation[0] << "," << rotation[1] << "," << rotation[2]
                      << ")" << std::endl;
            continue;
        }
        // applying accumulated rotations to rotation axis
        const auto rotationAxis = glm::vec3(glm::inverse(cubeMatrix) * glm::vec4(normalizedRotation, 1.0F));
        cubeMatrix = glm::rotate(cubeMatrix, rotationAngle, rotationAxis);
    }
    cubeRotation.rotationMatrix = cubeMatrix;

    if (isDoneRotating) {
        cubeRotation.rotations.emplace_back();
    }
}

Face rotateFaceBack(Face currentFace, glm::vec3 rotation) {
    if (rotation.x == 0.0F && rotation.y == 0.0F && rotation.z == 0.0F) {
        return currentFace;
    }

    if (rotation.x > 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][0];
    }
    if (rotation.x < 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][1];
    }
    if (rotation.y > 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][2];
    }
    if (rotation.y < 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][3];
    }
    if (rotation.z > 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][4];
    }
    if (rotation.z < 0.0F) {
        return NEXT_FACE_MAP[(int)currentFace - 1][5];
    }

    return currentFace;
}

unsigned int squashRotations(std::vector<glm::vec3> &rotations) {
    unsigned int removed = 0;
    int i = 0;
    int currentSize = rotations.size();
    while (currentSize > 0 && i < currentSize - 1) {
        auto &current = rotations.at(i);
        auto &next = rotations.at(i + 1);
        if ((current.x == -next.x && current.y == 0 && current.z == 0) ||
            (current.x == 0 && current.y == -next.y && current.z == 0) ||
            (current.x == 0 && current.y == 0 && current.z == -next.z)) {

            rotations.erase(rotations.begin() + i, rotations.begin() + (i + 2));
            i--;
            currentSize -= 2;
            removed += 2;
        }
        i++;
    }

    return removed;
}

unsigned int squashRotations(std::array<SmallCube, CUBELET_COUNT> &cubeRotations) {
    unsigned int removed = 0;
    for (auto &cube : cubeRotations) {
        removed += squashRotations(cube.rotations);
    }
    return removed;
}

Face getOppositeFace(Face face) {
    switch (face) {
    case Face::FRONT:
        return Face::BACK;
    case Face::BACK:
        return Face::FRONT;
    case Face::UP:
        return Face::DOWN;
    case Face::DOWN:
        return Face::UP;
    case Face::LEFT:
        return Face::RIGHT;
    case Face::RIGHT:
        return Face::LEFT;
    }
}

} // namespace rubiks
