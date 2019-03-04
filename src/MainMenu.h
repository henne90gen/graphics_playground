#pragma once

#include "Scene.h"

#include <vector>

class MainMenu {
  public:
    MainMenu(std::vector<Scene *> &scenes, unsigned int *currentSceneIndex)
        : scenes(scenes), currentSceneIndex(currentSceneIndex){};
    virtual ~MainMenu(){};

    void tick();

    bool isActive() { return active; }
    void activate() { active = true; }

  private:
    bool active = true;
    std::vector<Scene *> &scenes;
    unsigned int *currentSceneIndex;
};
