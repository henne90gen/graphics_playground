#pragma once

#include <glm/glm.hpp>
#include <vector>

struct GraphNode {
    glm::vec2 position = {};
    glm::vec2 velocity = {};
    glm::vec2 acceleration = {};
    float mass = 1.0F;
    glm::vec3 color = {};

    explicit GraphNode() {}
    GraphNode(glm::vec2 position, glm::vec3 color) : position(position), color(color) {}
};

struct GraphEdge {
    unsigned int node1;
    unsigned int node2;
};

struct GraphParameters {
    float k = 0.05F;
    float b = 0.05F;
    float d = 5.0F;
    float q = 5.0F;
};

void runGraphSimulation(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes,
                        const GraphParameters &params, const float lastFrameTime);

void updateSpringAcceleration(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes, const float t,
                              const float k, const float b, const float d);
void updateChargeAcceleration(std::vector<GraphNode> &nodes, const float q);
void updateVelocityAndPosition(GraphNode &node, const float t);
