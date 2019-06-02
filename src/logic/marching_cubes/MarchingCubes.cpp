#include "MarchingCubes.h"

MarchingCubes::MarchingCubes() : cubePosition(glm::vec3()), stepCount(0) {

}

void MarchingCubes::start() {
    isRunning = true;
}

void MarchingCubes::reset() {
    // reset all data in the scene
    isRunning = false;
    cubePosition = glm::vec3(0.0F);
}

void MarchingCubes::step() {
    if (!isRunning) {
        return;
    }
    if (stepCount++ % animationSpeed != 0) {
        return;
    }
    // move cube to new position
    cubePosition.x += 1.0F;
    if (cubePosition.x >= 10.0F) {
        cubePosition.x = 0.0F;
        cubePosition.y += 1.0F;
    }
    if (cubePosition.y >= 10.0F) {
        cubePosition.y = 0.0F;
        cubePosition.z += 1.0F;
    }

    // create vertices at new position
    //  - check which corners of the cube are inside the volume
    //  - get triangle indices from triangleMap
    //  - interpolate vertex positions (make it possible to switch this on and off)
}
