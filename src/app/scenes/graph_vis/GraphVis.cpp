#include "GraphVis.h"

void GraphVis::setup() {
    shader = std::make_shared<Shader>("scenes/graph_vis/GraphVisVert.glsl", "scenes/graph_vis/GraphVisFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    initNodeMesh();
    initEdgeMesh();

    resetGraph();
}

void GraphVis::onAspectRatioChange() {
    float aspectRatio = getAspectRatio();
    projectionMatrix = glm::ortho(-1.0F * aspectRatio, 1.0F * aspectRatio, -1.0F, 1.0F);
}

void GraphVis::tick() {
    static auto previousMousePos = glm::vec2(0.0F);
    static auto drawWireframe = false;
    static auto zoom = 0.2F;
    static auto pan = glm::vec2(0.0F);
    static auto panSpeed = 10.0F;
    static auto edgeColor = glm::vec3(1.0F);
    static auto shouldRunSimulation = false;
    static auto k = 0.05F;
    static auto b = 0.05F;
    static auto d = 5.0F;
    static auto q = 0.0F;

    ImGui::Begin("Settings");
    ImGui::DragFloat("Zoom", &zoom, 0.001F, 0.001F, 10000.0F);
    ImGui::DragFloat2("Pan", reinterpret_cast<float *>(&pan), 0.01F);
    if (ImGui::Button("Reset Pan")) {
        pan = glm::vec2(0.0F);
    }
    ImGui::DragFloat("Pan Speed", &panSpeed, 0.1F);
    ImGui::ColorEdit3("Edge Color", reinterpret_cast<float *>(&edgeColor));
    ImGui::Separator();
    ImGui::Checkbox("Run Simulation", &shouldRunSimulation);
    if (ImGui::Button("Reset Simulation")) {
        resetGraph();
    }
    ImGui::DragFloat("Tightness k", &k, 0.001F);
    ImGui::DragFloat("Damping b", &b, 0.001F);
    ImGui::DragFloat("Distance d", &d, 0.01F);
    ImGui::DragFloat("Charge q", &q, 0.001F);
    ImGui::End();

    auto viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(pan.x, pan.y, 0.0F));
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, 1.0F));

    glm::vec2 mousePos = getMousePos(zoom, pan);
    auto mouseMove = mousePos - previousMousePos;
    double lastFrameTime = getLastFrameTime();
    if (getInput()->mouse.left && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemActive()) {
        bool hasMovedNode = false;
        for (unsigned int i = 0; i < nodes.size(); i++) {
            auto &node = nodes[i];
            if (node.position.x - 0.5F < mousePos.x && node.position.x + 0.5F > mousePos.x &&
                node.position.y - 0.5F < mousePos.y && node.position.y + 0.5F > mousePos.y) {
                node.position = mousePos;
                hasMovedNode = true;
            }
        }
        if (!hasMovedNode) {
            pan += mouseMove * panSpeed * static_cast<float>(lastFrameTime);
        }
    }

    ImGui::Begin("Settings");
    ImGui::Text(" CurMouse: %.2f %.2f", mousePos.x, mousePos.y);
    ImGui::Text("PrevMouse: %.2f %.2f", previousMousePos.x, previousMousePos.y);
    ImGui::Text(" MovMouse: %.2f %.2f", mouseMove.x, mouseMove.y);
    for (unsigned int i = 0; i < nodes.size(); i++) {
        ImGui::Text("Node %d: %.2f %.2f", i, nodes[i].position.x, nodes[i].position.y);
    }
    ImGui::End();

    if (shouldRunSimulation) {
        for (auto &node : nodes) {
            node.acceleration = glm::vec2(0.0F);
        }

        for (const auto &edge : edges) {
            auto &n1 = nodes[edge.node1];
            auto &n2 = nodes[edge.node2];
            updateSpringAcceleration(n1, n2, lastFrameTime, k, b, d);
        }

        for (auto &node : nodes) {
            updateChargeAcceleration(node, q);
        }

        for (auto &node : nodes) {
            updateVelocityAndPosition(node, lastFrameTime);
        }
    }

    renderNodes(viewMatrix, drawWireframe);
    renderEdges(viewMatrix, edgeColor);

    previousMousePos = mousePos;
}

void GraphVis::updateSpringAcceleration(GraphNode &n1, GraphNode &n2, const float t, const float k, const float b,
                                        const float d) const {
    const float m = (n1.mass * n2.mass) / (n1.mass + n2.mass);
    const glm::vec2 d1 = glm::normalize(n1.position - n2.position) * (glm::length(n1.position - n2.position) - d);
    const glm::vec2 v1 = n1.velocity - n2.velocity;
    const glm::vec2 f1 = -(m / (t * t)) * k * d1 - (m / t) * b * v1;

    const glm::vec2 d2 = glm::normalize(n2.position - n1.position) * (glm::length(n1.position - n2.position) - d);
    const glm::vec2 v2 = n2.velocity - n1.velocity;
    const glm::vec2 f2 = -(m / (t * t)) * k * d2 - (m / t) * b * v2;

    if (std::isnan(f1.x) || std::isnan(f1.y) || std::isnan(f2.x) || std::isnan(f2.y)) {
        return;
    }

    n1.acceleration += f1 / n1.mass;
    n2.acceleration += f2 / n2.mass;
}

void GraphVis::updateChargeAcceleration(GraphNode &node, const float q) const {
    glm::vec2 forces = glm::vec2(0.0F);
    for (const auto &other : nodes) {
        const float qSq = q * q;
        const glm::vec2 d = glm::normalize(other.position - node.position);
        const glm::vec2 dSq = d * d;
        const glm::vec2 f = -qSq / dSq;
        if (std::isnan(f.x) || std::isnan(f.y)) {
            continue;
        }
        forces += f;
    }

    node.acceleration += forces / node.mass;
}

void GraphVis::updateVelocityAndPosition(GraphNode &node, const float t) const {
    node.velocity += node.acceleration * t;
    node.position += node.velocity * t;
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

void GraphVis::resetGraph() {
    nodes.clear();
    edges.clear();

#if 1
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
#else
    nodes.emplace_back(glm::vec2(0.0F, 0.0F), glm::vec3(1.0F, 0.0F, 0.0F));
    nodes.emplace_back(glm::vec2(1.0F, 1.0F), glm::vec3(0.0F, 1.0F, 0.0F));
    nodes.emplace_back(glm::vec2(-1.0F, 1.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(-1.0F, -1.0F), glm::vec3(1.0F, 0.0F, 1.0F));
    nodes.emplace_back(glm::vec2(1.0F, -1.0F), glm::vec3(1.0F, 0.0F, 1.0F));
    //    nodes.emplace_back(glm::vec2(1.5F, 1.0F), glm::vec3(1.0F, 0.0F, 1.0F));

    edges.push_back({0, 1});
    edges.push_back({0, 2});
    edges.push_back({0, 3});
    edges.push_back({0, 4});
//    edges.push_back({0, 5});
#endif
}
