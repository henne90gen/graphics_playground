#include "AnimationRubiksCube.h"

#include "RubiksCubeLogic.h"

namespace rubiks {

AnimationRubiksCube::AnimationRubiksCube(std::vector<RotationCommand> commands) : commands(commands) {
    for (unsigned int i = 0; i < CUBELET_COUNT; i++) {
        cubelets[i] = {glm::mat4(1.0F)};
    }

    this->commands = commands;
    currentCommandIndex = 0;
}

void updateCubeletRotation(Cubelet &cubelet, glm::vec3 rotationVector) {
    const auto normalizedRotation = glm::abs(glm::normalize(rotationVector));
    float rotationAngle = 0.0F;
    if (std::abs(normalizedRotation.x) > 0.0F) {
        rotationAngle = rotationVector.x;
    } else if (std::abs(normalizedRotation.y) > 0.0F) {
        rotationAngle = rotationVector.y;
    } else if (std::abs(normalizedRotation.z) > 0.0F) {
        rotationAngle = rotationVector.z;
    } else {
        // TODO do error handling
        // std::cerr << "Corrupted rotation vector! (" << rotation[0] << "," << rotation[1] << "," << rotation[2]
        //           << ")" << std::endl;
    }

    const auto rotationAxis = glm::vec3(glm::inverse(cubelet.rotationMatrix) * glm::vec4(normalizedRotation, 1.0F));
    cubelet.rotationMatrix = glm::rotate(cubelet.rotationMatrix, rotationAngle, rotationAxis);
}

void updateRotation(std::array<Cubelet, CUBELET_COUNT> &cubelets,
                    std::array<unsigned int, CUBELET_COUNT> &cubeletPositions, RotationCommand command,
                    float angleDiff) {
    std::array<unsigned int, SMALL_FACE_COUNT> cubes;
    glm::vec3 rotationVector;
    auto direction = static_cast<float>(rubiks::getDirection(command));
    switch (command.side) {
    case Face::FRONT:
        cubes = FRONT_CUBES;
        rotationVector = glm::vec3(0, 0, direction * angleDiff);
        break;
    case Face::BACK:
        cubes = BACK_CUBES;
        rotationVector = glm::vec3(0, 0, direction * angleDiff);
        break;
    case Face::LEFT:
        cubes = LEFT_CUBES;
        rotationVector = glm::vec3(direction * angleDiff, 0, 0);
        break;
    case Face::RIGHT:
        cubes = RIGHT_CUBES;
        rotationVector = glm::vec3(direction * angleDiff, 0, 0);
        break;
    case Face::UP:
        cubes = UP_CUBES;
        rotationVector = glm::vec3(0, direction * angleDiff, 0);
        break;
    case Face::DOWN:
        cubes = DOWN_CUBES;
        rotationVector = glm::vec3(0, direction * angleDiff, 0);
        break;
    case Face::NONE:
        // TODO do error handling
        break;
    }

    for (const auto cube : cubes) {
        const auto cubeIndex = cubeletPositions[cube];
        rubiks::updateCubeletRotation(cubelets[cubeIndex], rotationVector);
    }
}

void AnimationRubiksCube::rotate(float rotationSpeed = 2.0F) {
    if (isAnimationPaused || currentCommandIndex < 0) {
        return;
    }

    rotationSpeed = std::min(rotationSpeed, 2.0F);
    rotationSpeed = std::max(rotationSpeed, 0.0F);

    bool isDoneRotating = false;
    constexpr auto piHalf = glm::pi<float>() * 0.5F;
    if (currentAngle + rotationSpeed >= piHalf) {
        isDoneRotating = true;
        rotationSpeed = piHalf - currentAngle;
    } else {
        currentAngle += rotationSpeed;
    }

    auto currentCommand = commands[currentCommandIndex];
    updateRotation(cubelets, cube.globalIndexToCubeletIndex, currentCommand, rotationSpeed);
    if (!isDoneRotating) {
        return;
    }

    cube.rotate(currentCommand);

    currentAngle = 0.0F;
    currentCommandIndex++;
    if (currentCommandIndex >= commands.size()) {
        currentCommandIndex = -1;
    }
}

void AnimationRubiksCube::shuffle() {
    // TODO implement this
}

void AnimationRubiksCube::solve() {
    // TODO implement this
    // auto result = cube.solve();
    // commands.insert(commands.end(), result.begin(), result.end());
}

} // namespace rubiks
