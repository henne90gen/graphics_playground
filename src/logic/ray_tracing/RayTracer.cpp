#include "RayTracer.h"

#include <future>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/projection.hpp>
#include <iostream>
#include <optional>

#include "util/VectorUtils.h"

#define USE_GLM_INTERSECT 1

const glm::vec3 BACKGROUND_COLOR = {0.1F, 0.1F, 0.1F};

namespace RayTracer {

glm::vec3 trace(const Ray &ray, const Light &light, const glm::vec3 &cameraPosition, const std::vector<Object> &objects,
                unsigned int depth, unsigned int maxRayDepth);

Ray createRay(const unsigned int row, const unsigned int col, const glm::vec3 &cameraPosition, const float zDistance,
              const unsigned int width, const unsigned int height) {
    Ray result;
    result.startingPoint = cameraPosition;
    float x = static_cast<float>(col) / static_cast<float>(width);
    x *= 2.0F;
    x -= 1.0F;

    float y = static_cast<float>(row) / static_cast<float>(height);
    y *= 2.0F;
    y -= 1.0F;

    glm::vec3 direction = {x, y, zDistance};
    result.direction = glm::normalize(direction);
    return result;
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
    glm::vec3 rayDirection = glm::normalize(ray.direction);
    bool result = glm::intersectRayPlane(ray.startingPoint, rayDirection, object.position, plane.normal, distance);
    if (distance < 0.0F) {
        std::cerr << "distance=" << distance << std::endl;
        std::cerr << "distance should not be negative!" << std::endl;
    }
    if (result) {
        hitPoint = ray.startingPoint + rayDirection * distance;
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

bool intersectsCuboid(const Ray & /*ray*/, const Object &object, glm::vec3 & /*hitPoint*/, glm::vec3 & /*hitNormal*/) {
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

bool isPositionInShadow(const std::vector<Object> &objects, const Object & /*object*/, const Light &light,
                        const glm::vec3 &position) {
    Ray shadowRay = {};
    shadowRay.direction = glm::normalize(light.position - position);
    // moving starting point outside the object
    shadowRay.startingPoint = position + shadowRay.direction * 0.0001F;

    glm::vec3 hitPoint;
    glm::vec3 hitNormal;
    int index = -1;
    for (auto &currentObject : objects) {
        index++;
        if (index == 0) {
            // the first object is the light source, skip it
            continue;
        }

        if (!intersects(shadowRay, currentObject, hitPoint, hitNormal)) {
            continue;
        }

        float distanceToHitPoint = glm::length(shadowRay.startingPoint - hitPoint);
        float distanceToLight = glm::length(shadowRay.startingPoint - light.position);
        if (distanceToHitPoint < distanceToLight) {
            return true;
        }
    }

    return false;
}

Ray createReflectionRay(const glm::vec3 &direction, const glm::vec3 &hitPoint, const glm::vec3 &hitNormal) {
    Ray result = {};
    result.startingPoint = hitPoint;
    result.direction = glm::reflect(direction, hitNormal);
    return result;
}

Ray createRefractionRay(const glm::vec3 &direction, const glm::vec3 &hitPoint, const glm::vec3 &hitNormal,
                        const float transparency) {
    Ray result = {};
    result.startingPoint = hitPoint;
    result.direction = glm::refract(direction, hitNormal, transparency);
    return result;
}

glm::vec3 traceGlass(const Ray &ray, const Light &light, const glm::vec3 &cameraPosition,
                     const std::vector<Object> &objects, const Object &object, const glm::vec3 &hitPoint,
                     const glm::vec3 &hitNormal, unsigned int depth, const unsigned int maxRayDepth) {
    Ray reflectionRay = createReflectionRay(ray.direction, hitPoint, hitNormal);
    // TODO(henne): create multiple rays that go out in a circle around the actual direction
    auto color = trace(reflectionRay, light, cameraPosition, objects, depth + 1, maxRayDepth);
    return color * object.reflection;
}

struct HitResult {
    Object object;
    float distance;
    glm::vec3 point;
    glm::vec3 normal;
};

std::optional<HitResult> hitCheck(const Ray &ray, const glm::vec3 &cameraPosition, const std::vector<Object> &objects) {
    HitResult result = {};
    glm::vec3 hitPoint;
    glm::vec3 hitNormal;
    float minDistance = -1.0F;
    for (auto &currentObject : objects) {
        if (!intersects(ray, currentObject, hitPoint, hitNormal)) {
            continue;
        }

        float distance = glm::length(cameraPosition - hitPoint);
        if (minDistance >= 0.0F && distance > minDistance) {
            continue;
        }

        minDistance = distance;
        result.object = currentObject;
        result.distance = distance;
        result.point = hitPoint;
        result.normal = hitNormal;
    }

    if (minDistance < 0) {
        return {};
    }

    return result;
}

bool isTransparentOrReflective(const Object &object) { return object.reflection > 0.0F; }

auto trace(const Ray &ray, const Light &light, const glm::vec3 &cameraPosition, const std::vector<Object> &objects,
           unsigned int depth, const unsigned int maxRayDepth) -> glm::vec3 {
    auto hitOpt = hitCheck(ray, cameraPosition, objects);
    if (!hitOpt.has_value()) {
        return BACKGROUND_COLOR;
    }

    auto hit = hitOpt.value();
    auto object = hit.object;
    glm::vec3 hitPoint = hit.point;
    glm::vec3 hitNormal = hit.normal;

    if (isTransparentOrReflective(object) && depth < maxRayDepth) {
        auto color = traceGlass(ray, light, cameraPosition, objects, object, hitPoint, hitNormal, depth, maxRayDepth);
        if (color.x == 0.1F || color.y == 0.1F || color.z == 0.1F) {
            // PRINT_VEC3(color);
        }
        return color;
    }

    bool isInShadow = isPositionInShadow(objects, object, light, hitPoint);
    return object.color * light.brightness * static_cast<float>(!isInShadow);
}

void traceMultiple(const std::vector<Ray> &rays, const std::vector<Object> &objects, const Light &light,
                   const glm::vec3 cameraPosition, std::vector<glm::vec3> &pixels, const unsigned int maxRayDepth,
                   unsigned int startIndex, unsigned int endIndex) {
    for (unsigned int i = startIndex; i < endIndex; i++) {
        auto &ray = rays[i];
        pixels[i] = trace(ray, light, cameraPosition, objects, 0, maxRayDepth);
    }
}

void rayTrace(const std::vector<Object> &objects, const Light &light, const glm::vec3 &cameraPosition,
              const float zDistance, std::vector<glm::vec3> &pixels, const unsigned int width,
              const unsigned int height, const unsigned int maxRayDepth, std::vector<Ray> &rays, bool runAsync) {
    pixels.resize(width * height);

    if (runAsync) {
        rays.resize(pixels.size());
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col++) {
                Ray ray = createRay(row, col, cameraPosition, zDistance, width, height);
                rays[row * width + col] = ray;
            }
        }

#pragma omp parallel for
        for (int i = 0; i < rays.size(); i++) {
            pixels[i] = trace(rays[i], light, cameraPosition, objects, 0, maxRayDepth);
        }

    } else {
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col++) {
                Ray ray = createRay(row, col, cameraPosition, zDistance, width, height);
                rays.push_back(ray);
                pixels[row * width + col] = trace(ray, light, cameraPosition, objects, 0, maxRayDepth);
            }
        }
    }
}

Object sphere(const glm::vec3 &position, const glm::vec3 &color, const float radius, const float reflection) {
    Object result = {};
    result.type = Object::Sphere;
    result.position = position;
    result.color = color;
    result.reflection = reflection;
    result.data = {radius};
    return result;
}

Object plane(const glm::vec3 &position, const glm::vec3 &color, const glm::vec3 &normal) {
    Object result = {};
    result.type = Object::Plane;
    result.position = position;
    result.color = color;
    result.data.plane = Plane();
    result.data.plane.normal = glm::normalize(normal);
    return result;
}

} // namespace RayTracer
