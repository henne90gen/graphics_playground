#pragma once

#include <string>

#include "Image.h"

namespace ImageOps {
bool load(const std::string &fileName, Image &image);

bool save(Image &image);

void createCheckerBoard(Image &image);
} // namespace ImageOps
