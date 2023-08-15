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
      UP_CUBES,
      DOWN_CUBES};

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

Face RubiksCube::getCurrentFaceAtLocalIndex(Face face, unsigned int localIndex) {
    if (localIndex == 4) {
        return face;
    }

    unsigned int globalIndex = WHOLE_CUBE[(int)face - 1][localIndex];
    unsigned int cubeIndex = positionMapping[globalIndex];
    const auto smallCube = smallCubes[cubeIndex];

    Face currentFace = face;
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

void RubiksCube::solveBottomLayer() {
    // find miss-aligned bottom pieces
    struct EdgePiece {
        unsigned int localIndex;
        Face side;
        Face expectedEdgePartnerFace = Face::NONE;
    };
    const unsigned int EDGE_PIECE_COUNT = 24;
    const std::array<EdgePiece, EDGE_PIECE_COUNT> edgePieces = {{
          {1, Face::UP},                //
          {3, Face::UP},                //
          {5, Face::UP},                //
          {7, Face::UP},                //
          {1, Face::FRONT},             //
          {3, Face::FRONT},             //
          {5, Face::FRONT},             //
          {7, Face::FRONT},             //
          {1, Face::LEFT},              //
          {3, Face::LEFT},              //
          {5, Face::LEFT},              //
          {7, Face::LEFT},              //
          {1, Face::RIGHT},             //
          {3, Face::RIGHT},             //
          {5, Face::RIGHT},             //
          {7, Face::RIGHT},             //
          {1, Face::BACK},              //
          {3, Face::BACK},              //
          {5, Face::BACK},              //
          {7, Face::BACK},              //
          {1, Face::DOWN, Face::BACK},  //
          {3, Face::DOWN, Face::LEFT},  //
          {5, Face::DOWN, Face::RIGHT}, //
          {7, Face::DOWN, Face::FRONT}, //
    }};

    // find a piece that is at the top or generate moves to get one to the top
    for (const EdgePiece &edgePiece : edgePieces) {
        const Face pieceFace = getCurrentFaceAtLocalIndex(edgePiece.side, edgePiece.localIndex);
        if (pieceFace != Face::DOWN) {
            // face of piece is not a bottom face
            continue;
        }

        if (edgePiece.side == Face::DOWN) {
            const auto edgePartner = getEdgePartnerSide(edgePiece.side, edgePiece.localIndex);
            const auto edgePartnerCurrentFace = getCurrentFaceAtLocalIndex(edgePartner.first, edgePartner.second);
            if (edgePartnerCurrentFace == edgePiece.expectedEdgePartnerFace) {
                // piece is already at the correct position
                continue;
            }

            // face is at the bottom, but the neighboring side does not match, thus needs to be moved to the top
            rotationCommands.push({edgePiece.expectedEdgePartnerFace, Direction::CLOCKWISE});
            rotationCommands.push({edgePiece.expectedEdgePartnerFace, Direction::CLOCKWISE});
            return;
        }

        if (edgePiece.side == Face::LEFT || edgePiece.side == Face::RIGHT || edgePiece.side == Face::FRONT ||
            edgePiece.side == Face::BACK) {
            if (edgePiece.localIndex == 3 || edgePiece.localIndex == 5) {
                // face is at one of the sides and needs to be moved to the top
                const auto edgePartner = getEdgePartnerSide(edgePiece.side, 5);
                rotationCommands.push({edgePartner.first, edgePiece.localIndex == 3 ? Direction::COUNTER_CLOCKWISE
                                                                                    : Direction::CLOCKWISE});
                return;
            }

            if (edgePiece.localIndex == 1) {
                // face is at the bottom, but upside down
                const auto edgePartner = getEdgePartnerSide(edgePiece.side, 5);
                rotationCommands.push({edgePiece.side, Direction::CLOCKWISE});
                rotationCommands.push({edgePartner.first, Direction::CLOCKWISE});
                rotationCommands.push({Face::UP, Direction::CLOCKWISE});
                rotationCommands.push({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                return;
            }

            if (edgePiece.localIndex == 7) {
                // face is at the top, but upside down
                const auto edgePartner = getEdgePartnerSide(edgePiece.side, 5);
                rotationCommands.push({edgePiece.side, Direction::COUNTER_CLOCKWISE});
                rotationCommands.push({edgePartner.first, Direction::CLOCKWISE});
                rotationCommands.push({Face::UP, Direction::CLOCKWISE});
                rotationCommands.push({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                return;
            }

            assert(false);
        }

        // face is now at the top
        const auto edgePartner = getEdgePartnerSide(edgePiece.side, edgePiece.localIndex);
        const auto edgePartnerCurrentFace = getCurrentFaceAtLocalIndex(edgePartner.first, edgePartner.second);
        if (edgePartner.first != edgePartnerCurrentFace) {
            // TODO use negative rotationCount to signal counter clockwise rotations
            const std::array<std::array<int, 7>, 7> rotationCount = {
                  std::array<int, 7>(),  //
                  {0, 0, 2, 3, 1, 0, 0}, // FRONT
                  {0, 2, 0, 1, 3, 0, 0}, // BACK
                  {0, 1, 3, 0, 2, 0, 0}, // LEFT
                  {0, 3, 1, 2, 0, 0, 0}, // RIGHT
                  {0, 0, 0, 0, 0, 0, 0}, // UP
                  {0, 0, 0, 0, 0, 0, 0}, // DOWN
            };
            const auto count = rotationCount[(int)edgePartnerCurrentFace][(int)edgePartner.first];
            for (int i = 0; i < count; i++) {
                rotationCommands.push(R_U);
            }
        }

        rotationCommands.push({edgePartnerCurrentFace, Direction::CLOCKWISE});
        rotationCommands.push({edgePartnerCurrentFace, Direction::CLOCKWISE});
        return;
    }

    std::cout << rotationCommands.to_string() << std::endl;
    // find the bottom edge pieces
    // move them to the correct position
}

void RubiksCube::solve() {
    if (solveStage == SolveStage::NOT_SOLVING) {
        return;
    }

    rotationCommands.clear();
    std::cout << "Solving..." << std::endl;

    switch (solveStage) {
    case SolveStage::NOT_SOLVING:
    default:
        return;
    case SolveStage::BOTTOM_LAYER:
        solveBottomLayer();
        return;
    }
}

} // namespace rubiks
