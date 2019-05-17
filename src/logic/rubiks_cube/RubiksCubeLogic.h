#pragma once

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "RubiksCubeData.h"
#include "RotationCommandStack.h"

bool rotate(std::vector<SmallCube> &cubeRotations, std::vector<unsigned int> &cubePositions,
            RotationCommand command, float *currentAngle, float rotationSpeed = 0.1f);

int getDirection(RotationCommand &rot);

void adjustIndicesClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes);

void adjustIndicesCounterClockwise(std::vector<unsigned int> &positions, std::vector<unsigned int> &selectedCubes);

void updateCubeRotation(SmallCube &cubeRotation, glm::vec3 rotationVector, bool isDoneRotating);

Face rotateFace(Face currentFace, glm::vec3 rotation);
