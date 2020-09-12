#pragma once

#include <glm/glm.hpp>
#include <vector>

struct GraphNode {
    glm::vec2 position = {};
    glm::vec2 velocity = {};
    glm::vec2 acceleration = {};
    float mass = 1.0F;
    glm::vec3 color = {};

    explicit GraphNode() = default;
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
    float q = 10.0F;
};

void runGraphSimulation(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes,
                        const GraphParameters &params, float lastFrameTime);

void updateSpringAcceleration(const std::vector<GraphEdge> &edges, std::vector<GraphNode> &nodes, float t, float k,
                              float b, float d);
void updateChargeAcceleration(std::vector<GraphNode> &nodes, float q);
void updateVelocityAndPosition(GraphNode &node, float t);
