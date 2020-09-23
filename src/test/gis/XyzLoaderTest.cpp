#include <catch.hpp>

#include "gis/XyzLoader.h"
#include "util/VectorUtils.h"

#include <iostream>

TEST_CASE("Can load xyz directory") {
    std::vector<glm::vec3> result = {};
    BoundingBox3 bb = {};
    bool success = loadXyzDir("../../src/test/gis/dtm", bb, result);
    REQUIRE(success);
    REQUIRE(result.size() == 38220);
    PRINT_VEC3(bb.min);
    PRINT_VEC3(bb.max);
    REQUIRE(bb.min == glm::vec3(278240, 470.22, 5.58986e+06));
    REQUIRE(bb.max == glm::vec3(281980, 633.59, 5.60482e+06));
}

TEST_CASE("Can count lines in xyz directory") {
    unsigned long result = countLinesInDir("../../src/test/gis/dtm");
    REQUIRE(result == 38220);
}

TEST_CASE("Can parse batch name from file name") {
    auto result = getBatchName("332785590_dgm20.xyz");
    REQUIRE(result == 2785590);

    result = getBatchName("331234567_dgm20.xyz");
    REQUIRE(result == 1234567);
}
