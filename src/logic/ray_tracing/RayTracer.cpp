#include "RayTracer.h"

#include <future>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/projection.hpp>
#include <iostream>

#define USE_GLM_INTERSECT 1

namespace RayTracer {

Ray createRay(const unsigned int row, const unsigned int col, const glm::vec3 &cameraPosition, const float zDistance,
              const unsigned int width, const unsigned int height) {
    Ray result;
    result.startingPoint = cameraPosition;
    float x = (float)col / (float)width;
    float y = (float)row / (float)height;
    x -= 0.5F;
    y -= 0.5F;
    glm::vec3 direction = {x, y, zDistance};
    result.direction = glm::normalize(direction);
    return result;
}

inline float calculateDistance(const glm::vec3 &point1, const glm::vec3 &point2) {
    return glm::abs(glm::length(point2 - point1));
}

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
        Ray shadowRay = {};
        shadowRay.direction = glm::normalize(light.position - hitPoint);
        // moving starting point outside the object
        shadowRay.startingPoint = hitPoint + shadowRay.direction * 0.0001F;
        int index = -1;
        for (auto &currentObject : objects) {
            index++;
            if (index == 0) {
                // the first object is the light source, skipping it
                continue;
            }
            if (intersects(shadowRay, currentObject, hitPoint, hitNormal)) {
                float distance = glm::length(shadowRay.startingPoint - hitPoint);
                float lightDistance = glm::length(shadowRay.startingPoint - light.position);
                if (distance > lightDistance) {
                    isNotInShadow = false;
                    break;
                }
            }
        }
    }

    return object.color * light.brightness * (float)isNotInShadow;
}

void traceMultiple(const std::vector<Ray> &rays, const std::vector<Object> &objects, const Light &light,
                   const glm::vec3 cameraPosition, std::vector<glm::vec3> &pixels, unsigned int startIndex,
                   unsigned int endIndex) {
    for (unsigned int i = startIndex; i < endIndex; i++) {
        auto &ray = rays[i];
        pixels[i] = trace(ray, light, cameraPosition, objects, 0);
    }
}

void rayTrace(const std::vector<Object> &objects, const Light &light, const glm::vec3 &cameraPosition,
              const float zDistance, std::vector<glm::vec3> &pixels, const unsigned int width,
              const unsigned int height, bool runAsync) {
    pixels.resize(width * height);

    if (runAsync) {
        std::vector<Ray> rays = {};
        rays.resize(pixels.size());
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col++) {
                Ray ray = createRay(row, col, cameraPosition, zDistance, width, height);
                rays[row * width + col] = ray;
            }
        }

        std::vector<std::future<void>> results = {};
        // FIXME what do we do, if we have an odd number of rays?
        int numCores = 8;
        unsigned long numRaysPerCore = rays.size() / numCores;
        for (unsigned long i = 0; i < numCores; i++) {
            unsigned long startIndex = numRaysPerCore * i;
            unsigned long endIndex = numRaysPerCore * (i + 1);
            results.push_back(std::async(std::launch::async, traceMultiple, std::ref(rays), std::ref(objects),
                                         std::ref(light), cameraPosition, std::ref(pixels), startIndex, endIndex));
        }
        for (auto &result : results) {
            result.get();
        }
    } else {
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col++) {
                Ray ray = createRay(row, col, cameraPosition, zDistance, width, height);
                pixels[row * width + col] = trace(ray, light, cameraPosition, objects, 0);
            }
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

Object plane(const glm::vec3 &position, const glm::vec3 &color, const glm::vec3 &normal) {
    Object result = {};
    result.type = Object::Plane;
    result.position = position;
    result.color = color;
    result.data.plane = Plane();
    result.data.plane.normal = normal;
    return result;
}

} // namespace RayTracer
