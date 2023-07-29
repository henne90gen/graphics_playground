#pragma once

#include <array>

#include "RotationCommandStack.h"
#include "RubiksCubeData.h"
#include "RubiksCubeLogic.h"

namespace rubiks {
const unsigned int FACE_COUNT = 6;
const unsigned int SMALL_FACE_COUNT = 9;
extern const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, FACE_COUNT> WHOLE_CUBE;

enum class SolveStage { NOT_SOLVING, BOTTOM_LAYER };

class RubiksCube {
  public:
    explicit RubiksCube(const std::vector<RotationCommand> &initialCommands = {});

    void rotate(float rotationSpeed);

    void shuffle();

    /**
     * @brief Get the current face at the provided local index.
     *
     * This methods looks at the given face of the cube and returns to which face the piece at the given local index
     * belongs to.
     *
     * @param direction Global face to look at
     * @param localIndex Local index to look at
     * @return Face that is present at the selected position
     */
    Face getCurrentFaceAtLocalIndex(Face direction, unsigned int localIndex);

    glm::mat4 getRotationMatrix(unsigned int index) { return smallCubes[index].rotationMatrix; }

    unsigned int getMaximumRotationListLength();

    unsigned int getTotalRotationListEntriesCount();

    unsigned int getAverageRotationListLength();

    bool loop = false;
    unsigned int squashedRotations = 0;
    unsigned int executedRotationCommands = 0;

    void solve();

    void startSolving() { solveStage = SolveStage::BOTTOM_LAYER; }

  private:
    bool isRotating = true;
    float currentAngle = 0;
    RotationCommand currentCommand = {};

    std::vector<SmallCube> smallCubes;
    std::vector<unsigned int> positionMapping;

    RotationCommandStack rotationCommands;
    SolveStage solveStage = SolveStage::NOT_SOLVING;

    void solveBottomLayer();
};
} // namespace rubiks
