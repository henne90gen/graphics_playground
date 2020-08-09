#include "MarchingCubes.h"

#include <iostream>
#include <omp.h>

void MarchingCubes::start() { isRunning = true; }

void MarchingCubes::reset() {
    // reset all data in the scene
    isRunning = false;
    cubePosition = glm::vec3(0.0F);
    vertices = std::vector<glm::vec3>();
    indices = std::vector<glm::ivec3>();
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
        surfaceValues[i] = getSurfaceValue(cubeCorners[i] + cubePosition);
        cubeIndex |= static_cast<int>(surfaceValues[i] < surfaceLevel) * (1 << i);
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

        unsigned int index = indices.size() * 3;
        indices.emplace_back(index, index + 1, index + 2);
    }
}

float MarchingCubes::getSurfaceValue(const glm::vec3 &vec) {
    float result = noise.GetNoise(vec.x, vec.y, vec.z);
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
    if (!interpolate) {
        isRunning = true;
        while (isRunning) {
            runOneStep();
        }
    } else {
        const float threshold = surfaceLevel * 2.0F - 1.0F;
        const glm::ivec3 dimensions = {width, height, depth};
        implicit_surface_func func = [this, &threshold](const glm::vec3 &translated) {
            float result = noise.GetNoise(translated.x, translated.y, translated.z);
            return result - threshold;
        };
        runMarchingCubes(dimensions, vertices, indices, func);
    }
}

inline glm::vec3 getVertexOnEdge(const glm::vec3 &v1, const float &w1, const glm::vec3 &v2, const float &w2) {
    float t = (-1.0F * w1) / (w2 - w1);
    return v1 + t * (v2 - v1);
}

#define MARCHING_CUBES_SEQUENTIAL 0
// SEQUENTIAL
// --------------------------------------------------------------
// Benchmark                    Time             CPU   Iterations
// --------------------------------------------------------------
// BM_MarchingCubes/10       4343 ns         4340 ns       161594
// BM_MarchingCubes/20      24740 ns        24727 ns        28270
// BM_MarchingCubes/30      80010 ns        79887 ns         8713
// BM_MarchingCubes/40     201163 ns       198718 ns         3468
// BM_MarchingCubes/50     451063 ns       449817 ns         1648
// BM_MarchingCubes/60     698768 ns       697763 ns         1061

// PARALLEL
// --------------------------------------------------------------
// Benchmark                    Time             CPU   Iterations
// --------------------------------------------------------------
// BM_MarchingCubes/10       5572 ns         5526 ns       126943
// BM_MarchingCubes/20      12187 ns        11801 ns        57220
// BM_MarchingCubes/30      28386 ns        27983 ns        24976
// BM_MarchingCubes/40      61031 ns        60253 ns        10844
// BM_MarchingCubes/50     119281 ns       118597 ns         5889
// BM_MarchingCubes/60     197227 ns       195985 ns         3424
#if MARCHING_CUBES_SEQUENTIAL
void runMarchingCubes(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices, std::vector<glm::ivec3> &indices,
                      implicit_surface_func &func) {

    vertices.clear();
    indices.clear();

    const unsigned int cubeCount = dimensions.x * dimensions.y * dimensions.z;
    const unsigned int maxTrianglesPerCube = 5;
    vertices.reserve(cubeCount * maxTrianglesPerCube * 3);
    indices.reserve(cubeCount * maxTrianglesPerCube);

    glm::vec3 cubePosition = {};
    for (cubePosition.z = 0; cubePosition.z < dimensions.z; cubePosition.z++) {
        for (cubePosition.y = 0; cubePosition.y < dimensions.y; cubePosition.y++) {
            for (cubePosition.x = 0; cubePosition.x < dimensions.x; cubePosition.x++) {

                float cubeCenterValue = func(cubePosition);
                if (cubeCenterValue < -2.0F || cubeCenterValue > 2.0F) {
                    continue;
                }

                unsigned int cubeIndex = 0;
                std::array<float, 8> surfaceValues = {};
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

                    const glm::vec3 v0 =
                          getVertexOnEdge(cubeCorners[a0], surfaceValues[a0], cubeCorners[b0], surfaceValues[b0]);
                    const glm::vec3 v1 =
                          getVertexOnEdge(cubeCorners[a1], surfaceValues[a1], cubeCorners[b1], surfaceValues[b1]);
                    const glm::vec3 v2 =
                          getVertexOnEdge(cubeCorners[a2], surfaceValues[a2], cubeCorners[b2], surfaceValues[b2]);

                    vertices.push_back(v0 + cubePosition);
                    vertices.push_back(v1 + cubePosition);
                    vertices.push_back(v2 + cubePosition);
                }
            }
        }
    }

    for (unsigned int i = 0; i < vertices.size() / 3; i++) {
        indices.emplace_back(i * 3, i * 3 + 1, i * 3 + 2);
    }
}
#else
void runMarchingCubes(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices, std::vector<glm::ivec3> &indices,
                      implicit_surface_func &func) {

    vertices.clear();
    indices.clear();

    const unsigned int cubeCount = dimensions.x * dimensions.y * dimensions.z;
    const unsigned int maxTrianglesPerCube = 5;
    vertices.reserve(cubeCount * maxTrianglesPerCube * 3);

#pragma omp parallel for
    for (unsigned int z = 0; z < dimensions.z; z++) {
        glm::vec3 cubePosition = {0.0F, 0.0F, z};

        auto tmpVertices = std::vector<glm::vec3>();
        tmpVertices.reserve(dimensions.x * dimensions.y * maxTrianglesPerCube * 3);

        for (cubePosition.y = 0; cubePosition.y < dimensions.y; cubePosition.y++) {
            for (cubePosition.x = 0; cubePosition.x < dimensions.x; cubePosition.x++) {

                float cubeCenterValue = func(cubePosition);
                if (cubeCenterValue < -2.0F || cubeCenterValue > 2.0F) {
                    continue;
                }

                unsigned int cubeIndex = 0;
                std::array<float, 8> surfaceValues = {};
                for (unsigned long i = 0; i < surfaceValues.size(); i++) {
                    auto translated = cubeCorners[i] + cubePosition;
                    float surfaceValue = func(translated);
                    cubeIndex |= static_cast<int>(surfaceValue < 0.0F) * (1 << i);
                    surfaceValues[i] = surfaceValue;
                }

                for (unsigned int i = 0; triangulation[cubeIndex][i] != -1; i += 3) {
                    // Get indices of corner points A and B for each of the three edges
                    // of the cube that need to be joined to form the triangle.
                    auto a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
                    auto b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

                    auto a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
                    auto b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

                    auto a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
                    auto b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

                    const glm::vec3 v0 =
                          getVertexOnEdge(cubeCorners[a0], surfaceValues[a0], cubeCorners[b0], surfaceValues[b0]);
                    const glm::vec3 v1 =
                          getVertexOnEdge(cubeCorners[a1], surfaceValues[a1], cubeCorners[b1], surfaceValues[b1]);
                    const glm::vec3 v2 =
                          getVertexOnEdge(cubeCorners[a2], surfaceValues[a2], cubeCorners[b2], surfaceValues[b2]);

                    tmpVertices.push_back(v0 + cubePosition);
                    tmpVertices.push_back(v1 + cubePosition);
                    tmpVertices.push_back(v2 + cubePosition);
                }
            }
        }

#pragma omp critical
        {
            for (auto &v : tmpVertices) {
                vertices.push_back(v);
            }
        }
    }

    for (unsigned int i = 0; i < vertices.size() / 3; i++) {
        indices.emplace_back(i * 3, i * 3 + 1, i * 3 + 2);
    }
}
#endif
