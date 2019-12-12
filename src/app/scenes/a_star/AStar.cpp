#include "AStar.h"

#include "opengl/VertexBuffer.h"
#include "util/OpenGLUtils.h"
#include "util/RenderUtils.h"

void AStar::setup() {
    GL_Call(glDisable(GL_DEPTH_TEST));

    shader = std::make_shared<Shader>("scenes/a_star/AStarVert.glsl", "scenes/a_star/AStarFrag.glsl");
    shader->bind();

    vertexArray = createQuadVA(shader);
    vertexArray->bind();

    texture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);

    solver = std::make_unique<AStarSolver>();
}

void AStar::destroy() { GL_Call(glEnable(GL_DEPTH_TEST)); }

void AStar::tick() {
    // TODO(henne): move camera so that whole canvas can be seen
    static float zoom = 1.0F;
    static glm::vec3 position = {0.5F, 0.5F, 0.0F};
    static bool animate = false;
    static bool init = false;
    static bool renderDirectionArrows = true;

    if (!init) {
        setupDefaultProblem();
        init = true;
    }

    ImGui::Begin("Settings");
    ImGui::DragFloat("Zoom", &zoom, 0.001F);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position), 0.001F);
    if (ImGui::Button("Reset")) {
        setupDefaultProblem();
        animate = false;
    }
    if (animate) {
        if (ImGui::Button("Stop Animation")) {
            animate = false;
        }
    } else {
        if (ImGui::Button("Start Animation")) {
            animate = true;
        }
    }
    ImGui::Checkbox("Use Manhatten-Distance", &solver->useManhattenDistance);
    ImGui::Checkbox("Draw direction arrows", &renderDirectionArrows);
    ImGui::Text("Size WorkingSet: %lu", solver->workingSet.size());
    float estimatedDistance = -1.0F;
    if (!solver->workingSet.empty()) {
        estimatedDistance = solver->workingSet.back()->f;
    }
    ImGui::Text("Total Estimated Distance: %f", estimatedDistance);
    ImGui::Text("Solved: %d", static_cast<int>(solver->solved));
    ImGui::Text("Final Node: %p", (void *)solver->finalNode);
    if (solver->finalNode != nullptr) {
        ImGui::Text("Total Distance Traveled: %d", solver->finalNode->g);
    }
    ImGui::End();

    if (animate) {
        solver->nextStep(board);
    }

    auto viewMatrix = glm::translate(glm::identity<glm::mat4>(), position);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom));

    renderCanvas(viewMatrix);

    if (renderDirectionArrows) {
        visualizeNodeSet(solver->workingSet, viewMatrix);
        visualizeNodeSet(solver->visitedSet, viewMatrix);
    }
}

void AStar::visualizeNodeSet(const std::vector<Node *> &nodes, const glm::mat4 &viewMatrix) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", false);
    shader->setUniform("u_ViewMatrix", viewMatrix);

    for (auto &node : nodes) {
        if (node->predecessor == nullptr) {
            continue;
        }

        std::vector<glm::vec3> vertices = {};
        glm::vec3 start = {node->position.x, node->position.y, 0.0F};
        glm::vec3 end = {node->predecessor->position.x, node->predecessor->position.y, 0.0F};

        start += glm::vec3(0.5, 0.5, 0.0);
        start = {start.x / static_cast<float>(board.width), start.y / static_cast<float>(board.height), start.z};
        start *= 2.0F;
        start -= glm::vec3(1, 1, 0);

        end += glm::vec3(0.5, 0.5, 0.0);
        end = {end.x / static_cast<float>(board.width), end.y / static_cast<float>(board.height), end.z};
        end *= 2.0F;
        end -= glm::vec3(1, 1, 0);

        glm::vec3 arrowDir = end - start;
        arrowDir *= 0.8F;
        glm::vec3 arrowCenter = start + (arrowDir * 0.75F);
        glm::vec3 arrowNormal = {arrowDir.y, -arrowDir.x, arrowDir.z};
        const float arrowWidth = glm::length(arrowDir) * 0.25F;
        arrowNormal = glm::normalize(arrowNormal) * arrowWidth;
        glm::vec3 left = arrowCenter + arrowNormal;
        glm::vec3 arrowPeak = start + arrowDir;
        glm::vec3 right = arrowCenter + (arrowNormal * -1.0F);

        vertices.push_back(start);
        vertices.push_back(arrowCenter);
        vertices.push_back(left);
        vertices.push_back(arrowPeak);
        vertices.push_back(right);
        vertices.push_back(arrowCenter);

        auto va = VertexArray(shader);
        BufferLayout layout = {{ShaderDataType::Float3, "a_Position"}};
        auto vb = std::make_shared<VertexBuffer>(vertices, layout);
        va.addVertexBuffer(vb);

        std::vector<unsigned int> indices = {0, 1, //
                                             1, 2, //
                                             2, 3, //
                                             3, 4, //
                                             4, 5};
        auto ib = std::make_shared<IndexBuffer>(indices);
        va.setIndexBuffer(ib);

        va.bind();
        GL_Call(glDrawElements(GL_LINES, va.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }
}

void AStar::renderCanvas(const glm::mat4 &viewMatrix) {
    shader->bind();
    vertexArray->bind();
    texture->bind();

    checkForMouseClick(board.width, board.height, viewMatrix);
    texture->update(board.pixels, board.width, board.height);

    shader->setUniform("u_ViewMatrix", viewMatrix);
    shader->setUniform("u_RenderCanvas", true);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();
    shader->unbind();
}

void AStar::checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                               const glm::mat4 &viewMatrix) {
    InputData *input = getInput();
    if (!input->mouse.left) {
        return;
    }

    auto mappedMousePos = mapMouseOntoCanvas(input, viewMatrix, canvasWidth, canvasHeight, getWidth(), getHeight());
    auto canvasPos = mappedMousePos.canvasPos;

    if ((canvasPos.x < 0.0F || canvasPos.x >= canvasWidth) || (canvasPos.y < 0.0F || canvasPos.y >= canvasHeight)) {
        return;
    }

    unsigned int i = (canvasHeight - static_cast<unsigned int>(canvasPos.y)) * canvasWidth +
                     static_cast<unsigned int>(canvasPos.x);

    if (board.pixels[i] != startColor && board.pixels[i] != finishColor) {
        board.pixels[i] = obstacleColor;
    }
}

void AStar::setupDefaultProblem() {
    solver = std::make_unique<AStarSolver>();
    board.pixels = std::vector<glm::vec3>(board.width * board.height);
    for (auto &color : board.pixels) {
        color = backgroundColor;
    }
    board.pixels[10 * board.width + 10] = startColor;
    board.pixels[100 * board.width + 100] = finishColor;
}
