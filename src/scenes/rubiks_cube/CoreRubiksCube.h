#pragma once

#include <array>

#include "RubiksCube.h"

namespace rubiks {

struct Cubelet {};

struct CoreRubiksCube {
    std::array<Cubelet, CUBELET_COUNT> cubelets;
    std::array<unsigned int, CUBELET_COUNT> globalIndexToCubeletIndex;
    std::array<std::array<Face, SMALL_FACE_COUNT>, SIDE_COUNT> sideAndLocalIndexToFace;

    CoreRubiksCube();
    CoreRubiksCube(const std::vector<RotationCommand> &commands);

    /**
     * Applies the given RotationCommand to this Rubiks cube.
     */
    void rotate(RotationCommand cmd);

    /**
     * Applies the given sequence of RotationCommands to this Rubiks cube in the order they are given in.
     */
    void rotate(const std::vector<RotationCommand> &commands);

    /**
     * @brief Get the current face at the provided local index.
     *
     * This method looks at the given face of the cube and returns to which face the piece at the given local index
     * belongs to.
     *
     * @param side Side to look at
     * @param localIndex Local index to look at
     * @return Face that is present at the selected position
     */
    Face getCurrentFace(Face side, unsigned int localIndex);

    /**
     * Solves this Rubiks cube and returns the list of RotationCommands that were applied.
     * The cube is in a solved configuration after this method has been called, the same as calling the constructor CoreRubiksCube().
     * 
     * NOTE: This function is not yet fully implemented
     */
    std::vector<RotationCommand> solve();

  private:
    void init();

    void adjustCubeletIndicesClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);
    void adjustCubeletIndicesCounterClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);
    void adjustFaceIndicesClockwise(Face side);
    void adjustFaceIndicesCounterClockwise(Face side);

    std::vector<RotationCommand> solveBottomLayer();
    std::vector<RotationCommand> solveMiddleLayer();
    std::vector<RotationCommand> solveTopLayer();
};

} // namespace rubiks
