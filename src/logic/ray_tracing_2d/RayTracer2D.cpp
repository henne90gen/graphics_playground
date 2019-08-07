#include "RayTracer2D.h"

namespace RayTracer2D {

    std::vector<Ray>
    calculateRays(const std::vector<Polygon> &walls, const glm::vec2 &lightPosition, const float cutoff) {
        std::vector<Ray> rays = {};
        for (auto &wall : walls) {
            for (auto &vertex : wall.vertices) {
                const auto transformedVertex = wall.transformMatrix * glm::vec4(vertex.x, vertex.y, 0.0F, 1.0F);
                const glm::vec2 pos = glm::vec2(transformedVertex.x, transformedVertex.y);
                if (pos.x < -cutoff || pos.x > cutoff || pos.y < -cutoff || pos.y > cutoff) {
                    continue;
                }
                glm::vec2 direction = pos - lightPosition;

                Ray ray = {};
                ray.startingPoint = lightPosition;
                ray.direction = glm::normalize(direction);
                ray.intersections.push_back(pos);
                rays.push_back(ray);
            }
        }
        return rays;
    }
}
