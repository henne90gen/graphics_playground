#pragma once

#include <optional>
#include <string>
#include <vector>

namespace zip {

struct Container {
    std::string filepath;
    std::vector<std::string> files = {};
};

std::optional<Container> open(const std::string &filepath);

} // namespace zip
