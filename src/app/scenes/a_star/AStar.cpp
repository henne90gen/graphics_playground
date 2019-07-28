#include "AStar.h"

#include "opengl/VertexBuffer.h"
#include "util/OpenGLUtils.h"

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
    shader->setUniform("u_TextureSampler", 0);

    solver = std::make_unique<AStarSolver>();
}

void AStar::destroy() {}

void AStar::tick() {
    static glm::vec3 position = {0.5F, 0.5F, 0.0F};
    static float zoom = 0.5F;
    static bool isStartSelection = true;
    static glm::vec2 start = {};
    static bool isFinishSelection = false;
    static glm::vec2 finish = {};
    static bool animate = false;
    static bool init = false;

    if (!init) {
        reset();
        init = true;

        setupDefaultProblem();
    }

    ImGui::Begin("Settings");
    ImGui::DragFloat("Zoom", &zoom, 0.001F);
    ImGui::DragFloat3("Position", (float *) &position, 0.001F);
    if (ImGui::Button("Reset")) {
        isStartSelection = true;
        reset();
    }
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
    ImGui::Checkbox("Animate", &animate);
    ImGui::End();

    if (animate) {
        solver->nextStep(board);
    }

    shader->bind();
    vertexArray->bind();
    texture->bind();

    auto viewMatrix = glm::translate(glm::identity<glm::mat4>(), position);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom));

    checkForMouseClick(board.width, board.height, viewMatrix, isStartSelection, start, isFinishSelection, finish);
    texture->update(board.pixels, board.width, board.height);

    shader->setUniform("u_ViewMatrix", viewMatrix);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();
    shader->unbind();
}

void AStar::reset() {
    solver = std::make_unique<AStarSolver>();
    board.pixels = std::vector<glm::vec3>(board.width * board.height);
    for (auto &color : board.pixels) {
        color = backgroundColor;
    }
}

void AStar::checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                               const glm::mat4 &viewMatrix, bool &isStartSelection, glm::vec2 &start,
                               bool &isFinishSelection, glm::vec2 &finish) {
    InputData *input = getInput();
    if (!input->mouse.left) {
        return;
    }

    auto mappedMousePos = mapMouseOntoCanvas(input, viewMatrix, canvasWidth, canvasHeight, getWidth(), getHeight());
    auto canvasPos = mappedMousePos.canvasPos;

    if ((canvasPos.x < 0.0F || canvasPos.x >= canvasWidth) || (canvasPos.y < 0.0F || canvasPos.y >= canvasHeight)) {
        return;
    }

    unsigned int i =
            (canvasHeight - static_cast<unsigned int>(canvasPos.y)) * canvasWidth +
            static_cast<unsigned int>(canvasPos.x);

    glm::vec3 color = obstacleColor;
    if (isStartSelection) {
        color = startColor;
        start = canvasPos;
        isStartSelection = false;
    } else if (isFinishSelection) {
        color = finishColor;
        finish = canvasPos;
        isFinishSelection = false;
    }
    if (board.pixels[i] != startColor && board.pixels[i] != finishColor) {
        board.pixels[i] = color;
    }
}

void AStar::setupDefaultProblem() {
    reset();
    board.pixels[10 * board.width + 10] = startColor;
    board.pixels[100 * board.width + 100] = finishColor;
}
