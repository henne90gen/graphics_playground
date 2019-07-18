#include "AStar.h"

#include "opengl/VertexBuffer.h"

const glm::vec3 startColor = {0.0, 1.0, 0.0};
const glm::vec3 finishColor = {1.0, 0.0, 0.0};

void AStar::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/a_star/AStarVert.glsl",
                                      "../../../src/app/scenes/a_star/AStarFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
            -1.0, -1.0, 0.0, 0.0, 0.0, //
            1.0, -1.0, 0.0, 1.0, 0.0, //
            1.0, 1.0, 0.0, 1.0, 1.0, //
            -1.0, 1.0, 0.0, 0.0, 1.0
    };
    BufferLayout bufferLayout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_UV"}
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2},
            {0, 2, 3},
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);

    texture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_TextureSampler", 0);
}

void AStar::destroy() {}

void AStar::tick() {
    static float zoom = 1.0f;
    static bool isStartSelection = true;
    static glm::vec2 start = {};
    static bool isFinishSelection = false;
    static glm::vec2 finish = {};
    static bool init = false;

    if (!init) {
        resetBoard();
        init = true;
    }

    ImGui::Begin("Settings");
    ImGui::DragFloat("Zoom", &zoom);
    if (ImGui::Button("Select Start")) {
        isStartSelection = true;
        isFinishSelection = false;
    }
    ImGui::Text("Start: %f, %f", start.x, start.y);
    if (ImGui::Button("Select Finish")) {
        isStartSelection = false;
        isFinishSelection = true;
    }
    ImGui::Text("Finish: %f, %f", finish.x, finish.y);
    if (ImGui::Button("Use Default Problem")) {
        setupDefaultProblem();
    }
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    texture->bind();

    auto viewMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(zoom));

    checkForMouseClick(boardWidth, boardHeight, viewMatrix, isStartSelection, start, isFinishSelection, finish);
    texture->update(board, boardWidth, boardHeight);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();
    shader->unbind();
}

void AStar::resetBoard() {
    board = std::vector<glm::vec3>(boardWidth * boardHeight);
    for (auto &color : board) {
        color = {1.0, 0.0, 1.0};
    }
}

void AStar::checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                               const glm::mat4 &viewMatrix, bool &isStartSelection, glm::vec2 &start,
                               bool &isFinishSelection, glm::vec2 &finish) {
    InputData *input = getInput();
    if (input->mouse.left) {
        const auto widthF = static_cast<float>(canvasWidth);
        const auto heightF = static_cast<float>(canvasHeight);
        const auto displayWidthF = static_cast<float>(getWidth());
        const auto displayHeightF = static_cast<float>(getHeight());

        auto &mousePos = input->mouse.pos;

        auto mouseDisplaySpace = glm::vec2(mousePos.x / displayWidthF, (displayHeightF - mousePos.y) / displayHeightF);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

        auto adjustedDisplayPos = glm::inverse(viewMatrix) * glm::vec4(mouseDisplaySpace, 0.0, 0.0);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        auto canvasPos = (glm::vec2(adjustedDisplayPos.x, adjustedDisplayPos.y) + glm::vec2(1.0F, 1.0F)) / 2.0F;
        canvasPos = glm::vec2(canvasPos.x * widthF, heightF - (canvasPos.y * heightF));

        if ((canvasPos.x < 0.0F || canvasPos.x >= widthF) || (canvasPos.y < 0.0F || canvasPos.y >= heightF)) {
            return;
        }

        unsigned int i =
                (canvasHeight - static_cast<unsigned int>(canvasPos.y)) * canvasWidth +
                static_cast<unsigned int>(canvasPos.x);

        glm::vec3 color = {0.0, 0.0, 0.0};
        if (isStartSelection) {
            color = startColor;
            start = canvasPos;
            isStartSelection = false;
        } else if (isFinishSelection) {
            color = finishColor;
            finish = canvasPos;
            isFinishSelection = false;
        }
        if (board[i] != startColor && board[i] != finishColor) {
            board[i] = color;
        }
    }
}

void AStar::setupDefaultProblem() {
    resetBoard();
    board[10 * boardWidth + 10] = startColor;
    board[100 * boardWidth + 100] = finishColor;
}
