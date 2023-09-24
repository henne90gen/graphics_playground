#include <gtest/gtest.h>

#include <zip.h>

TEST(zip, opens_test_zip) {
    auto zOpt = zip::open("../../src/libs/zip/test.zip");
    ASSERT_TRUE(zOpt.has_value());

    auto z = zOpt.value();
    ASSERT_EQ("../../src/libs/zip/test.zip", z.filepath);

    ASSERT_EQ(2, z.files.size());
    ASSERT_EQ("hello.txt", z.files[0]);
    ASSERT_EQ("world.txt", z.files[1]);
}
