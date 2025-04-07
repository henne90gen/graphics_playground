#include "AnimationRubiksCube.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>

namespace rubiks {

void init(std::array<rubiks::Cubelet, CUBELET_COUNT> &cubelets) {
    for (unsigned int i = 0; i < CUBELET_COUNT; i++) {
        cubelets[i] = {glm::mat4(1.0F)};
    }
}

AnimationRubiksCube::AnimationRubiksCube() { init(cubelets); }

AnimationRubiksCube::AnimationRubiksCube(std::vector<RotationCommand> commands)
    : commands(commands), currentCommandIndex(0) {
    init(cubelets);
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

void updateRotation(std::array<Cubelet, CUBELET_COUNT> &cubelets,
                    std::array<unsigned int, CUBELET_COUNT> &cubeletPositions, RotationCommand command,
                    float angleDiff) {
    std::array<unsigned int, SMALL_FACE_COUNT> cubes;
    glm::vec3 rotationVector;
    auto direction = static_cast<float>(getDirection(command));
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
    if (isAnimationPaused || currentCommandIndex < 0 || currentCommandIndex >= (int)commands.size()) {
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
    if (currentCommand.side == Face::NONE) {
        // TODO do error handling
        return;
    }

    updateRotation(cubelets, cube.globalIndexToCubeletIndex, currentCommand, rotationSpeed);

    if (!isDoneRotating) {
        return;
    }

    cube.rotate(currentCommand);

    currentAngle = 0.0F;
    currentCommandIndex++;

    if (currentCommandIndex >= (int)commands.size()) {
        currentCommandIndex = 0;
        commands = {};
    }
}

void AnimationRubiksCube::addRotationCommand(RotationCommand command) { commands.push_back(command); }

void AnimationRubiksCube::shuffle() {
    constexpr auto rotationCommandCount = 12;
    constexpr std::array<RotationCommand, rotationCommandCount> rotations = {{
          R_R,
          R_RI,
          R_F,
          R_FI,
          R_D,
          R_DI,
          R_L,
          R_LI,
          R_U,
          R_UI,
          R_B,
          R_BI,
    }};

    const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, rotations.size() - 1);

    constexpr int shuffleCount = 20;
    for (unsigned int i = 0; i < shuffleCount; i++) {
        const unsigned int randomIndex = distribution(generator);
        RotationCommand rotation = rotations[randomIndex];
        commands.push_back(rotation);
    }
}

void AnimationRubiksCube::solve() {
    auto copy = cube.copy();
    for (size_t i = currentCommandIndex; i < commands.size(); i++) {
        copy.rotate(commands[i]);
    }
    const auto result = copy.solve();
    for (auto &command : result) {
        commands.push_back(command);
    }
}

} // namespace rubiks
