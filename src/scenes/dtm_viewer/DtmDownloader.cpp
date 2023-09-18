#include "DtmDownloader.h"

#include <curl/curl.h>
#include <iostream>

DtmDownloader::DtmDownloader() {
    auto version_info = curl_version_info(CURLVERSION_NOW);
    std::cout << "Using curl version " << version_info->version;
    std::cout << " with libz version " << version_info->libz_version;
    std::cout << std::endl;
    curl_global_init(CURL_GLOBAL_ALL);
}

DtmDownloader::~DtmDownloader() { curl_global_cleanup(); }

void DtmDownloader::download() {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return;
    }

    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
