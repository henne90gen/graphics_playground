#include <catch.hpp>
#include <glm/glm.hpp>

#include "fourier_transform/Fourier.h"

TEST_CASE("Fourier can calculate a simple circle") {
    std::vector<glm::vec2> points = {};
    points.emplace_back(1, 0);
    points.emplace_back(0, -1);
    points.emplace_back(-1, 0);
    points.emplace_back(0, 1);

    auto coefficients = Fourier::calculate(points, 1);
    REQUIRE(coefficients.size() == 3);
}

TEST_CASE("Average") {
    std::vector<glm::vec2> points = {};
    points.emplace_back(1, 0);
    points.emplace_back(0, -1);
    points.emplace_back(-1, 0);
    points.emplace_back(0, 1);

    auto average = Fourier::average(points);
    REQUIRE(average.x == 0);
    REQUIRE(average.y == 0);
}
