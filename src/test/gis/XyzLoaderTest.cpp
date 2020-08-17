#include <catch.hpp>

#include "gis/XyzLoader.h"
#include "util/VectorUtils.h"

#include <iostream>

TEST_CASE("Can load xyz directory") {
    std::vector<glm::vec3> result = {};
    BoundingBox3 bb = {};
    bool success = loadXyzDir("../../../src/test/gis/dtm", bb, result);
    REQUIRE(success);
    REQUIRE(result.size() == 38220);
    PRINT_VEC3(bb.min);
    PRINT_VEC3(bb.max);
    REQUIRE(bb.min == glm::vec3(278240, 470.22, 5.58986e+06));
    REQUIRE(bb.max == glm::vec3(281980, 633.59, 5.60482e+06));
}
