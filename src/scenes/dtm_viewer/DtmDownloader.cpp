#include "DtmDownloader.h"

#if !EMSCRIPTEN

#include <curl/curl.h>
#include <fstream>
#include <iostream>

DtmDownloader::DtmDownloader() {
    auto version_info = curl_version_info(CURLVERSION_NOW);
    std::cout << "Using curl version " << version_info->version;
    std::cout << " with libz version " << version_info->libz_version;
    std::cout << std::endl;
    curl_global_init(CURL_GLOBAL_ALL);
}

DtmDownloader::~DtmDownloader() { curl_global_cleanup(); }

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    auto r = size * nmemb;
    auto write_data = (std::ofstream *)userdata;
    write_data->write(ptr, r);
    return r;
}

void DtmDownloader::download(const std::string &url, const std::string &destinationFilename) {
    auto write_data = std::ofstream(destinationFilename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!write_data.is_open()) {
        std::cerr << "Failed to open output file: " << destinationFilename << std::endl;
        return;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return;
    }

    CURLcode res;
    res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (res != CURLE_OK) {
        std::cerr << "Failed to set url parameter for download of " << url << ": " << curl_easy_strerror(res)
                  << std::endl;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    if (res != CURLE_OK) {
        std::cerr << "Failed to set write function for download of " << url << ": " << curl_easy_strerror(res)
                  << std::endl;
        return;
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);
    if (res != CURLE_OK) {
        std::cerr << "Failed to set write data for download of " << url << ": " << curl_easy_strerror(res) << std::endl;
        return;
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to perform download of " << url << ": " << curl_easy_strerror(res) << std::endl;
        return;
    }

    int64_t responseCode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (res != CURLE_OK) {
        std::cerr << "Failed to get response code for download of " << url << ": " << curl_easy_strerror(res)
                  << std::endl;
        return;
    }

    if (responseCode != 200) {
        std::cerr << "Response code was not 200 for download of " << url << ": " << curl_easy_strerror(res)
                  << std::endl;
        return;
    }

    curl_easy_cleanup(curl);
}

#else

// TODO implement downloader using Fetch-API (see https://emscripten.org/docs/api_reference/fetch.html)

DtmDownloader::DtmDownloader() {}

DtmDownloader::~DtmDownloader() {}

void DtmDownloader::download(const std::string &url, const std::string &destinationFilename) {}

#endif
