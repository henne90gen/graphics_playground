#include "MineSweeper.h"

#include <Main.h>
#include <random>
#include <util/RenderUtils.h>

DEFINE_SCENE_MAIN(MineSweeper)
DEFINE_DEFAULT_SHADERS(mine_sweeper_MineSweeper)

void MineSweeper::setup() {
    auto aspectRatio = getAspectRatio();
    projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0F, 1.0F);
    shader = CREATE_DEFAULT_SHADER(mine_sweeper_MineSweeper);
    quadVA = createQuadVA(shader);

    const std::string fontPath = "mine_sweeper_resources/assets/Vonique 64.ttf";
    text.load(fontPath, 50);

    initGameData();

    GL_Call(glEnable(GL_BLEND));
    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void MineSweeper::onAspectRatioChange() {
    auto aspectRatio = getAspectRatio();
    projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0F, 1.0F);
}

void MineSweeper::destroy() {}

void MineSweeper::tick() {
    static float zoom = 0.1F;

    auto mousePixelPos = getInput().mouse.pos;

    const auto displayWidthF = static_cast<float>(getWidth());
    const auto displayHeightF = static_cast<float>(getHeight());

    auto mouseDisplaySpace =
          glm::vec2(mousePixelPos.x / displayWidthF, (displayHeightF - mousePixelPos.y) / displayHeightF);
    mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

    auto adjustedDisplayPos = glm::vec4(mouseDisplaySpace, 0.0F, 1.0F);
    auto mousePos = glm::vec3(adjustedDisplayPos.x, adjustedDisplayPos.y, 0.0F);
    mousePos.x *= getAspectRatio();

    if (!data.lost) {
        static bool previousRightMouseButton = false;
        if (previousRightMouseButton && !getInput().mouse.right) {
            toggleFlag(mousePos);
        }
        previousRightMouseButton = getInput().mouse.right;

        static bool previousLeftMouseButton = false;
        if (previousLeftMouseButton && !getInput().mouse.left) {
            uncoverField(mousePos);
        }
        previousLeftMouseButton = getInput().mouse.left;
    }

    ImGui::Begin("Settings");
    ImGui::Text("Pixel Pos: (%f,%f)", mousePixelPos.x, mousePixelPos.y);
    ImGui::Text("Mouse Pos: (%f,%f,%f)", mousePos.x, mousePos.y, mousePos.z);
    ImGui::DragFloat("Zoom", &zoom, 0.001F);
    if (data.lost) {
        ImGui::Text("You lost!");
        if (ImGui::Button("New Game")) {
            initGameData();
        }
    }
    ImGui::End();

    viewMatrix = glm::identity<glm::mat4>();
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom));

    // renderGame();
    // renderBackground();
    renderNumbers();
    // renderForeground();
}

int convertToIndex(const GameData &data, const glm::vec2 &position) {
    const int x = static_cast<int>((position.x * 10.0F) + 0.5F);
    const int y = static_cast<int>((position.y * 10.0F) + 0.5F);
    return y * data.width + x;
}

void MineSweeper::toggleFlag(const glm::vec2 &position) {
    const int index = convertToIndex(data, position);
    if (index < 0 || index > data.width * data.height) {
        return;
    }
    data.flags[index] = !data.flags[index];
}

void MineSweeper::uncoverField(const glm::vec2 &position) {
    int index = convertToIndex(data, position);
    if (index < 0 || index > data.width * data.height) {
        return;
    }

    if (data.opened[index]) {
        return;
    }

    if (data.mines[index]) {
        data.lost = true;
        for (int i = 0; i < data.width * data.height; i++) {
            if (data.mines[i]) {
                data.opened[i] = true;
            }
        }
        return;
    }

    std::vector<int> queue = {};
    queue.push_back(index);
    while (!queue.empty()) {
        index = queue.back();
        queue.pop_back();
        if (data.mines[index]) {
            continue;
        }

        data.opened[index] = true;

        if (data.numbers[index] > 0) {
            continue;
        }

        const int thisRow = index / data.width;
        const int thisCol = index % data.width;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                auto currentIndex = index + i + j * data.width;
                if (currentIndex == index) {
                    continue;
                }
                if (index < 0 || index > data.width * data.height) {
                    continue;
                }

                const int row = currentIndex / data.width;
                const int col = currentIndex % data.width;
                if (abs(thisRow - row) > 1 || abs(thisCol - col) > 1) {
                    continue;
                }

                if (data.opened[currentIndex]) {
                    continue;
                }

                queue.push_back(currentIndex);
            }
        }
    }
}

void MineSweeper::renderForeground() {
    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.height; col++) {
            const float x = static_cast<float>(col);
            const float y = static_cast<float>(row);
            const int i = row * data.width + col;
            if (data.opened[i]) {
                continue;
            }

            auto color = glm::vec3(0.8, 0.8, 0.8);
            if (data.flags[i]) {
                color = glm::vec3(0.0, 0.0, 1.0);
            }
            const auto position = glm::vec2(x, y);
            renderQuad(position, 1.0F, color);
        }
    }
}

void MineSweeper::renderNumbers() {
    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.height; col++) {
            const float x = static_cast<float>(col);
            const float y = static_cast<float>(row);
            const int i = row * data.width + col;
            const glm::vec2 position = glm::vec2(x, y);
            renderNumber(position, 0.0F, data.numbers[i]);
        }
    }
}

void MineSweeper::renderBackground() {
    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.height; col++) {
            const float x = static_cast<float>(col);
            const float y = static_cast<float>(row);
            const int i = row * data.width + col;

            auto color = glm::vec3(1.0, 0.0, 0.0);
            if (data.mines[i]) {
                color = glm::vec3(0.0, 1.0, 0.0);
            }

            const glm::vec2 position = glm::vec2(x, y);
            renderQuad(position, -1.0F, color);
        }
    }
}

void MineSweeper::renderNumber(const glm::vec2 &position, float layer, int number) {
    const auto characterOpt = text.character(static_cast<char>(number + 48));
    if (!characterOpt.has_value()) {
        return;
    }

    const auto character = characterOpt.value();
    auto characterScale = character.scale();
    // characterScale.y *= -1.0F;

    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, glm::vec3(position, layer));
    // modelMatrix = glm::scale(modelMatrix, glm::vec3(characterScale, 1.0F));

    shader->bind();
    shader->setUniform("uvScale", characterScale);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("useTexture", true);
    shader->setUniform("textureSampler", 0);
    shader->setUniform("flatColor", glm::vec3(1.0F));

    character.texture.bind();

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void MineSweeper::renderQuad(const glm::vec2 &position, float layer, const glm::vec3 &color) {
    shader->bind();

    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, glm::vec3(position, layer));

    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("useTexture", false);
    shader->setUniform("flatColor", color);

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void MineSweeper::initGameData() {
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    data.lost = false;
    data.width = 10;
    data.height = 10;
    data.mines.resize(data.width * data.height);
    data.opened.resize(data.width * data.height);
    data.flags.resize(data.width * data.height);
    data.numbers.resize(data.width * data.height);

    int numMines = 0;
    int maxNumMines = 20;
    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.width; col++) {
            int i = row * data.width + col;
            double rand = distribution(generator);
            data.mines[i] = numMines < maxNumMines && rand > 0.7;
            if (data.mines[i]) {
                numMines++;
            }
            data.opened[i] = false;
            data.flags[i] = false;
        }
    }

    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.width; col++) {
            int i = row * data.width + col;
            data.numbers[i] = countMines(data, row, col);
        }
    }
}

int MineSweeper::countMines(GameData &data, int row, int col) {
    if (data.mines[row * data.width + col]) {
        return 0;
    }

    int mines = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (row + i < 0 || row + i >= data.height) {
                continue;
            }
            if (col + j < 0 || col + j >= data.width) {
                continue;
            }
            if (data.mines[(row + i) * data.width + (col + j)]) {
                mines++;
            }
        }
    }

    return mines;
}
