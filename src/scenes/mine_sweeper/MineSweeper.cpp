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
    t.load(fontPath, 50);

    initGameData();
}

void MineSweeper::onAspectRatioChange() {
    auto aspectRatio = getAspectRatio();
    projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0F, 1.0F);
}

void MineSweeper::destroy() {}

void MineSweeper::tick() {
    static float scale = 0.5F;

    auto mousePixelPos = getInput().mouse.pos;

    const auto displayWidthF = static_cast<float>(getWidth());
    const auto displayHeightF = static_cast<float>(getHeight());

    auto mouseDisplaySpace =
          glm::vec2(mousePixelPos.x / displayWidthF, (displayHeightF - mousePixelPos.y) / displayHeightF);
    mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

    auto adjustedDisplayPos = glm::vec4(mouseDisplaySpace, 0.0F, 1.0F);
    auto mousePos = glm::vec3(adjustedDisplayPos.x, adjustedDisplayPos.y, 0.0F);
    mousePos.x *= getAspectRatio();

    ImGui::Begin("Settings");
    ImGui::Text("Pixel Pos: (%f,%f)", mousePixelPos.x, mousePixelPos.y);
    ImGui::Text("Mouse Pos: (%f,%f,%f)", mousePos.x, mousePos.y, mousePos.z);
    ImGui::DragFloat("Character Scale", &scale, 0.001F);
    ImGui::End();

    for (int row = 0; row < data.height; row++) {
        for (int col = 0; col < data.height; col++) {
            float x = static_cast<float>(col) * 0.1F;
            float y = static_cast<float>(row) * 0.1F;
            int i = row * data.width + col;
            glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);
            if (data.mines[i]) {
                color = glm::vec3(0.0, 1.0, 0.0);
            }
            const glm::vec2 pos = glm::vec2(x, y);
            renderQuad(pos, color);
            renderNumber(pos, data.numbers[i], scale);
        }
    }
}

void MineSweeper::renderNumber(const glm::vec2 &pos, int num, float scale) {
    std::optional<Character> characterOpt = t.character(static_cast<char>(num + 48));
    if (!characterOpt.has_value()) {
        return;
    }

    auto character = characterOpt.value();
    auto offset = character.offset(scale);
    glm::vec2 finalTranslation = pos - offset;
    auto characterScale = character.scale();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(finalTranslation, 0.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(characterScale, 1.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, 1.0F));
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", glm::identity<glm::mat4>());
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("useTexture", true);
    shader->setUniform("textureSampler", 0);
    shader->setUniform("flatColor", glm::vec3(1.0F));

    character.texture.bind();

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void MineSweeper::renderQuad(const glm::vec2 &position, const glm::vec3 &color) {
    shader->bind();
    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1F));
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", glm::identity<glm::mat4>());
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("useTexture", false);
    shader->setUniform("flatColor", color);

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void MineSweeper::initGameData() {
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

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
            if (row == 0 && col == 0) {
                continue;
            }
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
