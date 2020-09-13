#include "GraphVis.h"

#include "quad_tree/QuadTree.h"
#include "util/ImGuiUtils.h"

void GraphVis::setup() {
    shader = std::make_shared<Shader>("scenes/graph_vis/GraphVisVert.glsl", "scenes/graph_vis/GraphVisFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    initNodeMesh();
    initEdgeMesh();

    resetGraph(GraphType::SMALL_LATTICE);
}

void GraphVis::onAspectRatioChange() {
    float aspectRatio = getAspectRatio();
    projectionMatrix = glm::ortho(-1.0F * aspectRatio, 1.0F * aspectRatio, -1.0F, 1.0F);
}

void GraphVis::tick() {
    static auto previousMousePos = glm::vec2(0.0F);
    static auto drawWireframe = false;
    static auto zoom = 0.1F;
    static auto pan = glm::vec2(0.0F);
    static auto panSpeed = 10.0F;
    static auto edgeColor = glm::vec3(1.0F);
    static auto shouldRunSimulation = false;
    static auto params = GraphParameters();
    static auto chosenGraph = GraphType::SMALL_LATTICE;

    ImGui::Begin("Settings");
    ImGui::DragFloat("Zoom", &zoom, 0.001F, 0.001F, 10000.0F);
    ImGui::DragFloat2("Pan", reinterpret_cast<float *>(&pan), 0.01F);
    if (ImGui::Button("Reset Pan")) {
        pan = glm::vec2(0.0F);
    }
    ImGui::DragFloat("Pan Speed", &panSpeed, 0.1F);
    ImGui::ColorEdit3("Edge Color", reinterpret_cast<float *>(&edgeColor));
    ImGui::Separator();
    if (shouldRunSimulation) {
        if (ImGui::Button("Stop Simulation")) {
            shouldRunSimulation = false;
        }
    } else {
        if (ImGui::Button("Run Simulation")) {
            shouldRunSimulation = true;
        }
    }
    ImGui::Combo("", reinterpret_cast<int *>(&chosenGraph), "SMALL_LATTICE\0STAR\0LATTICE\0TREE\0\0");
    ImGui::SameLine();
    if (ImGui::Button("Reset Simulation")) {
        resetGraph(chosenGraph);
    }
    ImGui::DragFloat("Tightness k", &params.k, 0.001F);
    ImGui::DragFloat("Damping b", &params.b, 0.001F);
    ImGui::DragFloat("Distance d", &params.d, 0.01F);
    ImGui::DragFloat("Charge q", &params.q, 0.001F);
    ImGui::End();

    auto viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(pan.x, pan.y, 0.0F));
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, 1.0F));

    glm::vec2 mousePos = getMousePos(zoom, pan);
    auto mouseMove = mousePos - previousMousePos;
    double lastFrameTime = getLastFrameTime();

    doNodeDragging(mousePos);

    ImGui::Begin("Settings");
    ImGui::Text(" CurMouse: %.2f %.2f", mousePos.x, mousePos.y);
    ImGui::Text("PrevMouse: %.2f %.2f", previousMousePos.x, previousMousePos.y);
    ImGui::Text(" MovMouse: %.2f %.2f", mouseMove.x, mouseMove.y);
    for (unsigned int i = 0; i < nodes.size(); i++) {
        ImGui::Text("Node %d: %.2f %.2f", i, nodes[i].position.x, nodes[i].position.y);
    }
    ImGui::End();

    if (shouldRunSimulation) {
        runGraphSimulation(edges, nodes, params, lastFrameTime);
    }

    renderNodes(viewMatrix, drawWireframe);
    renderEdges(viewMatrix, edgeColor);

    previousMousePos = mousePos;
}
void GraphVis::doNodeDragging(const glm::vec2 &mousePos) {
    static int draggedNode = -1;
    if (!getInput()->mouse.left && draggedNode != -1) {
        draggedNode = -1;
    }
    if (getInput()->mouse.left && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemActive()) {
        if (draggedNode == -1) {
            for (unsigned int i = 0; i < nodes.size(); i++) {
                const auto &node = nodes[i];
                if (node.position.x - 0.5F < mousePos.x && node.position.x + 0.5F > mousePos.x &&
                    node.position.y - 0.5F < mousePos.y && node.position.y + 0.5F > mousePos.y) {
                    draggedNode = i;
                    break;
                }
            }
        }
        if (draggedNode != -1) {
            nodes[draggedNode].position = mousePos;
        }
    }
}

void GraphVis::renderNodes(const glm::mat4 &viewMatrix, const bool drawWireframe) const {
    auto positions = std::vector<std::pair<glm::vec2, glm::vec3>>(nodes.size());
    for (unsigned int i = 0; i < nodes.size(); i++) {
        positions[i] = std::make_pair(nodes[i].position, nodes[i].color);
    }

    shader->bind();
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("translationEnabled", true);
    shader->setUniform("useGlobalColor", false);

    nodeVa->bind();

    static unsigned int instanceVBO = 0;
    if (instanceVBO == 0) {
        GL_Call(glGenBuffers(1, &instanceVBO));
    }
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, instanceVBO));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW));

    GL_Call(glEnableVertexAttribArray(1));
    GL_Call(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(positions[0]), (void *)0));
    GL_Call(glVertexAttribDivisor(1, 1));

    GL_Call(glEnableVertexAttribArray(2));
    GL_Call(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(positions[0]), (void *)sizeof(glm::vec2)));
    GL_Call(glVertexAttribDivisor(2, 1));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElementsInstanced(GL_TRIANGLES, nodeVa->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
                                    positions.size()));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    nodeVa->unbind();
}

void GraphVis::renderEdges(const glm::mat4 &viewMatrix, const glm::vec3 &color) const {
    auto positions = std::vector<glm::vec2>(nodes.size());
    for (unsigned int i = 0; i < nodes.size(); i++) {
        positions[i] = nodes[i].position;
    }

    auto indices = std::vector<unsigned int>(edges.size() * 2);
    for (unsigned int i = 0; i < edges.size(); i++) {
        const GraphEdge &edge = edges[i];
        indices[i * 2] = edge.node1;
        indices[i * 2 + 1] = edge.node2;
    }

    shader->bind();
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("translationEnabled", false);
    shader->setUniform("useGlobalColor", true);
    shader->setUniform("uColor", color);

    edgesVa->bind();
    edgesVb->update(positions);
    edgesVa->getIndexBuffer()->update(indices);

    GL_Call(glDrawElements(GL_LINES, edgesVa->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    edgesVa->unbind();
    shader->unbind();
}

void GraphVis::initNodeMesh() {
    nodeVa = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {
          {-0.5F, -0.5F},
          {0.5F, -0.5F},
          {0.5F, 0.5F},
          {-0.5F, 0.5F},
    };
    BufferLayout layout = {
          {ShaderDataType::Float2, "position"},
    };
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    nodeVa->addVertexBuffer(vb);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2},
          {0, 2, 3},
    };
    auto ib = std::make_shared<IndexBuffer>(indices);
    nodeVa->setIndexBuffer(ib);
}

void GraphVis::initEdgeMesh() {
    edgesVa = std::make_shared<VertexArray>(shader);

    BufferLayout layout = {
          {ShaderDataType::Float2, "position"},
    };
    edgesVb = std::make_shared<VertexBuffer>(layout);
    edgesVa->addVertexBuffer(edgesVb);

    auto indexBuffer = std::make_shared<IndexBuffer>();
    edgesVa->setIndexBuffer(indexBuffer);
}

glm::vec2 GraphVis::getMousePos(float zoom, const glm::vec2 &pan) const {
    float aspectRatio = getAspectRatio();
    auto mousePos = getInput()->mouse.pos;
    auto mousePosScene = mousePos;
    mousePosScene.y = getHeight() - mousePosScene.y;
    mousePosScene /= glm::vec2(getWidth(), getHeight());
    mousePosScene *= 2.0F;
    mousePosScene -= 1.0F;
    mousePosScene.x *= aspectRatio;
    mousePosScene -= pan;
    mousePosScene /= zoom;
    return mousePosScene;
}

void createSmallLattice(std::vector<GraphNode> &nodes, std::vector<GraphEdge> &edges) {
    nodes.emplace_back(glm::vec2(-1.0F, -1.0F), glm::vec3(1.0F, 0.0F, 0.0F));
    nodes.emplace_back(glm::vec2(1.0F, -1.0F), glm::vec3(0.0F, 1.0F, 0.0F));
    nodes.emplace_back(glm::vec2(-1.0F, 1.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(1.0F, 1.0F), glm::vec3(1.0F, 0.0F, 1.0F));

    edges.push_back({0, 1});
    edges.push_back({0, 2});
    edges.push_back({0, 3});
    edges.push_back({1, 2});
    edges.push_back({1, 3});
    edges.push_back({2, 3});
}

void createStar(std::vector<GraphNode> &nodes, std::vector<GraphEdge> &edges) {
    nodes.emplace_back(glm::vec2(0.0F, 0.0F), glm::vec3(1.0F, 0.0F, 0.0F));
    nodes.emplace_back(glm::vec2(1.0F, 1.0F), glm::vec3(0.0F, 1.0F, 0.0F));
    nodes.emplace_back(glm::vec2(-1.0F, 1.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(-1.0F, -1.0F), glm::vec3(1.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(1.0F, -1.0F), glm::vec3(1.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(1.5F, 1.0F), glm::vec3(1.0F, 0.0F, 1.0F));

    edges.push_back({0, 1});
    edges.push_back({0, 2});
    edges.push_back({0, 3});
    edges.push_back({0, 4});
    edges.push_back({0, 5});
}

constexpr void addEdge(std::vector<GraphEdge> &edges, int x, int y, bool cond, int x_, int y_) {
    if (cond) {
        edges.push_back({static_cast<unsigned int>(y * 5 + x), static_cast<unsigned int>((y_)*5 + (x_))});
    }
}

void createLattice(std::vector<GraphNode> &nodes, std::vector<GraphEdge> &edges) {
    for (unsigned int y = 0; y < 5; y++) {
        for (unsigned int x = 0; x < 5; x++) {
            nodes.emplace_back(glm::vec2(x * 2.0F, y * 2.0F), glm::vec3(1.0F, 1.0F, 1.0F));
        }
    }

    for (unsigned int y = 0; y < 5; y++) {
        for (unsigned int x = 0; x < 5; x++) {
            addEdge(edges, x, y, x < 4, x + 1, y);
            addEdge(edges, x, y, y < 4, x, y + 1);
            addEdge(edges, x, y, y < 4 && x < 4, x + 1, y + 1);
            addEdge(edges, x, y, y < 4 && x > 0, x - 1, y + 1);
        }
    }
}

void createTree(std::vector<GraphNode> &nodes, std::vector<GraphEdge> &edges) {
    auto tree = QuadTree<unsigned int>(5);

    std::vector<std::pair<glm::vec3, unsigned int>> elements = {};
    const unsigned int width = 10;
    const unsigned int height = 10;
    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            elements.push_back(std::make_pair(glm::vec3(x, 0, z), x * height + z));
        }
    }
    tree.insert(elements);

    std::unordered_map<QuadTree<unsigned int>::Node *, unsigned int> nodeToIndexMap = {};
    tree.traversPostOrder([&nodeToIndexMap, &nodes, &edges](QuadTree<unsigned int>::Node *node) {
        unsigned int index = nodes.size();
        float x = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        float y = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        x *= 10.0F;
        y *= 10.0F;
        nodes.emplace_back(glm::vec2(x, y), glm::vec3(1.0F, 1.0F, 1.0F));
        nodeToIndexMap[node] = index;
        if (node->left != nullptr) {
            edges.push_back({index, nodeToIndexMap[node->left]});
        }
        if (node->right != nullptr) {
            edges.push_back({index, nodeToIndexMap[node->right]});
        }
    });
}

void GraphVis::resetGraph(const GraphType chosenGraph) {
    nodes.clear();
    edges.clear();

    switch (chosenGraph) {
    case GraphType::SMALL_LATTICE:
        createSmallLattice(nodes, edges);
        break;
    case GraphType::STAR:
        createStar(nodes, edges);
        break;
    case GraphType::LATTICE:
        createLattice(nodes, edges);
        break;
    case GraphType::TREE:
        createTree(nodes, edges);
        break;
    default:
        std::cout << "Could not create graph: invalid graph type" << std::endl;
        break;
    }
}
