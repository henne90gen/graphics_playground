#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace RayTracer {

struct Sphere {
    glm::vec3 position;
    float radius;
};

struct Plane {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Cuboid {
    glm::vec3 position;
    glm::vec3 xDim;
    glm::vec3 yDim;
    glm::vec3 zDim;
};

struct Object {
    enum ObjectType {
        None,
        Sphere,
        Plane,
        Cuboid,
    };
    ObjectType type = None;
    glm::vec3 color;

    union {
        struct Sphere sphere;
        struct Plane plane;
        struct Cuboid cuboid;
    } data;
};

struct Ray {
    glm::vec3 startingPoint;
    glm::vec3 direction;
};

struct Light {
    glm::vec3 position;
    float brightness;
};

void rayTrace(const std::vector<Object> &objects, const std::vector<Light> &lights, const glm::vec3 cameraPosition,
              std::vector<glm::vec3> &pixels, const unsigned int width, const unsigned int height);

} // namespace RayTracer
