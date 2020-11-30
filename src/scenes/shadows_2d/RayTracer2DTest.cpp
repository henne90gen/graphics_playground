#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <glm/glm.hpp>
#include <tuple>

#include "RayTracer2D.h"
#include "util/TestUtils.h"

int main(int argc, char *argv[]) {
    return Catch::Session().run(argc, argv);
}

TEST_CASE("Can determine whether two lines are parallel") {
    auto data = GENERATE(table<glm::vec2, glm::vec2, bool>({
          std::make_tuple(glm::vec2(), glm::vec2(), true),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(1, 0), true),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(0, 1), false),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(5, 0), true),
          std::make_tuple(glm::vec2(1, 5), glm::vec2(5, 1), false),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(5, 0), true),
          std::make_tuple(glm::vec2(1, 5), glm::vec2(2, 10), true),
    }));
    auto dir1 = std::get<0>(data);
    auto dir2 = std::get<1>(data);
    auto result = std::get<2>(data);

    INFO_VEC2(dir1)
    INFO_VEC2(dir2)
    INFO_BASIC(result)
    REQUIRE(RayTracer2D::parallel(dir1, dir2) == result);
}

RayTracer2D::Ray ray(const glm::vec2 &startingPoint, const glm::vec2 &direction) {
    RayTracer2D::Ray result = {startingPoint, direction};
    return result;
}

TEST_CASE("Can determine whether two rays intersect") {
    auto data = GENERATE(table<RayTracer2D::Ray, RayTracer2D::Ray, glm::vec2, float, bool>({
          std::make_tuple(ray({0, 0}, {1, 0}), ray({0, 0}, {0, 1}), glm::vec2(0, 0), 0, true),
          std::make_tuple(ray({0, 0}, {2, 1}), ray({5, 0}, {-2, 4}), glm::vec2(4, 2), 2, true),
          std::make_tuple(ray({0, 0}, {1, 2}), ray({1, 1}, {-2, 0}), glm::vec2(0.5, 1), 0.5, true),
          std::make_tuple(ray({0, 0}, {-1.15, -1.05}), ray({-1, -1}, {0, 0.1}), glm::vec2(-1.0, -0.913043499),
                          0.869565248, true),
    }));
    RayTracer2D::Ray ray = std::get<0>(data);
    RayTracer2D::Ray line = std::get<1>(data);
    glm::vec2 expectedIntersection = std::get<2>(data);
    float expectedA = std::get<3>(data);
    bool result = std::get<4>(data);
    glm::vec2 intersection = {};
    float a = 0;

    INFO_VEC2(ray.startingPoint)
    INFO_VEC2(ray.direction)
    INFO_VEC2(line.startingPoint)
    INFO_VEC2(line.direction)
    INFO_VEC2(expectedIntersection)
    INFO_BASIC(expectedA)
    INFO_BASIC(result)
    REQUIRE(RayTracer2D::intersects(ray, line, intersection, a) == result);
    REQUIRE(intersection == expectedIntersection);
    REQUIRE(a == Approx(expectedA));
}
