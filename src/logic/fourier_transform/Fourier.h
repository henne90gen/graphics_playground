#pragma once

#include <complex>
#include <vector>

#include <glm/glm.hpp>

struct fourier_result {
    int frequency;
    double amplitude;
    double phase;
};

class Fourier {
public:

    static std::vector<fourier_result> dft(const std::vector<glm::vec2> &x, int resolution);
};



