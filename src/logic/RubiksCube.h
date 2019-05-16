#pragma once

#include "RubiksCubeData.h"
#include "RubiksCubeLogic.h"
#include "RotationCommandStack.h"

class RubiksCube {
public:
    RubiksCube() {
        for (unsigned int i = 0; i < 27; i++) {
            CubeRotation cubeRotation = {
                    std::vector<glm::vec3>(),
                    glm::mat4(1.0f)
            };
            cubeRotation.rotations.emplace_back();
            cubeRotations.push_back(cubeRotation);

            cubePositions.emplace_back(i);
        }

        rotationCommands = RotationCommandStack();

        isRotating = rotationCommands.hasCommands();
        if (isRotating) {
            currentCommand = rotationCommands.next();
        }
    }

    void rotate(float rotationSpeed) {
        if (isRotating && ::rotate(cubeRotations, cubePositions, currentCommand, &currentAngle, rotationSpeed)) {
            currentAngle = 0.0f;
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
    }

    void shuffle();

    glm::mat4 getRotationMatrix(unsigned int index) {
        return cubeRotations[index].rotationMatrix;
    }

    bool loop = true;
private:
    bool isRotating = true;
    float currentAngle = 0;
    RotationCommand currentCommand = {};

    std::vector<CubeRotation> cubeRotations;
    std::vector<unsigned int> cubePositions;

    RotationCommandStack rotationCommands;
};
