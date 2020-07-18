#pragma once

#include <vector>

#include <FastNoise.h>
#include <glm/ext.hpp>

#include "TriangulationTable.h"

class MarchingCubes {
  public:
    MarchingCubes() = default;

    void start();

    void reset();

    void step();

    glm::vec3 &getCubeTranslation() { return cubePosition; }

    std::vector<glm::vec3> &getVertices() { return vertices; }

    std::vector<unsigned int> &getIndices() { return indices; }

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

typedef std::function<float(const glm::vec3 &)> implicit_surface_func;

void runMarchingCubes(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices, std::vector<glm::ivec3> &indices,
                      implicit_surface_func &func);
