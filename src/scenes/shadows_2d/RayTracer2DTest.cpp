#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <tuple>

#include "RayTracer2D.h"
#include "util/TestUtils.h"

TEST(RayTracer2DTest, can_determine_whether_two_lines_are_parallel) {
    const std::vector<std::tuple<glm::vec2, glm::vec2, bool>> data = {
          std::make_tuple(glm::vec2(), glm::vec2(), true),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(1, 0), true),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(0, 1), false),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(5, 0), true),
          std::make_tuple(glm::vec2(1, 5), glm::vec2(5, 1), false),
          std::make_tuple(glm::vec2(1, 0), glm::vec2(5, 0), true),
          std::make_tuple(glm::vec2(1, 5), glm::vec2(2, 10), true),
    };

    for (const auto &t : data) {
        const auto dir1 = std::get<0>(t);
        const auto dir2 = std::get<1>(t);
        const auto result = std::get<2>(t);

        ASSERT_EQ(RayTracer2D::parallel(dir1, dir2), result)
              << INFO_VEC2(dir1) << INFO_VEC2(dir2) << INFO_BASIC(result);
    }
}

RayTracer2D::Ray ray(const glm::vec2 &startingPoint, const glm::vec2 &direction) {
    RayTracer2D::Ray result = {startingPoint, direction};
    return result;
}

TEST(RayTracer2DTest, can_determine_whether_two_rays_intersect) {
    const std::vector<std::tuple<RayTracer2D::Ray, RayTracer2D::Ray, glm::vec2, float, bool>> data = {
          std::make_tuple(ray({0, 0}, {1, 0}), ray({0, 0}, {0, 1}), glm::vec2(0, 0), 0, true),
          std::make_tuple(ray({0, 0}, {2, 1}), ray({5, 0}, {-2, 4}), glm::vec2(4, 2), 2, true),
          std::make_tuple(ray({0, 0}, {1, 2}), ray({1, 1}, {-2, 0}), glm::vec2(0.5, 1), 0.5, true),
          std::make_tuple(ray({0, 0}, {-1.15, -1.05}), ray({-1, -1}, {0, 0.1}), glm::vec2(-1.0, -0.913043499),
                          0.869565248, true),
    };

    for (const auto &t : data) {
        const auto ray = std::get<0>(t);
        const auto line = std::get<1>(t);
        const auto expectedIntersection = std::get<2>(t);
        const auto expectedA = std::get<3>(t);
        const auto result = std::get<4>(t);
        glm::vec2 intersection = {};
        float a = 0;

        ASSERT_EQ(RayTracer2D::intersects(ray, line, intersection, a), result)
              << INFO_VEC2(ray.startingPoint) << INFO_VEC2(ray.direction) << INFO_VEC2(line.startingPoint)
              << INFO_VEC2(line.direction) << INFO_VEC2(expectedIntersection) << INFO_BASIC(expectedA)
              << INFO_BASIC(result);
        ASSERT_EQ(intersection, expectedIntersection);
        ASSERT_FLOAT_EQ(a, expectedA);
    }
}
