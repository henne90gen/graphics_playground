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
        const glm::vec2 d1 = glm::normalize(n1.position - n2.position) * (glm::length(n1.position - n2.position) - d);
        const glm::vec2 v1 = n1.velocity - n2.velocity;
        const glm::vec2 f1 = -(m / (t * t)) * k * d1 - (m / t) * b * v1;

        const glm::vec2 d2 = glm::normalize(n2.position - n1.position) * (glm::length(n1.position - n2.position) - d);
        const glm::vec2 v2 = n2.velocity - n1.velocity;
        const glm::vec2 f2 = -(m / (t * t)) * k * d2 - (m / t) * b * v2;

        if (std::isnan(f1.x) || std::isnan(f1.y) || std::isnan(f2.x) || std::isnan(f2.y)) {
            continue;
        }

        n1.acceleration += f1 / n1.mass;
        n2.acceleration += f2 / n2.mass;
    }
}

void updateChargeAcceleration(std::vector<GraphNode> &nodes, const float q) {
    for (auto &node : nodes) {
        glm::vec2 forces = glm::vec2(0.0F);
        for (unsigned int i = 0; i < nodes.size(); i++) {
            const auto &other = nodes[i];
            const float qSq = q * q;
            const glm::vec2 d = glm::normalize(node.position - other.position);
            const float dSq = d.x * d.x + d.y * d.y;
            const glm::vec2 f = d * (qSq / dSq);
            if (std::isnan(f.x) || std::isnan(f.y)) {
                continue;
            }
            forces += f;
        }

        node.acceleration += forces / node.mass;
    }
}
