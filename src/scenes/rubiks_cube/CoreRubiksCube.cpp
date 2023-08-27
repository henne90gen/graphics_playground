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

CoreRubiksCube CoreRubiksCube::copy() {
    CoreRubiksCube result = {};
    result.globalIndexToCubeletIndex = globalIndexToCubeletIndex;
    result.sideAndLocalIndexToFace = sideAndLocalIndexToFace;
    return result;
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
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> frontSide = {{
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // FRONT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // BACK
          {2, -1, -1, 5, -1, -1, 8, -1, -1},    // LEFT
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},    // RIGHT
          {6, 7, 8, -1, -1, -1, -1, -1, -1},    // UP
          {-1, -1, -1, -1, -1, -1, 0, 1, 2},    // DOWN
    }};
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> backSide = {{
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // FRONT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // BACK
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},    // LEFT
          {2, -1, -1, 5, -1, -1, 8, -1, -1},    // RIGHT
          {2, 1, 0, -1, -1, -1, -1, -1, -1},    // UP
          {-1, -1, -1, -1, -1, -1, 8, 7, 6},    // DOWN
    }};
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> leftSide = {{
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},    // FRONT
          {2, -1, -1, 5, -1, -1, 8, -1, -1},    // BACK
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // LEFT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // RIGHT
          {0, 3, 6, -1, -1, -1, -1, -1, -1},    // UP
          {-1, -1, -1, -1, -1, -1, 6, 3, 0},    // DOWN
    }};
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> rightSide = {{
          {2, -1, -1, 5, -1, -1, 8, -1, -1},    // FRONT
          {-1, -1, 0, -1, -1, 3, -1, -1, 6},    // BACK
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // LEFT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // RIGHT
          {8, 5, 2, -1, -1, -1, -1, -1, -1},    // UP
          {-1, -1, -1, -1, -1, -1, 2, 5, 8},    // DOWN
    }};
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> upSide = {{
          {-1, -1, -1, -1, -1, -1, 0, 1, 2},    // FRONT
          {2, 1, 0, -1, -1, -1, -1, -1, -1},    // BACK
          {0, -1, -1, 1, -1, -1, 2, -1, -1},    // LEFT
          {-1, -1, 2, -1, -1, 1, -1, -1, 0},    // RIGHT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // UP
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // DOWN
    }};
    constexpr std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT> downSide = {{
          {6, 7, 8, -1, -1, -1, -1, -1, -1},    // FRONT
          {-1, -1, -1, -1, -1, -1, 8, 7, 6},    // BACK
          {8, -1, -1, 7, -1, -1, 6, -1, -1},    // LEFT
          {-1, -1, 6, -1, -1, 7, -1, -1, 8},    // RIGHT
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // UP
          {-1, -1, -1, -1, -1, -1, -1, -1, -1}, // DOWN
    }};
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

Face CoreRubiksCube::getCurrentFace(const std::pair<Face, unsigned int> &pair) {
    return sideAndLocalIndexToFace[(int)pair.first - 1][pair.second];
}

std::vector<RotationCommand> CoreRubiksCube::solve() {
    auto result = std::vector<RotationCommand>();

    solveBottomLayer(result);
    solveMiddleLayer(result);
    solveTopLayer(result);

    return result;
}

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

    // use [side][(localIndex-1)/2] to index into this array
    constexpr std::array<std::array<std::pair<Face, unsigned int>, 4>, 7> edgePartnerLocalIndices = {{
          {std::make_pair(Face::UP, 7), {Face::LEFT, 5}, {Face::RIGHT, 3}, {Face::DOWN, 1}},    // FRONT
          {std::make_pair(Face::UP, 1), {Face::RIGHT, 5}, {Face::LEFT, 3}, {Face::DOWN, 7}},    // BACK
          {std::make_pair(Face::UP, 3), {Face::BACK, 5}, {Face::FRONT, 3}, {Face::DOWN, 3}},    // LEFT
          {std::make_pair(Face::UP, 5), {Face::FRONT, 5}, {Face::BACK, 3}, {Face::DOWN, 5}},    // RIGHT
          {std::make_pair(Face::BACK, 1), {Face::LEFT, 1}, {Face::RIGHT, 1}, {Face::FRONT, 1}}, // UP
          {std::make_pair(Face::FRONT, 7), {Face::LEFT, 7}, {Face::RIGHT, 7}, {Face::BACK, 7}}, // DOWN
    }};
    return edgePartnerLocalIndices[(int)side - 1][index];
}

/**
 * The resulting array contains the given (side, localIndex) pair at the first position and its partners in clockwise
 * rotation around the corner next.
 */
std::array<std::pair<Face, unsigned int>, 3> getCornerPartners(Face side, unsigned int localIndex) {
    constexpr std::array<std::array<std::pair<Face, unsigned int>, 3>, 8> corners = {{
          // FRONT
          {std::make_pair(Face::FRONT, 0), {Face::LEFT, 2}, {Face::UP, 6}},    // TOP-LEFT
          {std::make_pair(Face::FRONT, 2), {Face::UP, 8}, {Face::RIGHT, 0}},   // TOP-RIGHT
          {std::make_pair(Face::FRONT, 6), {Face::DOWN, 0}, {Face::LEFT, 8}},  // BOTTOM-LEFT
          {std::make_pair(Face::FRONT, 8), {Face::RIGHT, 6}, {Face::DOWN, 2}}, // BOTTOM-RIGHT

          // BACK
          {std::make_pair(Face::BACK, 0), {Face::RIGHT, 2}, {Face::UP, 2}},   // TOP-LEFT
          {std::make_pair(Face::BACK, 2), {Face::UP, 0}, {Face::LEFT, 0}},    // TOP-RIGHT
          {std::make_pair(Face::BACK, 8), {Face::LEFT, 6}, {Face::DOWN, 6}},  // BOTTOM-RIGHT
          {std::make_pair(Face::BACK, 6), {Face::DOWN, 8}, {Face::RIGHT, 8}}, // BOTTOM-LEFT
    }};

    for (const auto &corner : corners) {
        int foundCornerIndex = -1;
        for (int i = 0; i < corner.size(); i++) {
            const auto &[face, localIdx] = corner[i];
            if (face == side && localIdx == localIndex) {
                foundCornerIndex = i;
                break;
            }
        }
        if (foundCornerIndex == -1) {
            continue;
        }

        std::array<std::pair<Face, unsigned int>, 3> result = {};
        for (int i = 0; i < 3; i++) {
            result[i] = corner[(foundCornerIndex + i) % 3];
        }
        return result;
    }

    // TODO do error handling
    return {};
}

void CoreRubiksCube::solveCreateBottomCross(std::vector<RotationCommand> &result) {
    const auto localRotate = [this, &result](RotationCommand cmd) {
        rotate(cmd);
        result.push_back(cmd);
    };

    const auto bottomEdgePiecesAreCorrect = [this]() {
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
        unsigned int localIndex = 0;
        Face side = Face::NONE;
        Face expectedEdgePartnerFace = Face::NONE;
    };
    constexpr unsigned int EDGE_PIECE_COUNT = 24;
    constexpr std::array<EdgePiece, EDGE_PIECE_COUNT> edgePieces = {{
          {1, Face::UP},                //
          {3, Face::UP},                //
          {5, Face::UP},                //
          {7, Face::UP},                //
          {3, Face::FRONT},             //
          {5, Face::FRONT},             //
          {7, Face::FRONT},             //
          {3, Face::LEFT},              //
          {5, Face::LEFT},              //
          {7, Face::LEFT},              //
          {3, Face::RIGHT},             //
          {5, Face::RIGHT},             //
          {7, Face::RIGHT},             //
          {3, Face::BACK},              //
          {5, Face::BACK},              //
          {7, Face::BACK},              //
          {1, Face::FRONT},             //
          {1, Face::LEFT},              //
          {1, Face::RIGHT},             //
          {1, Face::BACK},              //
          {1, Face::DOWN, Face::FRONT}, //
          {3, Face::DOWN, Face::LEFT},  //
          {5, Face::DOWN, Face::RIGHT}, //
          {7, Face::DOWN, Face::BACK},  //
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
                break;
            }

            if (edgePiece.side == Face::LEFT || edgePiece.side == Face::RIGHT || edgePiece.side == Face::FRONT ||
                edgePiece.side == Face::BACK) {
                if (edgePiece.localIndex == 3 || edgePiece.localIndex == 5) {
                    // face is at one of the sides and needs to be moved to the top
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
                    localRotate({edgePartner.first,
                                 edgePiece.localIndex == 3 ? Direction::COUNTER_CLOCKWISE : Direction::CLOCKWISE});
                    localRotate({Face::UP, Direction::CLOCKWISE});
                    localRotate({edgePartner.first,
                                 edgePiece.localIndex == 3 ? Direction::CLOCKWISE : Direction::COUNTER_CLOCKWISE});
                } else if (edgePiece.localIndex == 1) {
                    // face is at the top, but upside down
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, 5);
                    localRotate({edgePiece.side, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::CLOCKWISE});
                    localRotate({Face::UP, Direction::COUNTER_CLOCKWISE});
                    localRotate({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                    localRotate({edgePiece.side, Direction::COUNTER_CLOCKWISE});
                } else if (edgePiece.localIndex == 7) {
                    // face is at the bottom, but upside down
                    const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, 3);
                    localRotate({edgePiece.side, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::COUNTER_CLOCKWISE});
                    localRotate({Face::UP, Direction::CLOCKWISE});
                    localRotate({edgePartner.first, Direction::CLOCKWISE});
                } else {
                    // TODO make sure this is never reached
                }
                break;
            }

            // face is now at the top
            const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
            const auto edgePartnerCurrentFace = getCurrentFace(edgePartner.first, edgePartner.second);
            if (edgePartner.first != edgePartnerCurrentFace) {
                // TODO use negative rotationCount to signal counter clockwise rotations
                constexpr std::array<std::array<int, 7>, 7> rotationCount = {{
                      {0, 2, 3, 1, 0, 0}, // FRONT
                      {2, 0, 1, 3, 0, 0}, // BACK
                      {1, 3, 0, 2, 0, 0}, // LEFT
                      {3, 1, 2, 0, 0, 0}, // RIGHT
                      {0, 0, 0, 0, 0, 0}, // UP
                      {0, 0, 0, 0, 0, 0}, // DOWN
                }};
                const auto count = rotationCount[(int)edgePartnerCurrentFace - 1][(int)edgePartner.first - 1];
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
    const auto localRotate = [this, &result](RotationCommand cmd) {
        rotate(cmd);
        result.push_back(cmd);
    };

    const auto bottomCornerPiecesAreCorrect = [this]() {
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
        unsigned int localIndex = 0;
        Face side = Face::NONE;
        Face expectedCornerPartnerFace0 = Face::NONE;
        Face expectedCornerPartnerFace1 = Face::NONE;
    };
    constexpr unsigned int CORNER_PIECE_COUNT = 24;
    // first check all pieces on the top layer, then move on to the bottom pieces
    constexpr std::array<CornerPiece, CORNER_PIECE_COUNT> cornerPieces = {{
          {0, Face::UP},                             //
          {2, Face::UP},                             //
          {6, Face::UP},                             //
          {8, Face::UP},                             //
          {0, Face::FRONT},                          //
          {2, Face::FRONT},                          //
          {0, Face::LEFT},                           //
          {2, Face::LEFT},                           //
          {0, Face::BACK},                           //
          {2, Face::BACK},                           //
          {0, Face::RIGHT},                          //
          {2, Face::RIGHT},                          //
          {6, Face::FRONT},                          //
          {8, Face::FRONT},                          //
          {6, Face::LEFT},                           //
          {8, Face::LEFT},                           //
          {6, Face::BACK},                           //
          {8, Face::BACK},                           //
          {6, Face::RIGHT},                          //
          {8, Face::RIGHT},                          //
          {0, Face::DOWN, Face::LEFT, Face::FRONT},  //
          {2, Face::DOWN, Face::FRONT, Face::RIGHT}, //
          {6, Face::DOWN, Face::BACK, Face::LEFT},   //
          {8, Face::DOWN, Face::RIGHT, Face::BACK},  //
    }};

    const auto cornerIsAtCorrectPosition = [this](const CornerPiece &cp) {
        const auto cornerPartners = getCornerPartners(cp.side, cp.localIndex);
        const auto face1 = getCurrentFace(cornerPartners[1].first, cornerPartners[1].second);
        const auto face2 = getCurrentFace(cornerPartners[2].first, cornerPartners[2].second);

        if (cp.side == Face::UP) {
            return face1 == cornerPartners[2].first && face2 == cornerPartners[1].first;
        } else if (cornerPartners[1].first == Face::UP) {
            return face1 == cp.side && face2 == cornerPartners[2].first;
        } else if (cornerPartners[2].first == Face::UP) {
            return face1 == cornerPartners[1].first && face2 == cp.side;
        }

        // TODO make sure this is not reached
        return false;
    };

    while (!bottomCornerPiecesAreCorrect()) {
        for (const auto &cornerPiece : cornerPieces) {
            const auto pieceFace = getCurrentFace(cornerPiece.side, cornerPiece.localIndex);
            if (pieceFace != Face::DOWN) {
                // face of piece is not a bottom face
                continue;
            }

            if (cornerPiece.side == Face::DOWN) {
                const auto cornerPartners = getCornerPartners(cornerPiece.side, cornerPiece.localIndex);
                const auto cornerPartner1 = cornerPartners[1];
                const auto cornerPartner2 = cornerPartners[2];
                const auto cornerPartner1CurrentFace = getCurrentFace(cornerPartner1.first, cornerPartner1.second);
                const auto cornerPartner2CurrentFace = getCurrentFace(cornerPartner2.first, cornerPartner2.second);
                if (cornerPartner1CurrentFace == cornerPiece.expectedCornerPartnerFace0 &&
                    cornerPartner2CurrentFace == cornerPiece.expectedCornerPartnerFace1) {
                    // piece is already at the correct position
                    continue;
                }

                // move the piece to the top layer
                localRotate({cornerPartners[1].first, Direction::COUNTER_CLOCKWISE});
                localRotate(R_UI);
                localRotate({cornerPartners[1].first, Direction::CLOCKWISE});
                break;
            }

            if ((cornerPiece.side == Face::FRONT || cornerPiece.side == Face::LEFT || cornerPiece.side == Face::BACK ||
                 cornerPiece.side == Face::RIGHT) &&
                (cornerPiece.localIndex == 6 || cornerPiece.localIndex == 8)) {
                // corners of the bottom layer need to be moved to the top
                const auto cornerPartners = getCornerPartners(cornerPiece.side, cornerPiece.localIndex);
                const auto rotationFace =
                      cornerPartners[1].first == Face::DOWN ? cornerPartners[2].first : cornerPiece.side;
                localRotate({rotationFace, Direction::COUNTER_CLOCKWISE});
                localRotate(R_UI);
                localRotate({rotationFace, Direction::CLOCKWISE});
                break;
            }

            // the remaining pieces are now at the top
            constexpr std::array<Face, 6> nextSidesClockwise = {{
                  Face::LEFT,  // FRONT
                  Face::RIGHT, // BACK
                  Face::BACK,  // LEFT
                  Face::FRONT, // RIGHT
                  Face::NONE,  // UP
                  Face::NONE,  // DOWN
            }};
            constexpr std::array<int, 9> nextLocalIndexClockwise = {{
                  2, 5, 8, //
                  1, 4, 7, //
                  0, 3, 6, //
            }};
            auto currentCornerPiece = cornerPiece;
            while (!cornerIsAtCorrectPosition(currentCornerPiece)) {
                localRotate(R_U);
                if (currentCornerPiece.side != Face::UP && currentCornerPiece.side != Face::DOWN) {
                    currentCornerPiece.side = nextSidesClockwise[(int)currentCornerPiece.side - 1];
                } else {
                    currentCornerPiece.localIndex = nextLocalIndexClockwise[currentCornerPiece.localIndex];
                }
            }

            const auto cornerPartners = getCornerPartners(currentCornerPiece.side, currentCornerPiece.localIndex);
            int rotationCount = 0;
            auto rotationFace = Face::NONE;
            if (currentCornerPiece.side != Face::UP) {
                if (currentCornerPiece.localIndex == 2) {
                    rotationCount = 1;
                    rotationFace = currentCornerPiece.side;
                } else if (currentCornerPiece.localIndex == 0) {
                    rotationCount = 5;
                    rotationFace = cornerPartners[1].first;
                } else {
                    // TODO make sure this is never reached
                }
            } else {
                rotationCount = 3;
                rotationFace = cornerPartners[2].first;
            }
            for (int i = 0; i < rotationCount; i++) {
                localRotate({rotationFace, Direction::COUNTER_CLOCKWISE});
                localRotate(R_UI);
                localRotate({rotationFace, Direction::CLOCKWISE});
                localRotate(R_U);
            }
            break;
        }
    }
}

void CoreRubiksCube::solveBottomLayer(std::vector<RotationCommand> &result) {
    solveCreateBottomCross(result);
    solveBottomCornerPieces(result);
}

void CoreRubiksCube::solveMiddleLayer(std::vector<RotationCommand> &result) {
    const auto localRotate = [this, &result](RotationCommand cmd) {
        rotate(cmd);
        result.push_back(cmd);
    };

    const auto middleEdgePiecesAreCorrect = [this]() {
        return getCurrentFace(Face::FRONT, 3) == Face::FRONT && //
               getCurrentFace(Face::FRONT, 5) == Face::FRONT && //
               getCurrentFace(Face::LEFT, 3) == Face::LEFT &&   //
               getCurrentFace(Face::LEFT, 5) == Face::LEFT &&   //
               getCurrentFace(Face::BACK, 3) == Face::BACK &&   //
               getCurrentFace(Face::BACK, 5) == Face::BACK &&   //
               getCurrentFace(Face::RIGHT, 3) == Face::RIGHT && //
               getCurrentFace(Face::RIGHT, 5) == Face::RIGHT;
    };

    // find miss-aligned middle edge pieces
    struct EdgePiece {
        unsigned int localIndex = 0;
        Face side = Face::NONE;
        Face expectedEdgePartnerFace = Face::NONE;
    };
    constexpr unsigned int EDGE_PIECE_COUNT = 24;
    constexpr std::array<EdgePiece, EDGE_PIECE_COUNT> edgePieces = {{
          {1, Face::FRONT},              //
          {1, Face::LEFT},               //
          {1, Face::RIGHT},              //
          {1, Face::BACK},               //
          {3, Face::FRONT, Face::LEFT},  //
          {5, Face::FRONT, Face::RIGHT}, //
          {3, Face::LEFT, Face::BACK},   //
          {5, Face::LEFT, Face::FRONT},  //
          {3, Face::RIGHT, Face::FRONT}, //
          {5, Face::RIGHT, Face::BACK},  //
          {3, Face::BACK, Face::RIGHT},  //
          {5, Face::BACK, Face::LEFT},   //
    }};

    const auto edgePieceFaceMatchesEdgePieceSide = [this](EdgePiece &ep) {
        auto epFace = getCurrentFace(ep.side, ep.localIndex);
        return ep.side == epFace;
    };

    while (!middleEdgePiecesAreCorrect()) {
        for (const auto &edgePiece : edgePieces) {
            const auto pieceFace = getCurrentFace(edgePiece.side, edgePiece.localIndex);
            const auto edgePartner = getEdgePartnerSideAndLocalIndex(edgePiece.side, edgePiece.localIndex);
            const auto edgePartnerFace = getCurrentFace(edgePartner);
            if (edgePiece.side == pieceFace && edgePartner.first == edgePartnerFace) {
                // edge piece is already at the correct position
                continue;
            }

            if (pieceFace == Face::UP || edgePartnerFace == Face::UP) {
                // we are not interested in UP faces
                continue;
            }

            if ((edgePiece.side == Face::FRONT || edgePiece.side == Face::LEFT || edgePiece.side == Face::BACK ||
                 edgePiece.side == Face::RIGHT) &&
                edgePiece.localIndex != 1) {
                // edge piece is in the middle layer but not correctly positioned
                auto leftRotationFace = edgePiece.side;
                auto rightRotationFace = edgePartner.first;
                if (edgePiece.localIndex == 3) {
                    leftRotationFace = edgePartner.first;
                    rightRotationFace = edgePiece.side;
                }
                localRotate({leftRotationFace, Direction::COUNTER_CLOCKWISE});
                localRotate(R_U);
                localRotate({leftRotationFace, Direction::CLOCKWISE});
                localRotate(R_U);
                localRotate({rightRotationFace, Direction::CLOCKWISE});
                localRotate(R_UI);
                localRotate({rightRotationFace, Direction::COUNTER_CLOCKWISE});
                break;
            }

            // piece is at the top and can now be dropped into its correct position
            constexpr std::array<Face, 6> nextSidesClockwise = {{
                  Face::LEFT,  // FRONT
                  Face::RIGHT, // BACK
                  Face::BACK,  // LEFT
                  Face::FRONT, // RIGHT
                  Face::NONE,  // UP
                  Face::NONE,  // DOWN
            }};
            auto currentEdgePiece = edgePiece;
            while (!edgePieceFaceMatchesEdgePieceSide(currentEdgePiece)) {
                localRotate(R_U);
                currentEdgePiece.side = nextSidesClockwise[(int)currentEdgePiece.side - 1];
            }

            const auto newEdgePartner =
                  getEdgePartnerSideAndLocalIndex(currentEdgePiece.side, currentEdgePiece.localIndex);
            const auto newEdgePartnerFace = getCurrentFace(newEdgePartner);
            const auto leftEdgePartner = getEdgePartnerSideAndLocalIndex(currentEdgePiece.side, 3);
            const auto rightEdgePartner = getEdgePartnerSideAndLocalIndex(currentEdgePiece.side, 5);
            if (leftEdgePartner.first == newEdgePartnerFace) {
                // piece needs to be dropped to the left
                localRotate(R_UI);
                localRotate({leftEdgePartner.first, Direction::COUNTER_CLOCKWISE});
                localRotate(R_U);
                localRotate({leftEdgePartner.first, Direction::CLOCKWISE});
                localRotate(R_U);
                localRotate({currentEdgePiece.side, Direction::CLOCKWISE});
                localRotate(R_UI);
                localRotate({currentEdgePiece.side, Direction::COUNTER_CLOCKWISE});
            } else if (rightEdgePartner.first == newEdgePartnerFace) {
                // piece needs to be dropped to the right
                localRotate(R_U);
                localRotate({rightEdgePartner.first, Direction::CLOCKWISE});
                localRotate(R_UI);
                localRotate({rightEdgePartner.first, Direction::COUNTER_CLOCKWISE});
                localRotate(R_UI);
                localRotate({currentEdgePiece.side, Direction::COUNTER_CLOCKWISE});
                localRotate(R_U);
                localRotate({currentEdgePiece.side, Direction::CLOCKWISE});
            } else {
                // TODO make sure this is never reached
            }
            break;
        }
    }
}

void CoreRubiksCube::solveCreateTopCross(std::vector<RotationCommand> &result) {
    const auto localRotate = [this, &result](RotationCommand cmd) {
        rotate(cmd);
        result.push_back(cmd);
    };

    if (getCurrentFace(Face::UP, 1) != Face::UP && //
        getCurrentFace(Face::UP, 3) != Face::UP && //
        getCurrentFace(Face::UP, 5) != Face::UP && //
        getCurrentFace(Face::UP, 7) != Face::UP) {
        // only the center of the cross is there
        localRotate(R_F);
        localRotate(R_U);
        localRotate(R_R);
        localRotate(R_UI);
        localRotate(R_RI);
        localRotate(R_FI);
    }

    if (getCurrentFace(Face::UP, 1) == Face::UP && //
        getCurrentFace(Face::UP, 3) != Face::UP && //
        getCurrentFace(Face::UP, 5) != Face::UP && //
        getCurrentFace(Face::UP, 7) == Face::UP) {
        // straight line from FRONT to BACK
        localRotate(R_U);
    }

    if (getCurrentFace(Face::UP, 1) != Face::UP && //
        getCurrentFace(Face::UP, 3) == Face::UP && //
        getCurrentFace(Face::UP, 5) == Face::UP && //
        getCurrentFace(Face::UP, 7) != Face::UP) {
        // straight line from LEFT to RIGHT
        localRotate(R_F);
        localRotate(R_U);
        localRotate(R_R);
        localRotate(R_UI);
        localRotate(R_RI);
        localRotate(R_FI);
    }

    if (getCurrentFace(Face::UP, 1) != Face::UP && //
        getCurrentFace(Face::UP, 3) == Face::UP && //
        getCurrentFace(Face::UP, 5) != Face::UP && //
        getCurrentFace(Face::UP, 7) == Face::UP) {
        // right angle at the bottom left
        localRotate(R_U);
    }

    if (getCurrentFace(Face::UP, 1) != Face::UP && //
        getCurrentFace(Face::UP, 3) != Face::UP && //
        getCurrentFace(Face::UP, 5) == Face::UP && //
        getCurrentFace(Face::UP, 7) == Face::UP) {
        // right angle at the bottom right
        localRotate(R_U);
        localRotate(R_U);
    }

    if (getCurrentFace(Face::UP, 1) == Face::UP && //
        getCurrentFace(Face::UP, 3) != Face::UP && //
        getCurrentFace(Face::UP, 5) == Face::UP && //
        getCurrentFace(Face::UP, 7) != Face::UP) {
        // right angle at the top right
        localRotate(R_UI);
    }

    if (getCurrentFace(Face::UP, 1) == Face::UP && //
        getCurrentFace(Face::UP, 3) == Face::UP && //
        getCurrentFace(Face::UP, 5) != Face::UP && //
        getCurrentFace(Face::UP, 7) != Face::UP) {
        // right angle at the top left
        localRotate(R_F);
        localRotate(R_U);
        localRotate(R_R);
        localRotate(R_UI);
        localRotate(R_RI);
        localRotate(R_FI);
    }

    // at this point we have the cross on top

    const auto faceAndSideMatch = [this](Face side, unsigned int localIndex) {
        const auto edgePartner = getEdgePartnerSideAndLocalIndex(side, localIndex);
        const auto edgePartnerFace = getCurrentFace(edgePartner);
        return edgePartner.first == edgePartnerFace;
    };
    const auto edgePartner1 = getEdgePartnerSideAndLocalIndex(Face::UP, 1);
    const auto edgePartner3 = getEdgePartnerSideAndLocalIndex(Face::UP, 3);
    const auto edgePartner5 = getEdgePartnerSideAndLocalIndex(Face::UP, 5);
    const auto edgePartner7 = getEdgePartnerSideAndLocalIndex(Face::UP, 7);
    if (faceAndSideMatch(Face::UP, 1) && //
        faceAndSideMatch(Face::UP, 3) && //
        faceAndSideMatch(Face::UP, 5) && //
        faceAndSideMatch(Face::UP, 7)) {
        // top cross is already correctly oriented
        return;
    }

    constexpr std::array<Face, 6> nextSidesClockwise = {{
          Face::LEFT,  // FRONT
          Face::RIGHT, // BACK
          Face::BACK,  // LEFT
          Face::FRONT, // RIGHT
          Face::NONE,  // UP
          Face::NONE,  // DOWN
    }};
    constexpr std::array<Face, 6> expectedPartners = {{
          Face::BACK,  // FRONT
          Face::FRONT, // BACK
          Face::RIGHT, // LEFT
          Face::LEFT,  // RIGHT
          Face::DOWN,  // UP
          Face::UP,    // DOWN
    }};
    const auto leftFace = getCurrentFace(Face::LEFT, 1);
    const auto rightFace = getCurrentFace(Face::RIGHT, 1);
    const auto frontFace = getCurrentFace(Face::FRONT, 1);
    const auto backFace = getCurrentFace(Face::BACK, 1);
    const auto leftFaceExpectedPartner = expectedPartners[(int)leftFace - 1];
    const auto frontFaceExpectedPartner = expectedPartners[(int)frontFace - 1];
    if (leftFaceExpectedPartner == rightFace && //
        frontFaceExpectedPartner == backFace && //
        nextSidesClockwise[(int)leftFace - 1] != backFace) {
        // faces on opposite sides are correct, but faces on adjacent sides are not correct
        localRotate(R_R);
        localRotate(R_U);
        localRotate(R_RI);
        localRotate(R_U);
        localRotate(R_R);
        localRotate(R_U);
        localRotate(R_U);
        localRotate(R_RI);
    }
}

void CoreRubiksCube::solveTopLayer(std::vector<RotationCommand> &result) { solveCreateTopCross(result); }

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
    default:
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
