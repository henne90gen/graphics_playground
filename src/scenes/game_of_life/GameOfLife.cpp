#include "GameOfLife.h"

#include "Main.h"
#include "util/RenderUtils.h"

#include <array>

DEFINE_SCENE_MAIN(GameOfLife)
DEFINE_DEFAULT_SHADERS(game_of_life_GameOfLife)

const glm::vec3 aliveColor = {45.0 / 255.0, 57.0 / 255.0, 66.0 / 255.0};
const glm::vec3 deadColor = {156.0 / 255.0, 140.0 / 255.0, 135.0 / 255.0};

void GameOfLife::setup() {
    GL_Call(glDisable(GL_DEPTH_TEST));

    shader = CREATE_DEFAULT_SHADER(game_of_life_GameOfLife);
    shader->bind();

    vertexArray = createQuadVA(shader, {2.0F, 2.0F});
    vertexArray->bind();

    auto textureSettings = TextureSettings();
    textureSettings.minificationFilter = GL_NEAREST;
    textureSettings.magnificationFilter = GL_NEAREST;
    texture = std::make_shared<Texture>(textureSettings);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    shader->setUniform("u_TextureSampler", 0);

    setupDefaultGame();
}

void GameOfLife::destroy() { GL_Call(glEnable(GL_DEPTH_TEST)); }

void GameOfLife::tick() {
    static float zoom = 1.0F;
    static glm::vec3 position = {0.0F, 0.0F, 0.0F}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool runGame = false;

    ImGui::Begin("Settings");
    const float dragSpeed = 0.001F;
    ImGui::DragFloat("Zoom", &zoom, dragSpeed);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position), dragSpeed);
    if (ImGui::Button("Reset")) {
        setupDefaultGame();
        runGame = false;
    }
    if (runGame) {
        if (ImGui::Button("Stop Animation")) {
            runGame = false;
        }
    } else {
        if (ImGui::Button("Start Animation")) {
            runGame = true;
        }
    }
    ImGui::End();

    if (runGame) {
        RECORD_SCOPE_NAME("GameStep");
        stepGame();
    }

    auto viewMatrix = glm::translate(glm::identity<glm::mat4>(), position);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom));

    renderCanvas(viewMatrix);
}

void GameOfLife::renderCanvas(const glm::mat4 &viewMatrix) {
    shader->bind();
    vertexArray->bind();
    texture->bind();

    checkForMouseClick(board.width, board.height, viewMatrix);
    texture->update(board.pixels_front, board.width, board.height);

    shader->setUniform("u_ViewMatrix", viewMatrix);
    shader->setUniform("u_RenderCanvas", true);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();
    shader->unbind();
}

void GameOfLife::checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                                    const glm::mat4 &viewMatrix) {
    const InputData &input = getInput();
    if (!input.mouse.left) {
        return;
    }

    const auto &mousePos = input.mouse.pos;
    auto mappedMousePos = mapMouseOntoCanvas(mousePos, viewMatrix, canvasWidth, canvasHeight, getWidth(), getHeight());
    auto canvasPos = mappedMousePos.canvasPos;

    if ((canvasPos.x < 0.0F || canvasPos.x >= static_cast<float>(canvasWidth)) ||
        (canvasPos.y < 0.0F || canvasPos.y >= static_cast<float>(canvasHeight))) {
        return;
    }

    unsigned int i = (canvasHeight - static_cast<unsigned int>(canvasPos.y)) * canvasWidth +
                     static_cast<unsigned int>(canvasPos.x);

    board.pixels_front[i] = aliveColor;
}

void GameOfLife::setupDefaultGame() {
    board.pixels_front = (glm::vec3 *)malloc(sizeof(glm::vec3) * board.width * board.height);
    for (int i = 0; i < board.width * board.height; i++) {
        board.pixels_front[i] = deadColor;
    }

    board.pixels_back = (glm::vec3 *)malloc(sizeof(glm::vec3) * board.width * board.height);
    for (int i = 0; i < board.width * board.height; i++) {
        board.pixels_back[i] = deadColor;
    }
}

bool is_in_bounds(Board &board, int x, int y) { return x >= 0 and x < board.width and y >= 0 and y < board.height; }

int count_neighbors(Board &board, int x, int y) {
    int result = 0;
    std::array<std::pair<int, int>, 8> neighbors = {
          std::make_pair(-1, 1), std::make_pair(0, 1),   std::make_pair(1, 1),  std::make_pair(-1, 0),
          std::make_pair(1, 0),  std::make_pair(-1, -1), std::make_pair(0, -1), std::make_pair(1, -1),
    };
    for (auto const &neighbor : neighbors) {
        auto const nx = x + neighbor.first;
        auto const ny = y + neighbor.second;

        if (not is_in_bounds(board, nx, ny)) {
            continue;
        }

        if (board.pixels_front[ny * board.width + nx] != aliveColor) {
            continue;
        }

        result++;
    }

    return result;
}

void GameOfLife::stepGame() {
    for (int i = 0; i < board.width * board.height; i++) {
        int x = i % board.width;
        int y = i / board.width;

        board.pixels_back[y * board.width + x] = board.pixels_front[y * board.width + x];

        const auto neighbor_count = count_neighbors(board, x, y);
        if (board.pixels_front[y * board.width + x] == aliveColor) {
            if (neighbor_count < 2) {
                // Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                board.pixels_back[y * board.width + x] = deadColor;
            } else if (neighbor_count > 3) {
                // Any live cell with more than three live neighbours dies, as if by overpopulation.
                board.pixels_back[y * board.width + x] = deadColor;
            } else {
                // Any live cell with two or three live neighbours lives on to the next generation.
                board.pixels_back[y * board.width + x] = aliveColor;
            }
        } else {
            if (neighbor_count == 3) {
                // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                board.pixels_back[y * board.width + x] = aliveColor;
            }
        }
    }

    const auto pixels_tmp = board.pixels_front;
    board.pixels_front = board.pixels_back;
    board.pixels_back = pixels_tmp;
}
