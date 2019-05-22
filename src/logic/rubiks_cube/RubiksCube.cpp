#include "RubiksCube.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

unsigned int WHOLE_CUBE[6][9] = {FRONT_CUBES, BACK_CUBES, LEFT_CUBES, RIGHT_CUBES, TOP_CUBES, BOTTOM_CUBES};

RubiksCube::RubiksCube(const std::vector<RotationCommand> &initialCommands) {
    for (unsigned int i = 0; i < 27; i++) {
        SmallCube cubeRotation = {
                std::vector<glm::vec3>(),
                glm::mat4(1.0F)
        };
        cubeRotation.rotations.emplace_back();
        smallCubes.push_back(cubeRotation);

        positionMapping.emplace_back(i);
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

std::string to_string(Face face) {
    switch (face) {
        case FRONT:
            return "FRONT";
        case BACK:
            return "BACK";
        case LEFT:
            return "LEFT";
        case RIGHT:
            return "RIGHT";
        case TOP:
            return "TOP";
        case BOTTOM:
            return "BOTTOM";
    }
}

std::string to_string(Direction dir) {
    switch (dir) {
        case CLOCKWISE:
            return "CLOCKWISE";
        case COUNTER_CLOCKWISE:
            return "COUNTER_CLOCKWISE";
    }
}

std::string to_string(RotationCommand cmd) {
    return to_string(cmd.face) + ", " + to_string(cmd.direction);
}

void RubiksCube::rotate(float rotationSpeed) {
    if (loop && !isRotating && rotationCommands.hasCommands()) {
        isRotating = true;
    }

    bool shouldStartNextCommand =
            isRotating && ::rotate(smallCubes, positionMapping, currentCommand, &currentAngle, rotationSpeed);
    if (!shouldStartNextCommand) {
        return;
    }

    executedRotationCommands++;

    squashedRotations += squashRotations(smallCubes);

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

void RubiksCube::shuffle() {
    rotationCommands.clear();
    RotationCommand rotations[] = {R_R, R_RI, R_F, R_FI, R_BO, R_BOI, R_L, R_LI, R_T, R_TI, R_BA, R_BAI};
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, 11);
    for (unsigned int i = 0; i < 20; i++) {
        unsigned int randomIndex = distribution(generator);
        rotationCommands.push(rotations[randomIndex]);
    }
    std::cout << "Shuffeling cube..." << std::endl;

    isRotating = true;
}

Face RubiksCube::getCurrentFace(Face direction, unsigned int localIndex) {
    if (localIndex == 4) {
        return direction;
    }
    unsigned int globalIndex = WHOLE_CUBE[direction][localIndex];
    unsigned int cubeIndex = positionMapping[globalIndex];
    SmallCube cube = smallCubes[cubeIndex];

    Face currentFace = direction;
    std::vector<glm::vec3> rotations(cube.rotations.size());
    std::reverse_copy(cube.rotations.begin(), cube.rotations.end(), rotations.begin());
    for (auto &rotation : rotations) {
        currentFace = rotateFaceBack(currentFace, rotation);
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
