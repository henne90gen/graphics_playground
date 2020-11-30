#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <glm/glm.hpp>
#include <tuple>

#include "RayTracer.h"
#include "util/TestUtils.h"

int main(int argc, char *argv[]) {
    return Catch::Session().run(argc, argv);
}

RayTracer::Ray ray(glm::vec3 startingPoint, glm::vec3 direction) { return {startingPoint, direction}; }

RayTracer::Object sphere(glm::vec3 position, float radius) {
    return {RayTracer::Object::Sphere, position, {}, false, {radius}};
}

TEST_CASE("Can calculate intersection of sphere and ray") {
    auto data = GENERATE(table<RayTracer::Ray, RayTracer::Object, glm::vec3, glm::vec3, bool>({
          std::make_tuple(ray({0, 0, 0}, {0, 0, 1}), sphere({0, 0, 2}, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1),
                          true),
          std::make_tuple(ray({0, 0, 0}, {0, 0, -1}), sphere({0, 0, 2}, 1), glm::vec3(), glm::vec3(), false),
    }));
    RayTracer::Ray ray = std::get<0>(data);
    RayTracer::Object object = std::get<1>(data);
    glm::vec3 expectedHitPoint = std::get<2>(data);
    glm::vec3 expectedHitNormal = std::get<3>(data);
    bool result = std::get<4>(data);
    glm::vec3 hitPoint = {0, 0, 0};
    glm::vec3 hitNormal = {};
    INFO_VEC3(ray.startingPoint)
    INFO_VEC3(ray.direction)
    INFO_VEC3(object.position)
    INFO_BASIC(object.data.sphere.radius)
    INFO_BASIC(result)
    REQUIRE(RayTracer::intersects(ray, object, hitPoint, hitNormal) == result);

    INFO_VEC3(expectedHitPoint)
    INFO_VEC3(hitPoint)
    REQUIRE(hitPoint == expectedHitPoint);

    INFO_VEC3(expectedHitNormal)
    INFO_VEC3(hitNormal)
    REQUIRE(hitNormal == expectedHitNormal);
}
