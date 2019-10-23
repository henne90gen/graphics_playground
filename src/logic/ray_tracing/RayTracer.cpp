#include "RayTracer.h"

namespace RayTracer {

Ray createRay(unsigned int row, unsigned int col, const glm::vec3 cameraPosition) {
    Ray result;
    result.startingPoint = cameraPosition;
    glm::vec3 direction = {col, row, 1000};
    result.direction = glm::normalize(direction);
    return result;
}

inline float calculateDistance(glm::vec3 point1, glm::vec3 point2) { return glm::abs(glm::length(point1 - point2)); }

bool intersects(const Object &object, const Ray &ray, glm::vec3 &hitPoint, glm::vec3 &hitNormal) { return true; }

bool intersects(const Object &object, const Ray &ray) {
    glm::vec3 point = {};
    glm::vec3 normal = {};
    return intersects(object, ray, point, normal);
}

void rayTrace(const std::vector<Object> &objects, const std::vector<Light> &lights, const glm::vec3 cameraPosition,
              std::vector<glm::vec3> &pixels, const unsigned int width, const unsigned int height) {
    const Light &light = lights[0];
    for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col < width; col++) {
            Ray ray = createRay(row, col, cameraPosition);
            glm::vec3 hitPoint;
            glm::vec3 hitNormal;
            float minDistance = INFINITY;
            Object object = {};
            for (auto &currentObject : objects) {
                if (intersects(currentObject, ray, hitPoint, hitNormal)) {
                    float distance = calculateDistance(cameraPosition, hitPoint);
                    if (distance < minDistance) {
                        object = currentObject;
                        minDistance = distance;
                    }
                }
            }
            bool isInShadow = false;
            if (object.type != Object::None) {
                // compute illumination
                Ray shadowRay;
                shadowRay.direction = light.position - hitPoint;
                for (auto &currentObject : objects) {
                    if (intersects(currentObject, shadowRay)) {
                        isInShadow = true;
                        break;
                    }
                }
            }
            pixels[row * width + col] = object.color * light.brightness * (float)isInShadow;
        }
    }
}

} // namespace RayTracer
