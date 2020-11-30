#include "Graph.h"

void runGraphSimulation(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes,
                        const GraphParameters &params, const float lastFrameTime) {
    for (auto &node : nodes) {
        node.acceleration = glm::vec2(0.0F);
    }

    updateSpringAcceleration(edges, nodes, lastFrameTime, params.k, params.b, params.d);

    updateChargeAcceleration(nodes, params.q);

    for (auto &node : nodes) {
        node.velocity += node.acceleration * lastFrameTime;
        node.position += node.velocity * lastFrameTime;
    }
}

void updateSpringAcceleration(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes, const float t,
                              const float k, const float b, const float d) {
    for (const auto &edge : edges) {
        auto &n1 = nodes[edge.node1];
        auto &n2 = nodes[edge.node2];

        const float m = (n1.mass * n2.mass) / (n1.mass + n2.mass);
        const float dist = glm::length(n1.position - n2.position);
        const glm::vec2 forceDir = (n1.position - n2.position) / dist;
        const float preferredDist = dist - d;
        const glm::vec2 vel = n1.velocity - n2.velocity;
        const float distanceCoefficient = -(m / (t * t));
        const float velocityCoefficient = m / t;

        const glm::vec2 d1 = forceDir * preferredDist;
        const glm::vec2 v1 = vel;
        const glm::vec2 f1 = distanceCoefficient * k * d1 - velocityCoefficient * b * v1;

        const glm::vec2 d2 = -forceDir * preferredDist;
        const glm::vec2 v2 = -vel;
        const glm::vec2 f2 = distanceCoefficient * k * d2 - velocityCoefficient * b * v2;

        if (std::isnan(f1.x) || std::isnan(f1.y) || std::isnan(f2.x) || std::isnan(f2.y)) {
            continue;
        }

        n1.acceleration += f1 / n1.mass;
        n2.acceleration += f2 / n2.mass;
    }
}

void updateChargeAcceleration(std::vector<GraphNode> &nodes, const float q) {
    for (auto &node : nodes) {
        auto forces = glm::vec2(0.0F);
        for (const auto &other : nodes) {
            const float qSq = q * q;
            const glm::vec2 dir = node.position - other.position;
            const float distSq = dir.x * dir.x + dir.y * dir.y;
            const glm::vec2 f = (dir / glm::sqrt(distSq)) * (qSq / distSq);
            if (std::isnan(f.x) || std::isnan(f.y)) {
                continue;
            }
            forces += f;
        }

        node.acceleration += forces / node.mass;
    }
}
