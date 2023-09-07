#pragma once

#include <Scene.h>

#include <functional>

#include <Text.h>
#include <camera/Camera.h>
#include <gl/Shader.h>
#include <gl/VertexArray.h>

struct GameData {
    /// whether the player has lost the game
    bool lost = false;

    /// the width of the game board in number of fields
    int width = 0;

    /// the height of the game board in number of fields
    int height = 0;

    /// whether a field contains a mine
    std::vector<bool> mines = {};

    /// whether a field has been opened
    std::vector<bool> opened = {};

    /// whether a flag has been placed on a field
    std::vector<bool> flags = {};

    /// how many mines are present next to each field
    std::vector<int> numbers = {};
};

class MineSweeper : public Scene {
  public:
    explicit MineSweeper() : Scene("MineSweeper"){};
    ~MineSweeper() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> quadVA;
    glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
    glm::mat4 viewMatrix = glm::identity<glm::mat4>();

    GameData data = {};
    Text text = {};

    void renderForeground();
    void renderNumbers();
    void renderBackground();
    void renderQuad(const glm::vec2 &position, float layer, const glm::vec3 &color);
    void renderNumber(const glm::vec2 &position, float layer, int number);

    void toggleFlag(const glm::vec2 &position);
    void uncoverField(const glm::vec2 &position);

    void initGameData();
    static int countMines(GameData &data, int row, int col);
};
