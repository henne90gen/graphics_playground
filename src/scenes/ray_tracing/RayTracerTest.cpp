#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <tuple>

#include "RayTracer.h"
#include "util/TestUtils.h"

RayTracer::Ray ray(glm::vec3 startingPoint, glm::vec3 direction) { return {startingPoint, direction}; }

RayTracer::Object sphere(glm::vec3 position, float radius) {
    return {RayTracer::Object::Sphere, position, {}, false, {radius}};
}

TEST(RayTracerTest, can_calculate_intersection_of_sphere_and_ray) {
    const std::vector<std::tuple<RayTracer::Ray, RayTracer::Object, glm::vec3, glm::vec3, bool>> data = {
          std::make_tuple(ray({0, 0, 0}, {0, 0, 1}), sphere({0, 0, 2}, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1),
                          true),
          std::make_tuple(ray({0, 0, 0}, {0, 0, -1}), sphere({0, 0, 2}, 1), glm::vec3(), glm::vec3(), false),
    };

    for (const auto &t : data) {
        RayTracer::Ray ray = std::get<0>(t);
        RayTracer::Object object = std::get<1>(t);
        glm::vec3 expectedHitPoint = std::get<2>(t);
        glm::vec3 expectedHitNormal = std::get<3>(t);
        bool result = std::get<4>(t);
        glm::vec3 hitPoint = {0, 0, 0};
        glm::vec3 hitNormal = {};
        ASSERT_EQ(RayTracer::intersects(ray, object, hitPoint, hitNormal), result)
              << INFO_VEC3(ray.startingPoint) << INFO_VEC3(ray.direction) << INFO_VEC3(object.position)
              << INFO_BASIC(object.data.sphere.radius) << INFO_BASIC(result);
        ASSERT_EQ(hitPoint, expectedHitPoint) << INFO_VEC3(expectedHitPoint) << INFO_VEC3(hitPoint);
        ASSERT_EQ(hitNormal, expectedHitNormal) << INFO_VEC3(expectedHitNormal) << INFO_VEC3(hitNormal);
    }
}
