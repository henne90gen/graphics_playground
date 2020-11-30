#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "graph_vis/Graph.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

class GraphVis : public Scene {
  public:
    enum class GraphType { SMALL_LATTICE, STAR, LATTICE, TREE };

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

    glm::vec2 getMousePos(float zoom, const glm::vec2 &pan) const;
    void doNodeDragging(const glm::vec2 &mousePos);
    void resetGraph(GraphType chosenGraph);
    void initEdgeMesh();
};
