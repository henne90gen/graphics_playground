#include <catch.hpp>
#include <glm/glm.hpp>

#include "fourier_transform/Fourier.h"

TEST_CASE("Fourier can calculate a simple circle") {
    std::vector<glm::vec2> points = {};
    points.emplace_back(1, 0);
    points.emplace_back(0, -1);
    points.emplace_back(-1, 0);
    points.emplace_back(0, 1);

    auto coefficients = fourier::dft2(points, 1);
    REQUIRE(coefficients.size() == 3);
}
