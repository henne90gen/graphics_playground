#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "fourier_transform/Fourier.h"

TEST(FourierTest, can_calculate_a_simple_circle) {
    // GIVEN
    std::vector<glm::vec2> points = {};
    points.emplace_back(1, 0);
    points.emplace_back(0, -1);
    points.emplace_back(-1, 0);
    points.emplace_back(0, 1);

    // WHEN
    auto coefficients = fourier::dft2(points, 1);

    // THEN
    ASSERT_EQ(3, coefficients.size());
}
