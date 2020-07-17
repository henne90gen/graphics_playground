#include "MarchingCubes.h"

#include <iostream>

void MarchingCubes::start() { isRunning = true; }

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
    unsigned int cubeIndex = 0;
    std::array<float, cornerCount> surfaceValues = {};
    for (unsigned long i = 0; i < surfaceValues.size(); i++) {
        surfaceValues[i] = getSurfaceValue(cubeCorners[i]);
        cubeIndex |= (surfaceValues[i] < surfaceLevel) * (1 << i);
    }

    for (int i = 0; triangulation[cubeIndex][i] != -1; i += 3) {
        // Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
        int b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

        int a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
        int b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

        int a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
        int b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

        const glm::vec3 v0 = interpolateVerts(glm::vec4(cubeCorners[a0], surfaceValues[a0]),
                                              glm::vec4(cubeCorners[b0], surfaceValues[b0]));
        const glm::vec3 v1 = interpolateVerts(glm::vec4(cubeCorners[a1], surfaceValues[a1]),
                                              glm::vec4(cubeCorners[b1], surfaceValues[b1]));
        const glm::vec3 v2 = interpolateVerts(glm::vec4(cubeCorners[a2], surfaceValues[a2]),
                                              glm::vec4(cubeCorners[b2], surfaceValues[b2]));

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

glm::vec3 MarchingCubes::interpolateVerts(glm::vec4 v1, glm::vec4 v2) const {
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

glm::vec3 getVertexOnEdge(glm::vec4 v1, glm::vec4 v2) {
    float t = (-1.0F * v1.w) / (v2.w - v1.w);
    return glm::vec3(v1) + t * (glm::vec3(v2) - glm::vec3(v1));
}

bool updatePosition(glm::vec3 &cubePosition, const glm::vec3 &dimensions) {
    cubePosition.x += 1.0F;
    if (cubePosition.x >= static_cast<float>(dimensions.x)) {
        cubePosition.x = 0.0F;
        cubePosition.y += 1.0F;
    }
    if (cubePosition.y >= static_cast<float>(dimensions.y)) {
        cubePosition.y = 0.0F;
        cubePosition.z += 1.0F;
    }
    if (cubePosition.z >= static_cast<float>(dimensions.z)) {
        return true;
    }
    return false;
}

void runMarchingCubes(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices, std::vector<glm::ivec3> &indices,
                      implicit_surface_func &func) {
    glm::vec3 cubePosition = {0.0F, 0.0F, 0.0F};

    vertices.clear();
    indices.clear();

    const unsigned int cubeCount = dimensions.x * dimensions.y * dimensions.z;
    const unsigned int maxTrianglesPerCube = 5;
    vertices.reserve(cubeCount * maxTrianglesPerCube * 3);
    indices.reserve(cubeCount * maxTrianglesPerCube);

    while (true) {
        float cubeCenterValue = func(cubePosition);
        if (cubeCenterValue < -2.0F || cubeCenterValue > 2.0F) {
            if (updatePosition(cubePosition, dimensions)) {
                break;
            }
            continue;
        }

        const unsigned int cornerCount = 8;
        unsigned int cubeIndex = 0;
        std::array<float, cornerCount> surfaceValues = {};
        for (unsigned long i = 0; i < surfaceValues.size(); i++) {
            auto translated = cubeCorners[i] + cubePosition;
            float surfaceValue = func(translated);
            cubeIndex |= (surfaceValue < 0.0F) * (1 << i);
            surfaceValues[i] = surfaceValue;
        }

        for (unsigned int i = 0; triangulation[cubeIndex][i] != -1; i += 3) {
            // Get indices of corner points A and B for each of the three edges
            // of the cube that need to be joined to form the triangle.
            int a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
            int b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

            int a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
            int b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

            int a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
            int b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

            const glm::vec3 v0 = getVertexOnEdge(glm::vec4(cubeCorners[a0], surfaceValues[a0]),
                                                 glm::vec4(cubeCorners[b0], surfaceValues[b0]));
            const glm::vec3 v1 = getVertexOnEdge(glm::vec4(cubeCorners[a1], surfaceValues[a1]),
                                                 glm::vec4(cubeCorners[b1], surfaceValues[b1]));
            const glm::vec3 v2 = getVertexOnEdge(glm::vec4(cubeCorners[a2], surfaceValues[a2]),
                                                 glm::vec4(cubeCorners[b2], surfaceValues[b2]));

            vertices.push_back(v0 + cubePosition);
            vertices.push_back(v1 + cubePosition);
            vertices.push_back(v2 + cubePosition);

            const unsigned int nextIndex = indices.size() * 3;
            indices.emplace_back(nextIndex, nextIndex + 1, nextIndex + 2);
        }

        if (updatePosition(cubePosition, dimensions)) {
            break;
        }
    }
}
