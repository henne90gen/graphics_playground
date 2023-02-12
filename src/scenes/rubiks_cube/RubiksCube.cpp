#include "RubiksCube.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

namespace rubiks {
const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, FACE_COUNT> WHOLE_CUBE = {
      std::array<unsigned int, SMALL_FACE_COUNT>(FRONT_CUBES),
      BACK_CUBES,
      LEFT_CUBES,
      RIGHT_CUBES,
      TOP_CUBES,
      BOTTOM_CUBES};

RubiksCube::RubiksCube(const std::vector<RotationCommand> &initialCommands) {
    const unsigned int smallCubeCount = 27;
    for (unsigned int i = 0; i < smallCubeCount; i++) {
        SmallCube cubeRotation = {std::vector<glm::vec3>(), glm::mat4(1.0F)};
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

void RubiksCube::rotate(float rotationSpeed) {
    if (loop && !isRotating && rotationCommands.hasCommands()) {
        isRotating = true;
    }
    if (rotationCommands.hasBeenCleared() && rotationCommands.hasCommands()) {
        isRotating = true;
        currentCommand = rotationCommands.next();
    }

    bool shouldStartNextCommand =
          isRotating && rubiks::rotate(smallCubes, positionMapping, currentCommand, &currentAngle, rotationSpeed);
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

void RubiksCube::shuffle() {
    rotationCommands.clear();

    const unsigned int rotationCommandCount = 12;
    std::array<RotationCommand, rotationCommandCount> rotations = {
          RotationCommand(R_R), R_RI, R_F, R_FI, R_BO, R_BOI, R_L, R_LI, R_T, R_TI, R_BA, R_BAI};
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, rotations.size() - 1);

    const int shuffleCount = 20;
    for (unsigned int i = 0; i < shuffleCount; i++) {
        unsigned int randomIndex = distribution(generator);
        RotationCommand rotation = rotations[randomIndex]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        rotationCommands.push(rotation);
    }

    std::cout << "Shuffeling cube..." << std::endl;
    std::cout << rotationCommands.to_string() << std::endl;

    isRotating = true;
}

Face RubiksCube::getCurrentFace(Face direction, unsigned int localIndex) {
    if (localIndex == 4) {
        return direction;
    }
    unsigned int globalIndex =
          WHOLE_CUBE[direction][localIndex]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    unsigned int cubeIndex = positionMapping[globalIndex];
    SmallCube cube = smallCubes[cubeIndex];

    Face currentFace = direction;
    std::vector<glm::vec3> rotations(cube.rotations.size());
    std::reverse_copy(cube.rotations.begin(), cube.rotations.end(), rotations.begin());
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
    if (!solving) {
        return;
    }

    rotationCommands.clear();
    std::cout << "Solving..." << std::endl;

    // find miss-aligned bottom pieces
    struct EdgePiece {
        unsigned int localIndex;
        Face face;
    };
    const unsigned int EDGE_PIECE_COUNT = 4;
    std::array<EdgePiece, EDGE_PIECE_COUNT> edgePieces = {{
          {1, BACK},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {3, LEFT},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {5, RIGHT}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {7, FRONT}  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }};
    for (const EdgePiece &edgePiece : edgePieces) {
        const Face bottomFace = getCurrentFace(BOTTOM, edgePiece.localIndex);
        if (bottomFace == BOTTOM) {
            const Face edgePartnerFace = rubiks::getEdgePartnerFace(this, BOTTOM, edgePiece.localIndex);
            if (edgePartnerFace != edgePiece.face) {
                const Face oppositeFace = rubiks::getOppositeFace(edgePiece.face);
                std::cout << to_string(edgePartnerFace) << " " << to_string(oppositeFace) << std::endl;
                if (edgePartnerFace == oppositeFace) {
                    rotationCommands.push(R_BO);
                    rotationCommands.push(R_BO);
                } else {
                    rotationCommands.push(R_BO);
                }
                return;
            }
        }
    }

    std::cout << rotationCommands.to_string() << std::endl;
    // find the bottom edge pieces
    // move them to the correct position
}

} // namespace rubiks
