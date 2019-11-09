#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace RayTracer {

struct Sphere {
    float radius;
};

struct Plane {
    glm::vec3 normal;
};

struct Cuboid {
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
    glm::vec3 position = {0, 0, 0};
    glm::vec3 color = {0.1, 0.1, 0.1};
    float transparency = 0.0F;
    float reflection = 0.0F;

    union {
        struct Sphere sphere;
        struct Plane plane;
        struct Cuboid cuboid;
    } data;
};

struct Ray {
    glm::vec3 startingPoint = {0, 0, 0};
    glm::vec3 direction = {0, 0, 0};
};

struct Light {
    glm::vec3 position = {0, 0, 0};
    float brightness = 1.0F;
};

void rayTrace(const std::vector<Object> &objects, const Light &light, const glm::vec3 &cameraPosition, float zDistance,
              std::vector<glm::vec3> &pixels, unsigned int width, unsigned int height,
              std::vector<RayTracer::Ray> &rays, bool runAsync);

bool intersects(const Ray &ray, const Object &object, glm::vec3 &hitPoint, glm::vec3 &hitNormal);

bool intersects(const Ray &ray, const Object &object);

Object sphere(const glm::vec3 &position, const glm::vec3 &color, float radius, float transparency = 0.0F,
              float reflection = 0.0F);
Object plane(const glm::vec3 &position, const glm::vec3 &color, const glm::vec3 &normal);

} // namespace RayTracer
