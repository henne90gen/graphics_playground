#pragma once

#include <stdint.h>

class SimplexNoise {
public:

    SimplexNoise(int64_t seed);

    ~SimplexNoise();

    int init_perm(int16_t *p, int nelements);

    /**
     * 2D OpenSimplex (Simplectic) Noise.
     */
    double noise2(double x, double y);

    /**
     * 3D OpenSimplex (Simplectic) Noise
     */
    double noise3(double x, double y, double z);

    /**
     * 4D OpenSimplex (Simplectic) Noise.
     */
    double noise4(double x, double y, double z, double w);

private:
    int16_t *perm;
    int16_t *permGradIndex3D;

    double extrapolate2(int xsb, int ysb, double dx, double dy);

    double extrapolate3(int xsb, int ysb, int zsb, double dx, double dy, double dz);

    double extrapolate4(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);

    int allocate_perm(int nperm, int ngrad);
};

