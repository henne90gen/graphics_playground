#include <catch.hpp>
#include <glm/glm.hpp>

#include "ray_tracing_2d/RayTracer2D.h"

TEST_CASE("Can determine whether two lines are parallel") {
    glm::vec2 dir1 = {};
    glm::vec2 dir2 = {};

    REQUIRE(RayTracer2D::parallel(dir1, dir2));

    dir1 = {1.0, 0.0};
    dir2 = {1.0, 0.0};
    REQUIRE(RayTracer2D::parallel(dir1, dir2));

    dir1 = {1.0, 0.0};
    dir2 = {0.0, 1.0};
    REQUIRE(!RayTracer2D::parallel(dir1, dir2));

    dir1 = {1.0, 0.0};
    dir2 = {5.0, 0.0};
    REQUIRE(RayTracer2D::parallel(dir1, dir2));

    dir1 = {1.0, 5.0};
    dir2 = {5.0, 1.0};
    REQUIRE(!RayTracer2D::parallel(dir1, dir2));

    dir1 = {1.0, 5.0};
    dir2 = {2.0, 10.0};
    REQUIRE(RayTracer2D::parallel(dir1, dir2));
}

TEST_CASE("Can determine whether two rays intersect") {
    Ray2D ray = {{0.0, 0.0},
               {1.0, 0.0}};
    Ray2D line = {{0.0, 0.0},
                {0.0, 1.0}};
    glm::vec2 intersection = {};
    REQUIRE(RayTracer2D::intersects(ray, line, intersection));
    REQUIRE(intersection == glm::vec2(0.0, 0.0));

    ray = {{0.0, 0.0},
           {2.0, 1.0}};
    line = {{5.0,  0.0},
            {-2.0, 4.0}};
    intersection = {};
    REQUIRE(RayTracer2D::intersects(ray, line, intersection));
    REQUIRE(intersection == glm::vec2(4.0, 2.0));

    ray = {{0.0, 0.0},
           {1.0, 2.0}};
    line = {{1.0,  1.0},
            {-2.0, 0.0}};
    intersection = {};
    REQUIRE(RayTracer2D::intersects(ray, line, intersection));
    REQUIRE(intersection == glm::vec2(0.5, 1.0));

    ray = {{0.0, 0.0},
           {-1.15, -1.05}};
    line = {{-1.0,  -1.0},
            {0.0, 0.1}};
    intersection = {};
    REQUIRE(RayTracer2D::intersects(ray, line, intersection));
    REQUIRE(intersection == glm::vec2(-1.0, -0.913043499));
}
