#include <gtest/gtest.h>

#include <zip.h>

TEST(zip, opens_test_zip) {
    auto zOpt = zip::open("../../src/libs/zip/test.zip");
    ASSERT_TRUE(zOpt.has_value());

    auto z = zOpt.value();
    ASSERT_EQ("../../src/libs/zip/test.zip", z.filepath);

    auto fileList = z.files();
    ASSERT_EQ(2, fileList.size());
    ASSERT_EQ("hello.txt", fileList[0]);
    ASSERT_EQ("world.txt", fileList[1]);
}
