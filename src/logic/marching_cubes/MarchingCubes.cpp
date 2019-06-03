#include "MarchingCubes.h"

#include <iostream>

void MarchingCubes::start() {
    isRunning = true;
}

void MarchingCubes::reset() {
    // reset all data in the scene
    isRunning = false;
    cubePosition = glm::vec3(0.0F);
    vertices = std::vector<glm::vec3>();
    indices = std::vector<unsigned int>();
}

void MarchingCubes::step() {
    noise.SetFrequency(frequency);
    if (!animate) {
        runComplete();
    }
    if (!isRunning) {
        return;
    }
    if (stepCount++ % animationSpeed != 0) {
        return;
    }

    runOneStep();
}

void MarchingCubes::runOneStep() {
    cubePosition.x += 1.0F;
    if (cubePosition.x >= width) {
        cubePosition.x = 0.0F;
        cubePosition.y += 1.0F;
    }
    if (cubePosition.y >= height) {
        cubePosition.y = 0.0F;
        cubePosition.z += 1.0F;
    }
    if (cubePosition.z >= depth) {
        cubePosition = glm::vec3();
        isRunning = false;
    }

    int cubeIndex = 0;
    if (getSurfaceValue(cubeCorners[0]) < surfaceLevel) cubeIndex |= 1;
    if (getSurfaceValue(cubeCorners[1]) < surfaceLevel) cubeIndex |= 2;
    if (getSurfaceValue(cubeCorners[2]) < surfaceLevel) cubeIndex |= 4;
    if (getSurfaceValue(cubeCorners[3]) < surfaceLevel) cubeIndex |= 8;
    if (getSurfaceValue(cubeCorners[4]) < surfaceLevel) cubeIndex |= 16;
    if (getSurfaceValue(cubeCorners[5]) < surfaceLevel) cubeIndex |= 32;
    if (getSurfaceValue(cubeCorners[6]) < surfaceLevel) cubeIndex |= 64;
    if (getSurfaceValue(cubeCorners[7]) < surfaceLevel) cubeIndex |= 128;

    for (int i = 0; triangulation[cubeIndex][i] != -1; i += 3) {
        // Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
        int b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

        int a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
        int b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

        int a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
        int b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

        const glm::vec3 v0 = interpolateVerts(
                glm::vec4(cubeCorners[a0], getSurfaceValue(cubeCorners[a0])),
                glm::vec4(cubeCorners[b0], getSurfaceValue(cubeCorners[b0])));
        const glm::vec3 v1 = interpolateVerts(
                glm::vec4(cubeCorners[a1], getSurfaceValue(cubeCorners[a1])),
                glm::vec4(cubeCorners[b1], getSurfaceValue(cubeCorners[b1])));
        const glm::vec3 v2 = interpolateVerts(
                glm::vec4(cubeCorners[a2], getSurfaceValue(cubeCorners[a2])),
                glm::vec4(cubeCorners[b2], getSurfaceValue(cubeCorners[b2])));
        vertices.push_back(v0 + cubePosition);
        vertices.push_back(v1 + cubePosition);
        vertices.push_back(v2 + cubePosition);

        indices.push_back(indices.size());
        indices.push_back(indices.size());
        indices.push_back(indices.size());
    }
}

float MarchingCubes::getSurfaceValue(const glm::vec3 &vec) {
    auto translated = vec + cubePosition;
    float result = noise.GetNoise(translated.x, translated.y, translated.z);
    result += 1.0F;
    result /= 2.0F;
    return result;
}

glm::vec3 MarchingCubes::interpolateVerts(glm::vec4 v1, glm::vec4 v2) {
    if (!interpolate) {
        return (v1 + v2) / 2.0F;
    }

    float t = (surfaceLevel - v1.w) / (v2.w - v1.w);
    return glm::vec3(v1) + t * (glm::vec3(v2) - glm::vec3(v1));
}

void MarchingCubes::runComplete() {
    reset();
    isRunning = true;
    while (isRunning) {
        runOneStep();
    }
}
