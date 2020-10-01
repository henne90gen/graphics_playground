#pragma once

#include <complex>
#include <vector>

#include <glm/glm.hpp>

namespace fourier {

struct DataPoint {
    double frequency;
    double magnitude;
    double phase;
};

std::vector<DataPoint> fft(const std::vector<float> &inputData, unsigned int sampleRate);
std::vector<DataPoint> dft(const std::vector<float> &inputData, unsigned int resolution);
std::vector<DataPoint> dft2(const std::vector<glm::vec2> &inputData, unsigned int resolution);

} // namespace fourier
