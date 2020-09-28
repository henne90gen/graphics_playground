#include "Dft.h"

#include <cmath>

void dft(const std::vector<float> &inputData, const float sampleRate, std::vector<float> &frequency,
         std::vector<float> &magnitude, std::vector<float> &phase) {
    float sign = -1.0F;
    float transformLength = inputData.size();
    for (unsigned int bin = 0; bin <= inputData.size() / 2; bin++) {
        float sinPart = 0.0F;
        float cosPart = 0.0F;
        for (unsigned int k = 0; k < inputData.size(); k++) {
            float tmp = (2.0F * static_cast<float>(bin) * M_PI * static_cast<float>(k)) / transformLength;
            sinPart += inputData[k] * sign * std::sin(tmp);
            cosPart += inputData[k] * std::cos(tmp);
        }

        frequency[bin] = (static_cast<float>(bin) * sampleRate) / transformLength;
        magnitude[bin] = (20.0F * log10(2.0F * std::sqrt(sinPart * sinPart + cosPart * cosPart))) / transformLength;
        phase[bin] = (100.0F * std::atan2(sinPart, cosPart)) / M_PI - 90.0F;
    }
}
