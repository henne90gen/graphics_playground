#include <gtest/gtest.h>

#include "XyzLoader.h"

#include <iostream>

TEST(XyzLoaderTest, Can_load_xyz_directory) {
    GTEST_SKIP_("This test depends on data which is not committed");

    std::vector<glm::vec3> result = {};
    BoundingBox3 bb = {};
    const auto success = loadXyzDir("../../../../src/test/gis/dtm", bb, result);
    ASSERT_TRUE(success);
    ASSERT_EQ(result.size(), 38220);
    ASSERT_EQ(bb.min, glm::vec3(278240, 470.22, 5.58986e+06));
    ASSERT_EQ(bb.max, glm::vec3(281980, 633.59, 5.60482e+06));
}

TEST(XyzLoaderTest, Can_count_lines_in_xyz_directory) {
    GTEST_SKIP_("This test depends on data which is not committed");

    const auto result = countLinesInDir("../../../../src/test/gis/dtm");
    ASSERT_EQ(result, 38220);
}
