#include "RayTracer2D.h"

#include "util/TimeUtils.h"

namespace RayTracer2D {

    glm::vec2 getTransformedVertex(const glm::vec2 &vec, const glm::mat4 &mat) {
        const auto transformedVertex = mat * glm::vec4(vec.x, vec.y, 0.0F, 1.0F);
        return glm::vec2(transformedVertex.x, transformedVertex.y);
    }

    bool parallel(const glm::vec2 &vec1, const glm::vec2 &vec2) {
        return vec1.x * vec2.y == vec1.y * vec2.x;
    }

    bool intersects(Ray &ray, Ray &line, glm::vec2 &intersection) {
        if (parallel(ray.direction, line.direction)) {
            return false;
        }

        auto &P = ray.startingPoint;
        auto &T = ray.direction;
        auto &R = line.startingPoint;
        auto &S = line.direction;

        if (P == R) {
            intersection = P;
            return true;
        }

        double xpr = P.x - R.x;
        double ypr = P.y - R.y;
        double xpr_ys_xs = (xpr * S.y) / S.x;
        double xpr_ys_xs_ypr = xpr_ys_xs - (ypr);
        double yt_xtys_xs = T.y - ((T.x * S.y) / S.x);
        float a = xpr_ys_xs_ypr / yt_xtys_xs;
        float b = (xpr + T.x * a) / S.x;
        intersection = P + T * a;
        return a >= 0.0 && (b >= 0.0 && b <= 1.0);
    }

    std::vector<Ray>
    calculateRays(const std::vector<Polygon> &walls, const glm::vec2 &lightPosition, const float cutoff) {
        std::vector<Ray> rays = {};
        std::vector<Ray> lineSegments = {};
        for (auto &wall : walls) {
            for (unsigned long i = 0; i < wall.vertices.size(); i++) {
                const auto &vertex = wall.vertices[i];
                const auto pos = getTransformedVertex(vertex, wall.transformMatrix);

                if (i + 1 < wall.vertices.size()) {
                    const auto nextPos = getTransformedVertex(wall.vertices[i + 1], wall.transformMatrix);
                    Ray lineSegment = {};
                    lineSegment.startingPoint = pos;
                    lineSegment.direction = nextPos - pos;
                    lineSegments.push_back(lineSegment);
                } else if (i == wall.vertices.size() - 1) {
                    const auto nextPos = getTransformedVertex(wall.vertices[0], wall.transformMatrix);
                    Ray lineSegment = {};
                    lineSegment.startingPoint = pos;
                    lineSegment.direction = nextPos - pos;
                    lineSegments.push_back(lineSegment);
                }

                if (pos.x - lightPosition.x < -cutoff || pos.x - lightPosition.x > cutoff ||
                    pos.y - lightPosition.y < -cutoff || pos.y - lightPosition.y > cutoff) {
                    continue;
                }

                glm::vec2 direction = pos - lightPosition;

                Ray ray = {};
                ray.startingPoint = lightPosition;
                ray.direction = glm::normalize(direction);
                rays.push_back(ray);
            }
        }

        for (auto &ray : rays) {
            for (auto &line : lineSegments) {
                glm::vec2 intersection = {};
                if (!intersects(ray, line, intersection)) {
                    continue;
                }
                ray.intersections.push_back(intersection);
            }
        }
        return rays;
    }
}
