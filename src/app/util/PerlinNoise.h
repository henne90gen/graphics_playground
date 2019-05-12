#pragma once

#include <vector>

class PerlinNoise {
public:
    PerlinNoise();

    PerlinNoise(unsigned int seed);

    double noise(double x, double y, double z);

private:
    // The permutation vector
    std::vector<int> p;

    double fade(double t);

    double lerp(double t, double a, double b);

    double grad(int hash, double x, double y, double z);
};
