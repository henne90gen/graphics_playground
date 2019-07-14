#include "Fourier.h"

#include <algorithm>
#include <glm/ext.hpp>

std::vector<fourier_result> Fourier::dft(const std::vector<glm::vec2> &x, int resolution) {
    const unsigned long N = x.size();
    auto X = std::vector<fourier_result>();

    for (int k = -resolution; k <= resolution; k++) {
        std::complex<double> sum = {0, 0};
        for (unsigned long n = 0; n < N; n++) {
            double angle =
                    (glm::two_pi<double>() * static_cast<double>(k) * static_cast<double>(n)) / static_cast<double>(N);
            sum += std::complex<double>(x[n].x, x[n].y) * std::complex<double>(cos(angle), -1 * sin(angle));
        }
        double re = sum.real() / static_cast<double>(N);
        double im = sum.imag() / static_cast<double>(N);
        double amp = glm::length(glm::vec2(re, im));
        double phase = atan2(im, re);
        X.push_back({k, amp, phase});
    }

    std::sort(X.begin(), X.end(), [](fourier_result &a, fourier_result &b) {
        return a.amplitude > b.amplitude;
    });
    return X;
}
