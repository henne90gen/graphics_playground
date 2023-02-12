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
      {TOP, BOTTOM, LEFT, RIGHT, FRONT, FRONT},   // FRONT
      {BOTTOM, TOP, RIGHT, LEFT, BACK, BACK},     // BACK
      {LEFT, LEFT, BACK, FRONT, TOP, BOTTOM},     // LEFT
      {RIGHT, RIGHT, FRONT, BACK, BOTTOM, TOP},   // RIGHT
      {BACK, FRONT, TOP, TOP, RIGHT, LEFT},       // TOP
      {FRONT, BACK, BOTTOM, BOTTOM, LEFT, RIGHT}, // BOTTOM
}};

bool rotate(std::vector<SmallCube> &cubeRotations, std::vector<unsigned int> &cubePositions, RotationCommand command,
            float *currentAngle, float rotationSpeed) {
    *currentAngle += rotationSpeed;

    bool isDoneRotating = false;
    const auto piHalf = glm::pi<float>() * 0.5F;
    if (*currentAngle >= piHalf) {
        isDoneRotating = true;
        *currentAngle = piHalf;
    }

    const unsigned int cubeCount = 9;
    std::vector<unsigned int> cubes(cubeCount);
    glm::vec3 rotationVector;
    auto direction = static_cast<float>(rubiks::getDirection(command));
    switch (command.face) {
    case FRONT:
        cubes = FRONT_CUBES;
        rotationVector = glm::vec3(0, 0, direction * *currentAngle);
        break;
    case BACK:
        cubes = BACK_CUBES;
        rotationVector = glm::vec3(0, 0, direction * *currentAngle);
        break;
    case LEFT:
        cubes = LEFT_CUBES;
        rotationVector = glm::vec3(direction * *currentAngle, 0, 0);
        break;
    case RIGHT:
        cubes = RIGHT_CUBES;
        rotationVector = glm::vec3(direction * *currentAngle, 0, 0);
        break;
    case TOP:
        cubes = TOP_CUBES;
        rotationVector = glm::vec3(0, direction * *currentAngle, 0);
        break;
    case BOTTOM:
        cubes = BOTTOM_CUBES;
        rotationVector = glm::vec3(0, direction * *currentAngle, 0);
        break;
    }

    for (const auto cube : cubes) {
        const auto cubeIndex = cubePositions[cube];
        rubiks::updateCubeRotation(cubeRotations[cubeIndex], rotationVector, isDoneRotating);
    }

    if (!isDoneRotating) {
        return isDoneRotating;
    }

    if (command.direction == CLOCKWISE) {
        rubiks::adjustIndicesClockwise(cubePositions, cubes);
    } else {
        rubiks::adjustIndicesCounterClockwise(cubePositions, cubes);
    }

    return isDoneRotating;
}

int getDirection(RotationCommand &rot) {
    if (rot.direction == CLOCKWISE) {
        if (rot.face == BOTTOM || rot.face == LEFT || rot.face == BACK) {
            return 1;
        }
        return -1;
    }

    if (rot.face == BOTTOM || rot.face == LEFT || rot.face == BACK) {
        return -1;
    }

    return 1;
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
        glm::vec3 normalizedRotation = glm::abs(glm::normalize(rotation));
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
        glm::vec3 rotationAxis = glm::vec3(glm::inverse(cubeMatrix) * glm::vec4(normalizedRotation, 1.0F));
        cubeMatrix = glm::rotate(cubeMatrix, rotationAngle, rotationAxis);
    }
    cubeRotation.rotationMatrix = cubeMatrix;

    if (isDoneRotating) {
        cubeRotation.rotations.emplace_back();
    }
}

void adjustIndicesCounterClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes) {
    // move the corners
    unsigned int tmp1 = positions[selectedCubes[2]];           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[2]] = positions[selectedCubes[0]]; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    unsigned int tmp2 = positions[selectedCubes[8]];           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[8]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp1 = positions[selectedCubes[6]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[6]] = tmp2;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[0]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    // move the edges
    tmp1 = positions[selectedCubes[5]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[5]] = positions[selectedCubes[1]]; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp2 = positions[selectedCubes[7]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[7]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp1 = positions[selectedCubes[3]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[3]] = tmp2;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[1]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

void adjustIndicesClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes) {
    // move the corners
    unsigned int tmp1 = positions[selectedCubes[6]];           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[6]] = positions[selectedCubes[0]]; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    unsigned int tmp2 = positions[selectedCubes[8]];           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[8]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp1 = positions[selectedCubes[2]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[2]] = tmp2;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[0]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    // move the edges
    tmp1 = positions[selectedCubes[3]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[3]] = positions[selectedCubes[1]]; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp2 = positions[selectedCubes[7]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[7]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    tmp1 = positions[selectedCubes[5]];                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[5]] = tmp2;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    positions[selectedCubes[1]] = tmp1;                        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

Face rotateFaceBack(Face currentFace, glm::vec3 rotation) {
    if (rotation.x == 0.0F && rotation.y == 0.0F && rotation.z == 0.0F) {
        return currentFace;
    }

    if (rotation.x > 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [0]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    if (rotation.x < 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [1]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    if (rotation.y > 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [2]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    if (rotation.y < 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [3]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    if (rotation.z > 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [4]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    if (rotation.z < 0.0F) {
        return NEXT_FACE_MAP
              [currentFace]
              [5]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-constant-array-index)
    }
    return currentFace;
}

unsigned int squashRotations(std::vector<glm::vec3> *rotations) {
    unsigned int removed = 0;
    int i = 0;
    int currentSize = rotations->size();
    while (currentSize > 0 && i < currentSize - 1) {
        auto &current = rotations->at(i);
        auto &next = rotations->at(i + 1);
        if ((current.x == -next.x && current.y == 0 && current.z == 0) ||
            (current.x == 0 && current.y == -next.y && current.z == 0) ||
            (current.x == 0 && current.y == 0 && current.z == -next.z)) {

            rotations->erase(rotations->begin() + i, rotations->begin() + (i + 2));
            i--;
            currentSize -= 2;
            removed += 2;
        }
        i++;
    }

    return removed;
}

unsigned int squashRotations(std::vector<SmallCube> &cubeRotations) {
    unsigned int removed = 0;
    for (auto &cube : cubeRotations) {
        removed += squashRotations(&cube.rotations);
    }
    return removed;
}

Face getEdgePartnerFace(rubiks::RubiksCube *cube, Face face, unsigned int index) {
    if (face == BOTTOM) {
        switch (index) {
        case 1: // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            return cube->getCurrentFace(BACK, 1);
        case 3: // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            return cube->getCurrentFace(LEFT, 1);
        case 5: // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            return cube->getCurrentFace(RIGHT, 1);
        case 7: // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            return cube->getCurrentFace(FRONT, 1);
        default:
            throw std::exception();
        }
    }
    throw std::exception();
}

Face getOppositeFace(Face face) {
    switch (face) {
    case FRONT:
        return BACK;
    case BACK:
        return FRONT;
    case TOP:
        return BOTTOM;
    case BOTTOM:
        return TOP;
    case LEFT:
        return RIGHT;
    case RIGHT:
        return LEFT;
    }
}

} // namespace rubiks
