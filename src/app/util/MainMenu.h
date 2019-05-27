#pragma once

#include "scenes/Scene.h"

#include <vector>

class MainMenu {
  public:
    MainMenu(GLFWwindow *window, std::vector<Scene *> &scenes, unsigned int *currentSceneIndex)
        : window(window), scenes(scenes), currentSceneIndex(currentSceneIndex){};
    virtual ~MainMenu(){};

    void render();

    bool isActive() { return active; }
    void activate() { active = true; }

    void goToScene(unsigned int sceneIndex);
  private:

    bool active = true;
    GLFWwindow *window;
    std::vector<Scene *> &scenes;
    unsigned int *currentSceneIndex;
};