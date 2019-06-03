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
    noise.SetNoiseType(noiseType);

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
    if (cubePosition.x >= static_cast<float>(width)) {
        cubePosition.x = 0.0F;
        cubePosition.y += 1.0F;
    }
    if (cubePosition.y >= static_cast<float>(height)) {
        cubePosition.y = 0.0F;
        cubePosition.z += 1.0F;
    }
    if (cubePosition.z >= static_cast<float>(depth)) {
        cubePosition = glm::vec3();
        isRunning = false;
    }

    const unsigned int cornerCount = 8;
    std::array<float, cornerCount> surfaceValues = {};

    for (unsigned long i = 0; i < surfaceValues.size(); i++) {
        surfaceValues[i] = getSurfaceValue(cubeCorners[i]);
    }

    unsigned int cubeIndex = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[0] < surfaceLevel) cubeIndex |= 1U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[1] < surfaceLevel) cubeIndex |= 2U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[2] < surfaceLevel) cubeIndex |= 4U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[3] < surfaceLevel) cubeIndex |= 8U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[4] < surfaceLevel) cubeIndex |= 16U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[5] < surfaceLevel) cubeIndex |= 32U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[6] < surfaceLevel) cubeIndex |= 64U;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,hicpp-braces-around-statements)
    if (surfaceValues[7] < surfaceLevel) cubeIndex |= 128U;

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
                glm::vec4(cubeCorners[a0], surfaceValues[a0]),
                glm::vec4(cubeCorners[b0], surfaceValues[b0]));
        const glm::vec3 v1 = interpolateVerts(
                glm::vec4(cubeCorners[a1], surfaceValues[a1]),
                glm::vec4(cubeCorners[b1], surfaceValues[b1]));
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
    const float offset = 1.0F;
    const float scaleFactor = 2.0F;
    result += offset;
    result /= scaleFactor;
    return result;
}

glm::vec3 MarchingCubes::interpolateVerts(glm::vec4 v1, glm::vec4 v2) {
    if (!interpolate) {
        const float scaleFactor = 2.0F;
        return (v1 + v2) / scaleFactor;
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
