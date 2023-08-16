#include "AnimationRubiksCube.h"

namespace rubiks {

AnimationRubiksCube::AnimationRubiksCube(std::vector<RotationCommand> commands) : commands(commands) {
    for (unsigned int i = 0; i < CUBELET_COUNT; i++) {
        cubelets[i] = {glm::mat4(1.0F)};
    }

    this->commands = commands;
}

void AnimationRubiksCube::rotate(float rotationSpeed = 2.0F) {
    rotationSpeed = std::min(rotationSpeed, 2.0F);
    rotationSpeed = std::max(rotationSpeed, 0.0F);
}

void AnimationRubiksCube::shuffle() {
    // TODO implement this
}

void AnimationRubiksCube::solve() {}

} // namespace rubiks
