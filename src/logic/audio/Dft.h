#pragma once

#include <vector>

void dft(const std::vector<float> &inputData, float sampleRate, std::vector<float> &frequency,
         std::vector<float> &magnitude, std::vector<float> &phase);
