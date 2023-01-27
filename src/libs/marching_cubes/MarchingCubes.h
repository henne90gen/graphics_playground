#pragma once

#include <FastNoiseLite.h>
#include <functional>
#include <glm/ext.hpp>
#include <vector>

#include "TriangulationTable.h"

class MarchingCubes {
  public:
    MarchingCubes() = default;

    void start();

    void reset();

    void step();

    glm::vec3 &getCubeTranslation() { return cubePosition; }

    std::vector<glm::vec3> &getVertices() { return vertices; }

    std::vector<glm::ivec3> &getIndices() { return indices; }

    bool animate = false;
    int animationSpeed = 15;
    unsigned int width = 20;
    unsigned int height = 20;
    unsigned int depth = 20;
    float surfaceLevel = 0.4;
    bool interpolate = true;
    float frequency = 0.08F;
    FastNoiseLite::NoiseType noiseType = FastNoiseLite::NoiseType_Value;

  private:
    glm::vec3 cubePosition = glm::vec3();
    int stepCount = 0;
    bool isRunning = false;
    FastNoiseLite noise = FastNoiseLite();
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> indices;

    float getSurfaceValue(const glm::vec3 &vec);

    glm::vec3 interpolateVerts(glm::vec4 v1, glm::vec4 v2) const;

    void runOneStep();

    void runComplete();
};

typedef std::function<float(const glm::vec3 &)> implicit_surface_func;

void runMarchingCubes(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices, std::vector<glm::ivec3> &indices,
                      implicit_surface_func &func);

void runMarchingCubesSequential(const glm::ivec3 &dimensions, std::vector<glm::vec3> &vertices,
                                std::vector<glm::ivec3> &indices, implicit_surface_func &func);
