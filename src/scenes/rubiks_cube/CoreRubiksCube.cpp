#include "CoreRubiksCube.h"

namespace rubiks {

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

int getDirection(RotationCommand &rot) {
    if (rot.direction == Direction::CLOCKWISE) {
        if (rot.side == Face::DOWN || rot.side == Face::LEFT || rot.side == Face::BACK) {
            return 1;
        }
        return -1;
    }

    if (rot.side == Face::DOWN || rot.side == Face::LEFT || rot.side == Face::BACK) {
        return -1;
    }

    return 1;
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
    constexpr std::array<std::array<std::array<int, SMALL_FACE_COUNT>, SIDE_COUNT>, SIDE_COUNT> arr = {
          frontSide, backSide, leftSide, rightSide, upSide, downSide,
    };
    return arr[(int)side - 1][(int)neighbor - 1][sideLocalIndex];
}

void CoreRubiksCube::adjustFaceIndicesClockwise(Face side) {
    /*
        Local face indices:
            0 1 2
            3 4 5
            6 7 8
    */
    auto sideArray = sideAndLocalIndexToFace[(int)side - 1];

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

} // namespace rubiks
