#include "MetaBalls.h"

implicit_surface_func exp_func(const std::vector<MetaBall> &metaballs) {
    return [&metaballs](const glm::vec3 &position) {
        float total = 0.0F;
        for (const MetaBall &metaball : metaballs) {
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

#define USE_METABALL_RADIUS 1

implicit_surface_func inverse_dist_func(const std::vector<MetaBall> &metaballs) {
    return [&metaballs](const glm::vec3 &position) {
        float total = 0.0F;
        for (const MetaBall &metaball : metaballs) {
            glm::vec3 final = position - metaball.position;
            final *= final;
            total += 1.0F / (final.x + final.y + final.z);
            float rSq = metaball.radiusSq * 2.0F;
            total -= 1.0F / rSq;
        }
        return total;
    };
}
