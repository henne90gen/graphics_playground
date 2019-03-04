#pragma once

#include "scenes/Scene.h"

#include <vector>

class MainMenu {
  public:
    MainMenu(GLFWwindow *window, std::vector<Scene *> &scenes, unsigned int *currentSceneIndex)
        : window(window), scenes(scenes), currentSceneIndex(currentSceneIndex) {
        // Go straight to a certain scene
        *currentSceneIndex = 2;
        active = false;
    };
    virtual ~MainMenu(){};

    void render();

    bool isActive() { return active; }
    void activate() { active = true; }

  private:
    GLFWwindow *window;
    bool active = true;
    std::vector<Scene *> &scenes;
    unsigned int *currentSceneIndex;
};
