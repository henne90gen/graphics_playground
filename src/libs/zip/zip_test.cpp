#include <gtest/gtest.h>

#include <zip.h>

TEST(zip, opens_test_zip) {
    auto zOpt = zip::open_from_file("../../src/libs/zip/test.zip");
    ASSERT_TRUE(zOpt.has_value());

    auto z = zOpt.value();
    ASSERT_EQ(2, z.files.size());
    ASSERT_EQ("hello.txt", z.files[0].local_file_header.get_file_name());
    ASSERT_EQ("world.txt", z.files[1].local_file_header.get_file_name());

    auto contentOpt = z.files[0].get_content();
    ASSERT_TRUE(contentOpt.has_value());
    ASSERT_EQ("This is a test file", contentOpt.value());
}
