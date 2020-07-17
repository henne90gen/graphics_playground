#pragma once

#include <vector>

#include "../marching_cubes/MarchingCubes.h"

struct MetaBall {
    glm::vec3 position;
    float radiusSq;
};

implicit_surface_func exp_func(const std::vector<MetaBall> &metaballs);

implicit_surface_func inverse_dist_func(const std::vector<MetaBall> &metaballs);
