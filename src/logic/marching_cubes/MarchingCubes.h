#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <FastNoise.h>

#include "TriangulationTable.h"

const std::vector<glm::vec3> cubeCorners = {
        // back
        glm::vec3(0.0F, 0.0F, 0.0F), // 0
        glm::vec3(1.0F, 0.0F, 0.0F), // 1
        glm::vec3(1.0F, 1.0F, 0.0F), // 2
        glm::vec3(0.0F, 1.0F, 0.0F), // 3

        // front
        glm::vec3(0.0F, 0.0F, 1.0F), // 4
        glm::vec3(1.0F, 0.0F, 1.0F), // 5
        glm::vec3(1.0F, 1.0F, 1.0F), // 6
        glm::vec3(0.0F, 1.0F, 1.0F)  // 7
};

class MarchingCubes {
public:
    MarchingCubes() = default;

    void start();

    void reset();

    void step();

    glm::vec3 &getCubeTranslation() {
        return cubePosition;
    }

    std::vector<glm::vec3> &getVertices() {
        return vertices;
    }

    std::vector<unsigned int> &getIndices() {
        return indices;
    }

    bool animate = false;
    int animationSpeed = 15;
    unsigned int width = 20;
    unsigned int height = 20;
    unsigned int depth = 20;
    float surfaceLevel = 0.4;
    bool interpolate = true;
    float frequency = 0.08F;
    FastNoise::NoiseType noiseType = FastNoise::ValueFractal;
private:
    glm::vec3 cubePosition = glm::vec3();
    int stepCount = 0;
    bool isRunning = false;
    FastNoise noise = FastNoise();
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    float getSurfaceValue(const glm::vec3 &vec);

    glm::vec3 interpolateVerts(glm::vec4 v1, glm::vec4 v2) const;

    void runOneStep();

    void runComplete();
};
