#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Polygon {
    glm::mat4 transformMatrix;
    std::vector<glm::vec2> vertices;
    std::vector<glm::ivec3> indices;
};

struct Ray {
    glm::vec2 startingPoint;
    glm::vec2 direction;
    std::vector<glm::vec2> intersections = {};
};

namespace RayTracer2D {

    std::vector<Ray> calculateRays(const std::vector<Polygon> &walls, const glm::vec2 &lightPosition, float cutoff);

}
