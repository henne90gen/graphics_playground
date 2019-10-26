#include "RayTracer.h"

#include <glm/gtx/intersect.hpp>
#include <glm/gtx/projection.hpp>
#include <iostream>

#define USE_GLM_INTERSECT 1

namespace RayTracer {

Ray createRay(const unsigned int row, const unsigned int col, const glm::vec3 &cameraPosition, const unsigned int width,
              const unsigned int height, const unsigned int zDistance) {
    Ray result;
    result.startingPoint = cameraPosition;
    int x = (int)col - (int)width / 2;
    int y = (int)row - (int)height / 2;
    glm::vec3 direction = {x, y, zDistance};
    result.direction = glm::normalize(direction);
    return result;
}

inline float calculateDistance(glm::vec3 point1, glm::vec3 point2) { return glm::abs(glm::length(point1 - point2)); }

bool intersectsSphere(const Ray &ray, const Object &object, glm::vec3 &hitPoint, glm::vec3 &hitNormal) {
    if (object.type != Object::Sphere) {
        return false;
    }
    auto sphere = object.data.sphere;
#if USE_GLM_INTERSECT
    return glm::intersectRaySphere(ray.startingPoint, ray.direction, object.position, sphere.radius, hitPoint,
                                   hitNormal);
#else
    /*
     * Sphere:   |X-M| = r
     * Ray:      s*R + Pr = X
     * Solution: |s*R + Pr - M| = r
     */
    glm::vec3 m_s = object.position - ray.startingPoint;
    glm::vec3 ray_dir = ray.direction;
    glm::vec3 projected = glm::proj(m_s, ray_dir);
    return false;
#endif
}

bool intersectsPlane(const Ray &ray, const Object &object, glm::vec3 &hitPoint, glm::vec3 &hitNormal) {
    if (object.type != Object::Plane) {
        return false;
    }
    auto plane = object.data.plane;
#if USE_GLM_INTERSECT
    float distance = 0.0F;
    bool result = glm::intersectRayPlane(ray.startingPoint, ray.direction, object.position, plane.normal, distance);
    if (result) {
        hitPoint = ray.startingPoint + ray.direction * distance;
        hitNormal = plane.normal;
    }
    return result;
#else
    // check that ray and plane are not parallel
    if (glm::dot(plane.normal, ray.direction) == 0) {
        // ray inside the plane is not counted as intersecting
        return false;
    }

    /*
     * Plane:    a*A + b*B + Pp = X
     * Ray:      s*R + Pr = X
     * Solution: a*A + b*B + Pp = s*R + Pr
     */
    return false;
#endif
}

bool intersectsCuboid(const Ray &ray, const Object &object, glm::vec3 &hitPoint, glm::vec3 &hitNormal) {
    if (object.type != Object::Cuboid) {
        return false;
    }
    return false;
}

bool intersects(const Ray &ray, const Object &object, glm::vec3 &hitPoint, glm::vec3 &hitNormal) {
    switch (object.type) {
    case Object::Sphere:
        return intersectsSphere(ray, object, hitPoint, hitNormal);
    case Object::Plane:
        return intersectsPlane(ray, object, hitPoint, hitNormal);
    case Object::Cuboid:
        return intersectsCuboid(ray, object, hitPoint, hitNormal);
    case Object::None:
    default:
        return false;
    }
    return false;
}

bool intersects(const Ray &ray, const Object &object) {
    glm::vec3 point = {};
    glm::vec3 normal = {};
    return intersects(ray, object, point, normal);
}

glm::vec3 trace(const Ray &ray, const Light &light, const glm::vec3 &cameraPosition, const std::vector<Object> &objects,
                unsigned int depth) {
    glm::vec3 hitPoint;
    glm::vec3 hitNormal;
    float minDistance = INFINITY;
    Object object = {};
    for (auto &currentObject : objects) {
        if (intersects(ray, currentObject, hitPoint, hitNormal)) {
            float distance = calculateDistance(cameraPosition, hitPoint);
            if (distance < minDistance) {
                object = currentObject;
                minDistance = distance;
            }
        }
    }
    bool isNotInShadow = true;
    if (object.type != Object::None) {
        // compute illumination
        Ray shadowRay = {};
        shadowRay.direction = glm::normalize(light.position - hitPoint);
        for (auto &currentObject : objects) {
            if (intersects(shadowRay, currentObject)) {
                isNotInShadow = false;
                break;
            }
        }
    }
    return object.color * light.brightness * (float)isNotInShadow;
}

void rayTrace(const std::vector<Object> &objects, const std::vector<Light> &lights, const glm::vec3 cameraPosition,
              std::vector<glm::vec3> &pixels, const unsigned int width, const unsigned int height,
              const unsigned int zDistance) {
    pixels.resize(width * height);
    if (lights.empty()) {
        std::cout << "Could not find any lights in the scene." << std::endl;
        return;
    }
    const Light &light = lights[0];

    for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col < width; col++) {
            Ray ray = createRay(row, col, cameraPosition, width, height, zDistance);
            pixels[row * width + col] = trace(ray, light, cameraPosition, objects, 0);
        }
    }
}
Object sphere(const glm::vec3 &position, const glm::vec3 &color, const float radius) {
    Object result = {};
    result.type = Object::Sphere;
    result.position = position;
    result.color = color;
    result.data = {radius};
    return result;
}

} // namespace RayTracer
