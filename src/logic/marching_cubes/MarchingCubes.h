#pragma once

#include <glm/ext.hpp>
#include "TriangulationTable.h"

class MarchingCubes {
public:
    MarchingCubes();

    void start();

    void reset();

    void step();

    glm::vec3 getCubeTranslation() {
        return cubePosition;
    }

    int animationSpeed = 60;
private:
    glm::vec3 cubePosition;
    int stepCount;
    bool isRunning;
};
