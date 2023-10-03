#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

#include "DtmDownloader.h"

TEST(DtmDownloaderTest, can_download_html_file_over_https) {
    GTEST_SKIP_("This test depends on data which is not committed");

    // GIVEN
    auto expected_is = std::ifstream("dtm_viewer_resources/dtm_viewer_test_cmp.html", std::ios::in | std::ios::binary);
    auto expected_ss = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);
    expected_ss << expected_is.rdbuf();
    const auto expected_s = expected_ss.str();

    const auto testFilePath = "dtm_viewer_resources/dtm_viewer_test.html";
    auto downloader = DtmDownloader();

    // WHEN
    downloader.download("https://henne90gen.github.io/graphics_playground/", testFilePath);

    // THEN
    auto actual_is = std::ifstream(testFilePath, std::ios::in | std::ios::binary);
    auto actual_ss = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);
    actual_ss << actual_is.rdbuf();
    const auto actual_s = actual_ss.str();

    ASSERT_EQ(expected_s.size(), actual_s.size());
    ASSERT_EQ(expected_s, actual_s);
}

TEST(DtmDownloaderTest, can_download_zip_file_over_https) {
    GTEST_SKIP_("This test depends on data which is not committed");

    // GIVEN
    auto expected_is = std::ifstream("dtm_viewer_resources/dtm_viewer_test_cmp.zip", std::ios::in | std::ios::binary);
    auto expected_ss = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);
    expected_ss << expected_is.rdbuf();
    const auto expected_s = expected_ss.str();
    ASSERT_EQ(20089339, expected_s.size());

    const auto testFilePath = "dtm_viewer_resources/dtm_viewer_test.zip";
    auto downloader = DtmDownloader();

    // WHEN
    downloader.download(
          "https://geocloud.landesvermessung.sachsen.de/index.php/s/388qlKhVVdMwbX9/download?path=%2F&files=dgm1_33390_5638_2_sn_xyz.zip",
          testFilePath);

    // THEN
    auto actual_is = std::ifstream(testFilePath, std::ios::in | std::ios::binary);
    auto actual_ss = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);
    actual_ss << actual_is.rdbuf();
    const auto actual_s = actual_ss.str();

    ASSERT_EQ(expected_s.size(), actual_s.size());
    ASSERT_EQ(expected_s, actual_s);
}
