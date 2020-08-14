#include <catch.hpp>

#include "gis/XyzLoader.h"

#include <iostream>

TEST_CASE("Can load xyz directory") {
    std::vector<glm::vec3> result = {};
    BoundingBox3 bb;
    bool success = loadXyzDir("../../../src/test/gis/dtm", result, bb);
    REQUIRE(success);
    REQUIRE(result.size() == 38220);
    REQUIRE(bb.min == glm::vec3());
    REQUIRE(bb.max == glm::vec3());
}
