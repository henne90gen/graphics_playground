#pragma once

#include <glm/glm.hpp>

struct MouseData {
    bool left = false;
    bool right = false;
    glm::vec2 pos = {0.0F, 0.0F};
};

struct KeyboardData {

};

struct InputData {
    MouseData mouse;
    KeyboardData keyboard;
};
