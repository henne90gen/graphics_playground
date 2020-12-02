#include "MetaBalls.h"

#include <iostream>

implicit_surface_func exp_func(const std::vector<MetaBall> &metaballs) {
    return [&metaballs](const glm::vec3 &position) {
        float total = 0.0F;
        for (auto metaball : metaballs) {
            auto dir = position - metaball.position;
            dir *= dir;
            float distSq = dir.x + dir.y + dir.z;
            float exponent = (-1.0F * distSq) / metaball.radiusSq;
            float h = glm::exp(exponent);
            total += h;
        }
        float invE = 1 / glm::e<float>();
        return invE - total;
    };
}

// TODO(henne): this does not correctly use the radius of the balls
implicit_surface_func inverse_dist_func(const std::vector<MetaBall> &metaballs) {
    return [&metaballs](const glm::vec3 &position) {
        float total = 0.0F;
        for (const MetaBall &metaball : metaballs) {
            auto final = position - metaball.position;
            final *= final;
            float rSq = metaball.radiusSq * 2.0F;
            total += 1.0F / (final.x + final.y + final.z);
            total -= 1.0F / rSq;
        }
        return total;
    };
}

implicit_surface_func sphere_func(const std::vector<MetaBall> &metaballs) {
    return [&metaballs](const glm::vec3 &position) {
        float min = std::numeric_limits<float>::max();
        for (const auto &metaball : metaballs) {
            auto final = position - metaball.position;
            final *= final;
            float result = final.x + final.y + final.z - metaball.radiusSq;
            min = std::min(min, result);
        }
        return min / 10.0F;
    };
}
