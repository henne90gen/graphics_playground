#pragma once

#include <glm/glm.hpp>

struct MouseData {
    bool left;
    bool right;
    glm::vec2 pos;
};

struct KeyboardData {

};

struct InputData {
    MouseData mouse;
    KeyboardData keyboard;
};
