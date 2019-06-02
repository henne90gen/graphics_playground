#include "MarchingCubes.h"

MarchingCubes::MarchingCubes() : cubeTranslation(glm::vec3()) {

}

void MarchingCubes::start() {

}

void MarchingCubes::reset() {
    // reset all data in the scene
}

void MarchingCubes::step() {
    stepCount++;
    if (stepCount % animationSpeed != 0) {
        return;
    }
    // move cube to new position
    cubeTranslation += glm::vec3(2.0F, 0.0F, 0.0F);

    // create vertices at new position
    //  - check which corners of the cube are inside the volume
    //  - get triangle indices from triangleMap
    //  - interpolate vertex positions (make it possible to switch this on and off)
}
