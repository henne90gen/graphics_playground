#pragma once

#include <vector>
#include <glm/glm.hpp>

class Fourier {
public:
    /**
     * This function will calculate (2 * resolution + 1) coefficients
     * @param points
     * @param resolution
     * @return
     */
    static std::vector<glm::vec2> calculate(const std::vector<glm::vec2> &points, unsigned int resolution);

    static void step();

    static glm::vec2 average(const std::vector<glm::vec2> &points, int factor = 0);

};



