#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

struct GraphNode {
    glm::vec2 position = {};
    glm::vec2 velocity = {};
    glm::vec2 acceleration = {};
    float mass = 1.0F;
    glm::vec3 color = {};

    GraphNode(glm::vec2 position, glm::vec3 color) : position(position), color(color) {}
};

struct GraphEdge {
    unsigned int node1;
    unsigned int node2;
};

class GraphVis : public Scene {
  public:
    explicit GraphVis(SceneData &data) : Scene(data, "GraphVis"){};
    ~GraphVis() override = default;

    void setup() override;
    void tick() override;
    void destroy() override{};
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> nodeVa;
    std::shared_ptr<VertexArray> edgesVa;
    std::shared_ptr<VertexBuffer> edgesVb;
    glm::mat4 projectionMatrix;

    std::vector<GraphNode> nodes = {};
    std::vector<GraphEdge> edges = {};

    void initNodeMesh();
    void renderNodes(const glm::mat4 &viewMatrix, bool drawWireframe) const;
    void renderEdges(const glm::mat4 &viewMatrix, const glm::vec3 &color) const;

    void updateSpringAcceleration(GraphNode &n1, GraphNode &n2, const float t, const float k, const float b,
                                  const float d) const;
    void updateChargeAcceleration(GraphNode &node, const float q) const;
    void updateVelocityAndPosition(GraphNode &node, const float t) const;

    glm::vec2 getMousePos(float zoom, const glm::vec2 &pan) const;
    void resetGraph();
    void initEdgeMesh();
};
