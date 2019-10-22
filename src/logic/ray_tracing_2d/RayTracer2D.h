#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <limits>
#include <vector>

struct Polygon {
    glm::mat4 transformMatrix;
    std::vector<glm::vec2> vertices;
    std::vector<glm::ivec3> indices;
};

struct Ray {
    glm::vec2 startingPoint;
    glm::vec2 direction;
    glm::vec2 closestIntersection = {0, 0};
    float closestIntersectionFactor = std::numeric_limits<float>::max();
    std::vector<glm::vec2> intersections = {};
};

namespace RayTracer2D {

std::vector<Ray> calculateRays(const std::vector<Polygon> &walls, const Polygon &screenBorder,
                               const glm::vec2 &lightPosition);

bool parallel(const glm::vec2 &vec1, const glm::vec2 &vec2);

bool intersects(Ray &ray, Ray &line, glm::vec2 &intersection);

} // namespace RayTracer2D
