#include "Fourier.h"

#include <glm/ext.hpp>

#define M_PI 3.14159265359

namespace fourier {

void internalFFT(float *fftBuffer, long fftFrameSize, long sign) {
    float wr, wi, arg, *p1, *p2, temp;
    float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
    long i, bitm, j, le, le2, k, logN;
    logN = (long)(log(fftFrameSize) / log(2.) + .5);

    for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
        for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
            if (i & bitm)
                j++;
            j <<= 1;
        }
        if (i < j) {
            p1 = fftBuffer + i;
            p2 = fftBuffer + j;
            temp = *p1;
            *(p1++) = *p2;
            *(p2++) = temp;
            temp = *p1;
            *p1 = *p2;
            *p2 = temp;
        }
    }

    for (k = 0, le = 2; k < logN; k++) {
        le <<= 1;
        le2 = le >> 1;
        ur = 1.0;
        ui = 0.0;
        arg = M_PI / (le2 >> 1);
        wr = cos(arg);
        wi = sign * sin(arg);
        for (j = 0; j < le2; j += 2) {
            p1r = fftBuffer + j;
            p1i = p1r + 1;
            p2r = p1r + le2;
            p2i = p2r + 1;
            for (i = j; i < 2 * fftFrameSize; i += le) {
                tr = *p2r * ur - *p2i * ui;
                ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr;
                *p2i = *p1i - ti;
                *p1r += tr;
                *p1i += ti;
                p1r += le;
                p1i += le;
                p2r += le;
                p2i += le;
            }
            tr = ur * wr - ui * wi;
            ui = ur * wi + ui * wr;
            ur = tr;
        }
    }
}

long roundUpToPowerOfTwo(long num) {
    long currentPower = 2;
    while (true) {
        if (num < currentPower) {
            return currentPower;
        }
        currentPower *= 2;
    }
}

std::vector<DataPoint> fft(const std::vector<float> &inputData, unsigned int sampleRate) {
    const auto fftFrameSize = roundUpToPowerOfTwo(inputData.size());
    float *fftBuffer = reinterpret_cast<float *>(std::malloc(2 * fftFrameSize * sizeof(float)));
    const auto sign = -1L;

    for (unsigned int i = 0; i < fftFrameSize; i++) {
        if (i < inputData.size()) {
            fftBuffer[i * 2] = inputData[i];
        } else {
            fftBuffer[i * 2] = 0.0F;
        }
        fftBuffer[i * 2 + 1] = 0.0F;
    }

    internalFFT(fftBuffer, fftFrameSize, sign);

    auto result = std::vector<DataPoint>();
    const auto transformLength = inputData.size();
    for (unsigned int bin = 0; bin < fftFrameSize; bin += 2) {
        const auto cosPart = fftBuffer[bin];
        const auto sinPart = fftBuffer[bin + 1];

        const auto frequency = (static_cast<double>(bin) * static_cast<double>(sampleRate)) / transformLength;
        //        const auto magnitude = (20.0 * log10(2.0 * std::sqrt(sinPart * sinPart + cosPart * cosPart))) /
        //        transformLength;
        const auto magnitude = std::sqrt(sinPart * sinPart + cosPart * cosPart);
        const auto phase = (180.0 * std::atan2(sinPart, cosPart)) / M_PI;
        result.push_back({frequency, magnitude, phase});
    }

    free(fftBuffer);

    return result;
}

std::vector<DataPoint> dft(const std::vector<float> &inputData, unsigned int sampleRate) {
    std::vector<DataPoint> result = {};
    double sign = -1.0;
    double transformLength = inputData.size();
    for (unsigned int bin = 0; bin <= inputData.size() / 2; bin++) {
        double sinPart = 0.0;
        double cosPart = 0.0;
        for (unsigned int k = 0; k < inputData.size(); k++) {
            double tmp = (2.0 * static_cast<double>(bin) * M_PI * static_cast<double>(k)) / transformLength;
            sinPart += inputData[k] * sign * std::sin(tmp);
            cosPart += inputData[k] * std::cos(tmp);
        }

        double frequency = (static_cast<double>(bin) * static_cast<double>(sampleRate)) / transformLength;
        double magnitude = (20.0 * log10(2.0 * std::sqrt(sinPart * sinPart + cosPart * cosPart))) / transformLength;
        double phase = (100.0 * std::atan2(sinPart, cosPart)) / M_PI - 90.0;
        result.push_back({frequency, magnitude, phase});
    }

    return result;
}

std::vector<DataPoint> dft2(const std::vector<glm::vec2> &inputData, unsigned int resolution) {
    const unsigned long N = inputData.size();
    auto result = std::vector<DataPoint>();

    int startFrequency = -1 * static_cast<int>(resolution);
    for (int frequency = startFrequency; frequency <= static_cast<int>(resolution); frequency++) {
        std::complex<double> sum = {0, 0};
        for (unsigned long n = 0; n < N; n++) {
            double angle = (glm::two_pi<double>() * static_cast<double>(frequency) * static_cast<double>(n)) /
                           static_cast<double>(N);
            sum += std::complex<double>(inputData[n].x, inputData[n].y) *
                   std::complex<double>(cos(angle), -1 * sin(angle));
        }

        double re = sum.real() / static_cast<double>(N);
        double im = sum.imag() / static_cast<double>(N);
        double magnitude = glm::length(glm::vec2(re, im));
        double phase = atan2(im, re);
        result.push_back({static_cast<double>(frequency), magnitude, phase});
    }

    return result;
}

} // namespace fourier
