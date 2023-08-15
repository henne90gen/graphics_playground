#pragma once

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "RotationCommandStack.h"
#include "RubiksCubeData.h"

#include "RubiksCube.h"

namespace rubiks {
void printRotations(std::vector<glm::vec3> &rotations);

bool rotate(std::array<SmallCube, CUBELET_COUNT> &cubeRotations, std::array<unsigned int, CUBELET_COUNT> &cubePositions,
            RotationCommand command, float &currentAngle);

int getDirection(RotationCommand &rot);

void adjustCubeletIndicesClockwise(std::array<unsigned int, CUBELET_COUNT> &positions,
                                   std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);

void adjustCubeletIndicesCounterClockwise(std::array<unsigned int, CUBELET_COUNT> &positions,
                                          std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);

void updateCubeRotation(SmallCube &cubeRotation, glm::vec3 rotationVector, bool isDoneRotating);

Face rotateFaceBack(Face currentFace, glm::vec3 rotation);

unsigned int squashRotations(std::array<SmallCube, CUBELET_COUNT> &cubeRotations);

std::pair<Face, unsigned int> getEdgePartnerSide(Face face, unsigned int index);

Face getOppositeFace(Face face);
} // namespace rubiks
