#pragma once

#include "scenes/Scene.h"

#include <vector>

class MainMenu {
  public:
    MainMenu(GLFWwindow *window, std::vector<Scene *> &scenes, unsigned int *currentSceneIndex)
        : window(window), scenes(scenes), currentSceneIndex(currentSceneIndex) {
        // Go straight to a certain scene
        *currentSceneIndex = 3;
        active = false;
    };
    virtual ~MainMenu(){};

    void render();

    bool isActive() { return active; }
    void activate() { active = true; }

  private:
    void goToScene(unsigned int sceneIndex);

    bool active = true;
    GLFWwindow *window;
    std::vector<Scene *> &scenes;
    unsigned int *currentSceneIndex;
};
