#pragma once

#include <array>
#include <glm/ext.hpp>
#include <vector>

#include "CoreRubiksCube.h"

namespace rubiks {

struct Cubelet {
    glm::mat4 rotationMatrix;
};

struct AnimationRubiksCube {
    AnimationRubiksCube() = default;
    AnimationRubiksCube(std::vector<RotationCommand> commands);

    /**
     * Executes the rotations that have been added to the rotation command queue.
     * Calling this function with the default rotationSpeed of 2.0F results in one full rotation being executed.
     * Any larger value will be clamped to 2.0F, which means that this function can only ever execute one rotation
     * at a time.
     * Providing a smaller rotationSpeed results in a partial rotation.
     * Should be called every frame to create a continuous animation.
     * @param rotationSpeed Determines how fast the rotation should be executed. Is clamped to the range
     * [0.0, 2.0].
     */
    void rotate(float rotationSpeed);

    /**
     * Shuffles the rubiks cube by generating a random sequence of RotatationCommands.
     * Should only be called once, not every frame.
     */
    void shuffle();

    /**
     * Saves the commands required to solve this rubiks cube and plays them back in order.
     * Should only be called once, not every frame.
     */
    void solve();

    /**
     * Toggles the pause state of the animation.
     * Should only be called once, not every frame.
     */
    void togglePause() { isAnimationPaused = !isAnimationPaused; }

    /**
     * Returns the rotation matrix for the cubelet at the given global index.
     */
    glm::mat4 getCubeletRotationMatrix(unsigned int globalIndex) { return cubelets[globalIndex].rotationMatrix; }

  private:
    CoreRubiksCube cube = {};
    std::array<Cubelet, CUBELET_COUNT> cubelets = {};
    std::vector<RotationCommand> commands = {};
    bool isAnimationPaused = false;
    float currentAngle = 0.0F;
    int currentCommandIndex = -1;
};

} // namespace rubiks
