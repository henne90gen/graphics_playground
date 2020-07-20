#include "RayTracer2D.h"

#include <functional>
#include <future>

#include "util/TimeUtils.h"

namespace RayTracer2D {

glm::vec2 getTransformedVertex(const glm::vec2 &vec, const glm::mat4 &mat) {
    const auto transformedVertex = mat * glm::vec4(vec.x, vec.y, 0.0F, 1.0F);
    return glm::vec2(transformedVertex.x, transformedVertex.y);
}

bool parallel(const glm::vec2 &vec1, const glm::vec2 &vec2) { return vec1.x * vec2.y == vec1.y * vec2.x; }

bool intersects(const Ray &ray, const Ray &line, glm::vec2 &intersection, float &a) {
    if (parallel(ray.direction, line.direction)) {
        return false;
    }

    auto &rayPoint = ray.startingPoint;
    auto &rayDir = ray.direction;
    auto &linePoint = line.startingPoint;
    auto &lineDir = line.direction;

    if (rayPoint == linePoint) {
        intersection = rayPoint;
        return true;
    }

    auto r_l = rayPoint - linePoint;
    if (lineDir.y != 0.0) {
        float ypr_xs_ys = (r_l.y * lineDir.x) / lineDir.y;
        float ypr_xs_ys_xpr = ypr_xs_ys - r_l.x;
        float xt_ytxs_ys = rayDir.x - ((rayDir.y * lineDir.x) / lineDir.y);
        a = ypr_xs_ys_xpr / xt_ytxs_ys;
        float b = (r_l.y + rayDir.y * a) / lineDir.y;
        intersection = rayPoint + rayDir * a;
        return a >= 0.0 && (b >= 0.0 && b <= 1.0);
    }
    if (lineDir.x != 0.0) {
        float xpr_ys_xs = (r_l.x * lineDir.y) / lineDir.x;
        float xpr_ys_xs_ypr = xpr_ys_xs - r_l.y;
        float yt_xtys_xs = rayDir.y - ((rayDir.x * lineDir.y) / lineDir.x);
        a = xpr_ys_xs_ypr / yt_xtys_xs;
        float b = (r_l.x + rayDir.x * a) / lineDir.x;
        intersection = rayPoint + rayDir * a;
        return a >= 0.0 && (b >= 0.0 && b <= 1.0);
    }
    return false;
}

void calculateRaysForPolygon(const Polygon &polygon, std::vector<Ray> &rays, std::vector<Ray> &lineSegments,
                             const glm::vec2 &lightPosition) {
    for (unsigned long i = 0; i < polygon.vertices.size(); i++) {
        const auto pos = getTransformedVertex(polygon.vertices[i], polygon.transformMatrix);

        if (i + 1 < polygon.vertices.size()) {
            const auto nextPos = getTransformedVertex(polygon.vertices[i + 1], polygon.transformMatrix);
            Ray lineSegment = {};
            lineSegment.startingPoint = pos;
            lineSegment.direction = nextPos - pos;
            lineSegments.push_back(lineSegment);
        } else if (i == polygon.vertices.size() - 1) {
            const auto nextPos = getTransformedVertex(polygon.vertices[0], polygon.transformMatrix);
            Ray lineSegment = {};
            lineSegment.startingPoint = pos;
            lineSegment.direction = nextPos - pos;
            lineSegments.push_back(lineSegment);
        }

        glm::vec2 direction = pos - lightPosition;

        const double deviation = 0.00001;
        double cs = cos(deviation);
        double sn = sin(deviation);
        double px = direction.x * cs - direction.y * sn;
        double py = direction.x * sn + direction.y * cs;
        Ray ray = {};
        ray.startingPoint = lightPosition;
        ray.direction = glm::normalize(glm::vec2(px, py));
        ray.intersections = {};
        rays.push_back(ray);

        cs = cos(-deviation);
        sn = sin(-deviation);
        px = direction.x * cs - direction.y * sn;
        py = direction.x * sn + direction.y * cs;
        ray = {};
        ray.startingPoint = lightPosition;
        ray.direction = glm::normalize(glm::vec2(px, py));
        ray.intersections = {};
        rays.push_back(ray);
    }
}

void findIntersections(const std::vector<Ray> &lineSegments, std::vector<Ray> &rays, unsigned long startIndex,
                       unsigned long endIndex) {
    for (unsigned long i = startIndex; i < endIndex; i++) {
        auto &ray = rays[i];
        for (auto &line : lineSegments) {
            glm::vec2 intersection = {};
            float a = 0.0;
            if (!intersects(ray, line, intersection, a)) {
                continue;
            }

            if (a < ray.closestIntersectionFactor) {
                ray.closestIntersectionFactor = a;
                ray.closestIntersection = intersection;
            }
            ray.intersections.push_back(intersection);
        }
    }
}

std::vector<Ray> calculateRays(const std::vector<Polygon> &walls, const Polygon &screenBorder,
                               const glm::vec2 &lightPosition, bool runAsync) {
    std::vector<Ray> rays = {};
    std::vector<Ray> lineSegments = {};
    for (auto &polygon : walls) {
        calculateRaysForPolygon(polygon, rays, lineSegments, lightPosition);
    }
    calculateRaysForPolygon(screenBorder, rays, lineSegments, lightPosition);

    if (runAsync) {
#pragma omp parallel for
        for (unsigned int i = 0; i < rays.size(); i++) {
            findIntersections(lineSegments, rays, i, i + 1);
        }
    } else {
        findIntersections(lineSegments, rays, 0, rays.size());
    }
    return rays;
}

} // namespace RayTracer2D
