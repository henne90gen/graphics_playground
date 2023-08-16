#include "RubiksCube.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

#include "RubiksCubeLogic.h"

namespace rubiks {
const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, SIDE_COUNT> WHOLE_CUBE = {
      std::array<unsigned int, SMALL_FACE_COUNT>(FRONT_CUBES),
      BACK_CUBES,
      LEFT_CUBES,
      RIGHT_CUBES,
      UP_CUBES,
      DOWN_CUBES};

RubiksCube::RubiksCube(const std::vector<RotationCommand> &initialCommands) {
    for (unsigned int i = 0; i < CUBELET_COUNT; i++) {
        SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4(1.0F)};
        cubeRotation.rotations.emplace_back();
        smallCubes[i] = cubeRotation;
        positionMapping[i] = i;
    }

    rotationCommands = RotationCommandStack();

    for (RotationCommand cmd : initialCommands) {
        rotationCommands.push(cmd);
    }

    isRotating = rotationCommands.hasCommands();
    if (isRotating) {
        currentCommand = rotationCommands.next();
    }
}

void RubiksCube::rotate(float rotationSpeed = 2.0F) {
    rotationSpeed = std::min(rotationSpeed, 2.0F);
    rotationSpeed = std::max(rotationSpeed, 0.0F);

    if (loop && !isRotating && rotationCommands.hasCommands()) {
        isRotating = true;
    }
    if (rotationCommands.hasBeenCleared() && rotationCommands.hasCommands()) {
        isRotating = true;
        currentCommand = rotationCommands.next();
    }

    currentAngle += rotationSpeed;
    const auto shouldStartNextCommand =
          isRotating && rubiks::rotate(smallCubes, positionMapping, currentCommand, currentAngle);
    if (!shouldStartNextCommand) {
        return;
    }

#if 0
    std::cout << "Rotated: " << to_string(currentCommand, true) << std::endl;
#endif

    executedRotationCommands++;

    squashedRotations += rubiks::squashRotations(smallCubes);

    currentAngle = 0.0F;
    if (rotationCommands.hasNext()) {
        currentCommand = rotationCommands.next();
    } else if (loop) {
        if (rotationCommands.hasCommands()) {
            currentCommand = rotationCommands.reset();
        }
    } else {
        isRotating = false;
    }
}

void RubiksCube::rotateAll() {
    isRotating = true;
    while (isRotating) {
        rotate();
    }
}

void RubiksCube::shuffle() {
    rotationCommands.clear();

    const auto rotationCommandCount = 12;
    std::array<RotationCommand, rotationCommandCount> rotations = {
          RotationCommand(R_R), R_RI, R_F, R_FI, R_D, R_DI, R_L, R_LI, R_U, R_UI, R_B, R_BI};
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, rotations.size() - 1);

    const int shuffleCount = 20;
    for (unsigned int i = 0; i < shuffleCount; i++) {
        unsigned int randomIndex = distribution(generator);
        RotationCommand rotation = rotations[randomIndex]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        rotationCommands.push(rotation);
    }

    std::cout << "Shuffling cube..." << std::endl;
    std::cout << rotationCommands.to_string() << std::endl;

    isRotating = true;
}

Face RubiksCube::getCurrentFace(Face side, unsigned int localIndex) {
    if (localIndex == 4) {
        return side;
    }

    unsigned int globalIndex = WHOLE_CUBE[(int)side - 1][localIndex];
    unsigned int cubeIndex = positionMapping[globalIndex];
    const auto smallCube = smallCubes[cubeIndex];

    Face currentFace = side;
    std::vector<glm::vec3> rotations(smallCube.rotations.size());
    std::reverse_copy(smallCube.rotations.begin(), smallCube.rotations.end(), rotations.begin());
    for (auto &rotation : rotations) {
        currentFace = rubiks::rotateFaceBack(currentFace, rotation);
    }

    return currentFace;
}

unsigned int RubiksCube::getMaximumRotationListLength() {
    unsigned int result = 0;
    for (auto &cube : smallCubes) {
        if (cube.rotations.size() > result) {
            result = cube.rotations.size();
        }
    }
    return result;
}

unsigned int RubiksCube::getTotalRotationListEntriesCount() {
    unsigned int result = 0;
    for (auto &cube : smallCubes) {
        result += cube.rotations.size();
    }
    return result;
}

unsigned int RubiksCube::getAverageRotationListLength() {
    unsigned int result = 0;
    for (auto &cube : smallCubes) {
        result += cube.rotations.size();
    }
    return result / smallCubes.size();
}

void RubiksCube::solve() {
    rotationCommands.clear();
    // TODO implement this
}

} // namespace rubiks
