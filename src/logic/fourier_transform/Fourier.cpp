#include <glm/ext.hpp>
#include "Fourier.h"

std::vector<glm::vec2> Fourier::calculate(const std::vector<glm::vec2> &points, unsigned int resolution) {
    auto coefficients = std::vector<glm::vec2>(2 * resolution + 1);
    coefficients[0] = average(points);
    for (unsigned int i = 1; i <= 2 * resolution; i += 2) {
        coefficients[i] = average(points, i);
        coefficients[i + 1] = average(points, -1 * (int) i);
    }
    return coefficients;
}

void Fourier::step() {

}

glm::vec2 Fourier::average(const std::vector<glm::vec2> &points, const int factor) {
    glm::vec2 result = {};
    glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), (float) factor, {0.0, 0.0, 1.0});
    for (auto point : points) {
        const glm::vec3 rotatedPoint = rotation * glm::vec4(point.x, point.y, 0.0, 0.0);
        result += glm::vec2(rotatedPoint.x, rotatedPoint.y);
    }
    result /= points.size();
    return result;
}
