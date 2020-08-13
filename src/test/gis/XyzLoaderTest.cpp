#include <catch.hpp>

#include "gis/XyzLoader.h"

#include <iostream>

TEST_CASE("Can load xyz directory") {
    std::vector<glm::vec3> result = {};
    bool success = loadXyzDir("../../../src/test/gis/dtm", result);
    REQUIRE(success);
    REQUIRE(result.size() == 38220);
}
