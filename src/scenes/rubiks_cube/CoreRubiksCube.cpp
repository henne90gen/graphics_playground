#include "CoreRubiksCube.h"

namespace rubiks {

const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, SIDE_COUNT> WHOLE_CUBE = {
      std::array<unsigned int, SMALL_FACE_COUNT>(FRONT_CUBES),
      BACK_CUBES,
      LEFT_CUBES,
      RIGHT_CUBES,
      UP_CUBES,
      DOWN_CUBES,
};

CoreRubiksCube::CoreRubiksCube() { init(); }

CoreRubiksCube::CoreRubiksCube(const std::vector<RotationCommand> &commands) {
    init();
    rotate(commands);
}

void CoreRubiksCube::init() {
    for (unsigned int i = 0; i < CUBELET_COUNT; i++) {
        globalIndexToCubeletIndex[i] = i;
    }
    std::array<Face, SIDE_COUNT> sides = {Face::FRONT, Face::BACK, Face::LEFT, Face::RIGHT, Face::UP, Face::DOWN};
    for (int i = 0; i < sides.size(); i++) {
        for (int j = 0; j < SMALL_FACE_COUNT; j++) {
            sideAndLocalIndexToFace[i][j] = sides[i];
        }
    }
}

void CoreRubiksCube::adjustCubeletIndicesClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */

    auto &indices = globalIndexToCubeletIndex;

    // move the corners
    unsigned int tmp1 = indices[selectedCubes[2]];
    indices[selectedCubes[2]] = indices[selectedCubes[0]];
    unsigned int tmp2 = indices[selectedCubes[8]];
    indices[selectedCubes[8]] = tmp1;
    tmp1 = indices[selectedCubes[6]];
    indices[selectedCubes[6]] = tmp2;
    indices[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = indices[selectedCubes[5]];
    indices[selectedCubes[5]] = indices[selectedCubes[1]];
    tmp2 = indices[selectedCubes[7]];
    indices[selectedCubes[7]] = tmp1;
    tmp1 = indices[selectedCubes[3]];
    indices[selectedCubes[3]] = tmp2;
    indices[selectedCubes[1]] = tmp1;
}

void CoreRubiksCube::adjustCubeletIndicesCounterClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */

    auto &indices = globalIndexToCubeletIndex;

    // move the corners
    unsigned int tmp1 = indices[selectedCubes[6]];
    indices[selectedCubes[6]] = indices[selectedCubes[0]];
    unsigned int tmp2 = indices[selectedCubes[8]];
    indices[selectedCubes[8]] = tmp1;
    tmp1 = indices[selectedCubes[2]];
    indices[selectedCubes[2]] = tmp2;
    indices[selectedCubes[0]] = tmp1;

    // move the edges
    tmp1 = indices[selectedCubes[3]];
    indices[selectedCubes[3]] = indices[selectedCubes[1]];
    tmp2 = indices[selectedCubes[7]];
    indices[selectedCubes[7]] = tmp1;
    tmp1 = indices[selectedCubes[5]];
    indices[selectedCubes[5]] = tmp2;
    indices[selectedCubes[1]] = tmp1;
}

int getNeighboringLocalIndex(Face side, Face neighbor, unsigned int sideLocalIndex) {
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> frontSide = {
          std::array<int, SMALL_FACE_COUNT>({-1, -1, -1, -1, -1, -1, -1, -1, -1}), // FRONT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                    // BACK
          {2, -1, -1, 5, -1, -1, 8, -1, -1},                                       // LEFT
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},                                       // RIGHT
          {6, 7, 8, -1, -1, -1, -1, -1, -1},                                       // UP
          {-1, -1, -1, -1, -1, -1, 0, 1, 2},                                       // DOWN
    };
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> backSide = {
          std::array<int, SMALL_FACE_COUNT>({-1, -1, -1, -1, -1, -1, -1, -1, -1}), // FRONT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                    // BACK
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},                                       // LEFT
          {2, -1, -1, 5, -1, -1, 8, -1, -1},                                       // RIGHT
          {2, 1, 0, -1, -1, -1, -1, -1, -1},                                       // UP
          {-1, -1, -1, -1, -1, -1, 8, 7, 6},                                       // DOWN
    };
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> leftSide = {
          std::array<int, SMALL_FACE_COUNT>({-1, -1, 0, -1, -1, 3, -1, -1, 6}), // FRONT
          {2, -1, -1, 5, -1, -1, 8, -1, -1},                                    // BACK
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // LEFT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // RIGHT
          {0, 3, 6, -1, -1, -1, -1, -1, -1},                                    // UP
          {-1, -1, -1, -1, -1, -1, 6, 3, 0},                                    // DOWN
    };
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> rightSide = {
          std::array<int, SMALL_FACE_COUNT>({2, -1, -1, 5, -1, -1, 8, -1, -1}), // FRONT
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},                                    // BACK
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // LEFT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // RIGHT
          {8, 5, 2, -1, -1, -1, -1, -1, -1},                                    // UP
          {-1, -1, -1, -1, -1, -1, 2, 5, 8},                                    // DOWN
    };
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> upSide = {
          std::array<int, SMALL_FACE_COUNT>({-1, -1, -1, -1, -1, -1, 0, 1, 2}), // FRONT
          {2, 1, 0, -1, -1, -1, -1, -1, -1},                                    // BACK
          {0, -1, -1, 1, -1, -1, 2, -1, -1},                                    // LEFT
          {-1, -1, 2, -1, -1, 1, -1, -1, 0},                                    // RIGHT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // UP
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // DOWN
    };
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> downSide = {
          std::array<int, SMALL_FACE_COUNT>({6, 7, 8, -1, -1, -1, -1, -1, -1}), // FRONT
          {-1, -1, -1, -1, -1, -1, 8, 7, 6},                                    // BACK
          {8, -1, -1, 7, -1, -1, 6, -1, -1},                                    // LEFT
          {-1, -1, 6, -1, -1, 7, -1, -1, 8},                                    // RIGHT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // UP
          {-1, -1, -1, -1, -1, -1, -1, -1, -1},                                 // DOWN
    };
    constexpr std::array<std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT>, SIDE_COUNT> allSides = {
          frontSide, backSide, leftSide, rightSide, upSide, downSide,
    };
    return allSides[(int)side - 1][(int)neighbor - 1][sideLocalIndex];
}

void CoreRubiksCube::adjustFaceIndicesClockwise(Face side) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */
    auto &sideArray = sideAndLocalIndexToFace[(int)side - 1];

    // move the corners
    auto tmp1 = sideArray[2];
    sideArray[2] = sideArray[0];
    auto tmp2 = sideArray[8];
    sideArray[8] = tmp1;
    tmp1 = sideArray[6];
    sideArray[6] = tmp2;
    sideArray[0] = tmp1;

    // move the edges
    tmp1 = sideArray[5];
    sideArray[5] = sideArray[1];
    tmp2 = sideArray[7];
    sideArray[7] = tmp1;
    tmp1 = sideArray[3];
    sideArray[3] = tmp2;
    sideArray[1] = tmp1;

    std::array<Face, 4> sidesToSwitch;
    switch (side) {
    case Face::FRONT:
        sidesToSwitch = {Face::UP, Face::RIGHT, Face::DOWN, Face::LEFT};
        break;
    case Face::BACK:
        sidesToSwitch = {Face::UP, Face::LEFT, Face::DOWN, Face::RIGHT};
        break;
    case Face::LEFT:
        sidesToSwitch = {Face::UP, Face::FRONT, Face::DOWN, Face::BACK};
        break;
    case Face::RIGHT:
        sidesToSwitch = {Face::UP, Face::BACK, Face::DOWN, Face::FRONT};
        break;
    case Face::UP:
        sidesToSwitch = {Face::BACK, Face::RIGHT, Face::FRONT, Face::LEFT};
        break;
    case Face::DOWN:
        sidesToSwitch = {Face::FRONT, Face::RIGHT, Face::BACK, Face::LEFT};
        break;
    case Face::NONE:
        // TODO do error handling
        break;
    }

    std::array<std::array<int, 3>, 4> localIndicesToTraverse = {
          std::array<int, 3>({0, 1, 2}),
          {2, 5, 8},
          {8, 7, 6},
          {6, 3, 0},
    };

    auto lastSideToSwitch = sidesToSwitch[sidesToSwitch.size() - 1];
    auto localIndex0 = getNeighboringLocalIndex(side, lastSideToSwitch, 6);
    auto localIndex1 = getNeighboringLocalIndex(side, lastSideToSwitch, 3);
    auto localIndex2 = getNeighboringLocalIndex(side, lastSideToSwitch, 0);
    std::array<Face, 3> current = {
          getCurrentFace(lastSideToSwitch, localIndex0),
          getCurrentFace(lastSideToSwitch, localIndex1),
          getCurrentFace(lastSideToSwitch, localIndex2),
    };
    for (int i = 0; i < sidesToSwitch.size(); i++) {
        auto sideToSwitch = sidesToSwitch[i];
        auto localIndices = localIndicesToTraverse[i];
        for (int j = 0; j < localIndices.size(); j++) {
            auto localIndex = getNeighboringLocalIndex(side, sideToSwitch, localIndices[j]);
            auto &faceToSwap = sideAndLocalIndexToFace[(int)sideToSwitch - 1][localIndex];
            auto tmp = faceToSwap;
            faceToSwap = current[j];
            current[j] = tmp;
        }
    }
}

void CoreRubiksCube::adjustFaceIndicesCounterClockwise(Face side) {
    // TODO copy "adjustFaceIndicesClockwise" and change it to work counter clockwise
    for (int i = 0; i < 3; i++) {
        adjustFaceIndicesClockwise(side);
    }
}

void CoreRubiksCube::rotate(RotationCommand cmd) {
    std::array<unsigned int, SMALL_FACE_COUNT> cubes;
    switch (cmd.side) {
    case Face::FRONT:
        cubes = FRONT_CUBES;
        break;
    case Face::BACK:
        cubes = BACK_CUBES;
        break;
    case Face::LEFT:
        cubes = LEFT_CUBES;
        break;
    case Face::RIGHT:
        cubes = RIGHT_CUBES;
        break;
    case Face::UP:
        cubes = UP_CUBES;
        break;
    case Face::DOWN:
        cubes = DOWN_CUBES;
        break;
    case Face::NONE:
        // TODO do error handling
        break;
    }

    if (cmd.direction == Direction::CLOCKWISE) {
        adjustCubeletIndicesClockwise(cubes);
        adjustFaceIndicesClockwise(cmd.side);
    } else {
        adjustCubeletIndicesCounterClockwise(cubes);
        adjustFaceIndicesCounterClockwise(cmd.side);
    }
}

void CoreRubiksCube::rotate(const std::vector<RotationCommand> &commands) {
    for (auto &cmd : commands) {
        rotate(cmd);
    }
}

Face CoreRubiksCube::getCurrentFace(Face side, unsigned int localIndex) {
    return sideAndLocalIndexToFace[(int)side - 1][localIndex];
}

std::vector<RotationCommand> CoreRubiksCube::solve() {
    auto result = std::vector<RotationCommand>();

    solveBottomLayer(result);
    solveMiddleLayer(result);
    solveTopLayer(result);

    return result;
}

// use [side][(localIndex-1)/2] to index into this array
std::array<std::array<std::pair<Face, unsigned int>, 4>, 7> edgePartnerLocalIndices = {
      std::array<std::pair<Face, unsigned int>, 4>(
            {std::make_pair(Face::UP, 7), {Face::LEFT, 5}, {Face::RIGHT, 3}, {Face::DOWN, 1}}), // FRONT
      {std::make_pair(Face::UP, 1), {Face::RIGHT, 5}, {Face::LEFT, 3}, {Face::DOWN, 7}},        // BACK
      {std::make_pair(Face::UP, 3), {Face::BACK, 5}, {Face::FRONT, 3}, {Face::DOWN, 3}},        // LEFT
      {std::make_pair(Face::UP, 5), {Face::FRONT, 5}, {Face::BACK, 3}, {Face::DOWN, 5}},        // RIGHT
      {std::make_pair(Face::BACK, 1), {Face::LEFT, 1}, {Face::RIGHT, 1}, {Face::FRONT, 1}},     // UP
      {std::make_pair(Face::FRONT, 7), {Face::LEFT, 7}, {Face::RIGHT, 7}, {Face::BACK, 7}},     // DOWN
};
/**
 * This function only returns valid results for edge pieces. Corner pieces are not supported.
 */
std::pair<Face, unsigned int> getEdgePartnerSideAndLocalIndex(Face side, unsigned int localIndex) {
    const auto tmp = (localIndex - 1);
    if (tmp % 2 != 0) {
        return std::make_pair(Face::NONE, 0);
    }
    const auto index = (localIndex - 1) / 2;
    if (index < 0 || index >= 4) {
        return std::make_pair(Face::NONE, 0);
    }

    return edgePartnerLocalIndices[(int)side - 1][index];
}

std::pair<Face, unsigned int> getCornerPartnerSideAndLocalIndex(Face side, unsigned int localIndex) {
    return std::make_pair(Face::NONE, 0);
}

void CoreRubiksCube::solveCreateBottomCross(std::vector<RotationCommand> &result) {
    auto localRotate = [this, &result](RotationCommand cmd) {
        rotate(cmd);
        result.push_back(cmd);
    };

    auto bottomEdgePiecesAreCorrect = [this]() {
        return getCurrentFace(Face::DOWN, 1) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 3) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 5) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 7) == Face::DOWN &&   //
               getCurrentFace(Face::FRONT, 7) == Face::FRONT && //
               getCurrentFace(Face::LEFT, 7) == Face::LEFT &&   //
               getCurrentFace(Face::BACK, 7) == Face::BACK &&   //
               getCurrentFace(Face::RIGHT, 7) == Face::RIGHT;
    };

    // find miss-aligned bottom edge pieces
    struct EdgePiece {
        unsigned int localIndex;
        Face side;
        Face expectedEdgePartnerFace = Face::NONE;
    };
    constexpr unsigned int EDGE_PIECE_COUNT = 24;
    constexpr std::array<EdgePiece, EDGE_PIECE_COUNT> edgePieces = {{
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

    while (!bottomEdgePiecesAreCorrect()) {
        for (const auto &edgePiece : edgePieces) {
            // find a piece that is at the top or generate moves to get one to the top
            const auto pieceFace = getCurrentFace(edgePiece.side, edgePiece.localIndex);
            if (pieceFace != Face::DOWN) {
                // face of piece is not a bottom face
                continue;
            }

            if (edgePiece.side == Face::DOWN) {
                const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
                const auto edgePartnerCurrentFace = getCurrentFace(edgePartner.first, edgePartner.second);
                if (edgePartnerCurrentFace == edgePiece.expectedEdgePartnerFace) {
                    // piece is already at the correct position
                    continue;
                }

                // face is at the bottom, but the neighboring side does not match, thus needs to be moved to the top
                localRotate({edgePiece.expectedEdgePartnerFace, Direction::CLOCKWISE});
                localRotate({edgePiece.expectedEdgePartnerFace, Direction::CLOCKWISE});
            }

            if (edgePiece.side == Face::LEFT || edgePiece.side == Face::RIGHT || edgePiece.side == Face::FRONT ||
                edgePiece.side == Face::BACK) {
                if (edgePiece.localIndex == 3 || edgePiece.localIndex == 5) {
                    // face is at one of the sides and needs to be moved to the top
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
                    localRotate({edgePartner.first,
                                 edgePiece.localIndex == 3 ? Direction::COUNTER_CLOCKWISE : Direction::CLOCKWISE});
                } else if (edgePiece.localIndex == 1) {
                    // face is at the bottom, but upside down
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
                    localRotate({edgePiece.side, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::CLOCKWISE});
                    localRotate({Face::UP, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                } else if (edgePiece.localIndex == 7) {
                    // face is at the top, but upside down
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
                    localRotate({edgePiece.side, Direction::COUNTER_CLOCKWISE});
                    localRotate({edgePartner.first, Direction::CLOCKWISE});
                    localRotate({Face::UP, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                } else {
                    // TODO add assertion that fails if this point is reached
                }
            }

            // face is now at the top
            const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
            const auto edgePartnerCurrentFace = getCurrentFace(edgePartner.first, edgePartner.second);
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
                    localRotate(R_U);
                }
            }

            localRotate({edgePartnerCurrentFace, Direction::CLOCKWISE});
            localRotate({edgePartnerCurrentFace, Direction::CLOCKWISE});
            break;
        }
    }
}

void CoreRubiksCube::solveBottomCornerPieces(std::vector<RotationCommand> &result) {
    auto bottomCornerPiecesAreCorrect = [this]() {
        return getCurrentFace(Face::DOWN, 0) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 2) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 6) == Face::DOWN &&   //
               getCurrentFace(Face::DOWN, 8) == Face::DOWN &&   //
               getCurrentFace(Face::FRONT, 6) == Face::FRONT && //
               getCurrentFace(Face::FRONT, 8) == Face::FRONT && //
               getCurrentFace(Face::LEFT, 6) == Face::LEFT &&   //
               getCurrentFace(Face::LEFT, 8) == Face::LEFT &&   //
               getCurrentFace(Face::BACK, 6) == Face::BACK &&   //
               getCurrentFace(Face::BACK, 8) == Face::BACK &&   //
               getCurrentFace(Face::RIGHT, 6) == Face::RIGHT && //
               getCurrentFace(Face::RIGHT, 8) == Face::RIGHT;
    };

    // find the bottom corner pieces
    struct CornerPiece {
        unsigned int localIndex;
        Face side;
        Face expectedCornerPartnerFace0 = Face::NONE;
        Face expectedCornerPartnerFace1 = Face::NONE;
    };
    constexpr unsigned int CORNER_PIECE_COUNT = 24;
    constexpr std::array<CornerPiece, CORNER_PIECE_COUNT> cornerPieces = {{
          {0, Face::FRONT},                          //
          {2, Face::FRONT},                          //
          {6, Face::FRONT},                          //
          {8, Face::FRONT},                          //
          {0, Face::LEFT},                           //
          {2, Face::LEFT},                           //
          {6, Face::LEFT},                           //
          {8, Face::LEFT},                           //
          {0, Face::BACK},                           //
          {2, Face::BACK},                           //
          {6, Face::BACK},                           //
          {8, Face::BACK},                           //
          {0, Face::RIGHT},                          //
          {2, Face::RIGHT},                          //
          {6, Face::RIGHT},                          //
          {8, Face::RIGHT},                          //
          {0, Face::UP},                             //
          {2, Face::UP},                             //
          {6, Face::UP},                             //
          {8, Face::UP},                             //
          {0, Face::DOWN, Face::LEFT, Face::FRONT},  //
          {2, Face::DOWN, Face::FRONT, Face::RIGHT}, //
          {6, Face::DOWN, Face::BACK, Face::LEFT},   //
          {8, Face::DOWN, Face::RIGHT, Face::BACK},  //
    }};

    /*
        while (!bottomCornerPiecesAreCorrect()) {
            for (const auto &cornerPiece : cornerPieces) {
                const auto pieceFace = getCurrentFace(cornerPiece.side, cornerPiece.localIndex);
                if (pieceFace != Face::DOWN) {
                    // face of piece is not a bottom face
                    continue;
                }
            }
        }
    */
}

void CoreRubiksCube::solveBottomLayer(std::vector<RotationCommand> &result) {
    solveCreateBottomCross(result);
    solveBottomCornerPieces(result);
}

void CoreRubiksCube::solveMiddleLayer(std::vector<RotationCommand> &result) {}
void CoreRubiksCube::solveTopLayer(std::vector<RotationCommand> &result) {}

} // namespace rubiks

std::string to_string(const rubiks::Face &face, bool simple) {
    switch (face) {
    case rubiks::Face::FRONT:
        if (simple) {
            return "R_F";
        } else {
            return "FRONT";
        }
    case rubiks::Face::BACK:
        if (simple) {
            return "R_B";
        } else {
            return "BACK";
        }
    case rubiks::Face::LEFT:
        if (simple) {
            return "R_L";
        } else {
            return "LEFT";
        }
    case rubiks::Face::RIGHT:
        if (simple) {
            return "R_R";
        } else {
            return "RIGHT";
        }
    case rubiks::Face::UP:
        if (simple) {
            return "R_U";
        } else {
            return "UP";
        }
    case rubiks::Face::DOWN:
        if (simple) {
            return "R_D";
        } else {
            return "DOWN";
        }
    case rubiks::Face::NONE:
        return "NONE";
    }
}

std::string to_string(const rubiks::Direction &dir, bool simple) {
    if (dir == rubiks::Direction::CLOCKWISE) {
        if (simple) {
            return "";
        }
        return "CLOCKWISE";
    }
    if (simple) {
        return "I";
    }
    return "COUNTER_CLOCKWISE";
}

std::string to_string(const rubiks::RotationCommand &cmd, bool simple) {
    std::string separator;
    if (simple) {
        separator = "";
    } else {
        separator = ", ";
    }
    return to_string(cmd.side, simple) + separator + to_string(cmd.direction, simple);
}
