#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

struct MouseData {
    bool left = false;
    bool right = false;
    bool middle = false;
    glm::vec2 pos = {0.0F, 0.0F};
};

struct KeyboardData {
    std::unordered_map<char, bool> keys = {};

    /**
     * @param key is one of the GLFW_KEY_* constants
     * @return
     */
    [[nodiscard]] inline bool isKeyDown(char key) const {
        auto itr = keys.find(key);
        return itr != keys.end() && itr->second;
    }
};

struct InputData {
    MouseData mouse;
    KeyboardData keyboard;
};
