#pragma once

#include <array>

#include "RubiksCubeData.h"
#include "RubiksCubeLogic.h"
#include "RotationCommandStack.h"

const unsigned int FACE_COUNT = 6;
const unsigned int SMALL_FACE_COUNT = 9;
extern const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, FACE_COUNT> WHOLE_CUBE;

class RubiksCube {
public:
    explicit RubiksCube(const std::vector<RotationCommand> &initialCommands = {});

    void rotate(float rotationSpeed);

    void shuffle();

    Face getCurrentFace(Face direction, unsigned int localIndex);

    glm::mat4 getRotationMatrix(unsigned int index) {
        return smallCubes[index].rotationMatrix;
    }

    unsigned int getMaximumRotationListLength();

    unsigned int getTotalRotationListEntriesCount();

    unsigned int getAverageRotationListLength();

    bool loop = false;
    unsigned int squashedRotations = 0;
    unsigned int executedRotationCommands = 0;

    void solve();

private:
    bool isRotating = true;
    float currentAngle = 0;
    RotationCommand currentCommand = {};

    std::vector<SmallCube> smallCubes;
    std::vector<unsigned int> positionMapping;

    RotationCommandStack rotationCommands;
};
