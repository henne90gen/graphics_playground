#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

struct MouseData {
    bool left = false;
    bool right = false;
    glm::vec2 pos = {0.0F, 0.0F};
};

struct KeyboardData {
    std::unordered_map<char, bool> keys = {};
};

struct InputData {
    MouseData mouse;
    KeyboardData keyboard;
};
