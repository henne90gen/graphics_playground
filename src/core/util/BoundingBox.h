#pragma once

#include <glm/glm.hpp>
#include <algorithm>

#define UPDATE(left, op, right)                                                                                        \
    if (left op right) {                                                                                               \
        left = right;                                                                                                  \
    }

struct BoundingBox3 {
    glm::vec3 min = {
          std::numeric_limits<float>::max(), //
          std::numeric_limits<float>::max(), //
          std::numeric_limits<float>::max()  //
    };
    glm::vec3 max = {
          std::numeric_limits<float>::min(), //
          std::numeric_limits<float>::min(), //
          std::numeric_limits<float>::min()  //
    };

    void update(const glm::vec3 &point) {
        UPDATE(min.x, >, point.x)
        UPDATE(min.y, >, point.y)
        UPDATE(min.z, >, point.z)

        UPDATE(max.x, <, point.x)
        UPDATE(max.y, <, point.y)
        UPDATE(max.z, <, point.z)
    }

    void update(const BoundingBox3 &bb){
          UPDATE(min.x, >, bb.min.x) //
          UPDATE(min.y, >, bb.min.y) //
          UPDATE(min.z, >, bb.min.z) //
          UPDATE(max.x, <, bb.max.x) //
          UPDATE(max.y, <, bb.max.y) //
          UPDATE(max.z, <, bb.max.z) //
    }

    glm::vec3 closestPointOnSurface(const glm::vec3 &point) const {
        float x = std::min(max.x, std::max(point.x, min.x));
        float y = std::min(max.y, std::max(point.y, min.x));
        float z = std::min(max.z, std::max(point.z, min.x));
        return {x, y, z};
    }

    glm::vec3 center() const { return (min + max) / 2.0F; }
};
