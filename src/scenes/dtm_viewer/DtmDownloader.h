#pragma once

#include <string>

struct DtmDownloader {
    DtmDownloader();
    ~DtmDownloader();

    void download(const std::string &url);
};
