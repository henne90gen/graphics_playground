#pragma once

#include "RubiksCubeData.h"
#include "RubiksCubeLogic.h"
#include "RotationCommandStack.h"

extern unsigned int WHOLE_CUBE[6][9];

class RubiksCube {
public:
    explicit RubiksCube(const std::vector<RotationCommand> &initialCommands = {});

    void rotate(float rotationSpeed);

    void shuffle();

    Face getCurrentFace(Face direction, unsigned int localIndex);

    glm::mat4 getRotationMatrix(unsigned int index) {
        return smallCubes[index].rotationMatrix;
    }

    bool loop = true;
private:
    bool isRotating = true;
    float currentAngle = 0;
    RotationCommand currentCommand = {};

    std::vector<SmallCube> smallCubes;
    std::vector<unsigned int> positionMapping;

    RotationCommandStack rotationCommands;
};
