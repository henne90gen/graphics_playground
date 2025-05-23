#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <random>
#include <unordered_map>
#include <vector>

struct HeightMap {
    std::vector<glm::vec2> grid = {};
    std::unordered_map<int64_t, float> data = {};

    void set(int32_t x, int32_t z, float height);
    [[nodiscard]] float get(int32_t x, int32_t z) const;
};

struct Raindrop {
    glm::vec2 startingPosition = {};
    float velocity = 0.0F;
    float water = 0.0F;
    float sediment = 0.0F;
    std::vector<glm::vec3> path = {};
};

struct SimulationParams {
    float Kq = 10;          // constant parameter for soil carry capacity formula
    float Kw = 0.001f;      // water evaporation speed
    float Kr = 0.9f;        // erosion speed
    float Kd = 0.02f;       // deposition speed
    float Ki = 0.1F;        // direction inertia
    float minSlope = 0.05f; // minimum slope for soil carry capacity formula
    float g = 20;           // gravity
};

void simulateRaindrop(HeightMap &heightMap, std::mt19937 randomGenerator,
                      std::uniform_real_distribution<double> randomDistribution, const SimulationParams &params,
                      Raindrop &raindrop, size_t maxPathLength = 100);
