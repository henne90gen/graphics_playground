#pragma once

#include "Scene.h"

int runScene(Scene *scene);

#define DEFINE_SCENE_MAIN(name)                                                                                        \
    int main() {                                                                                                       \
        auto scene = new name();                                                                                       \
        return runScene(scene);                                                                                               \
    }
