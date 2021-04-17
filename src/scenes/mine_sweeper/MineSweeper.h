#pragma once

#include <Scene.h>

#include <functional>

#include <Text.h>
#include <camera/Camera.h>
#include <gl/Shader.h>
#include <gl/VertexArray.h>

struct GameData {
    int width = 0;
    int height = 0;
    std::vector<bool> mines = {};
    std::vector<bool> opened = {};
    std::vector<bool> flags = {};
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

    GameData data = {};
    Text t = {};

    void renderQuad(const glm::vec2 &position, const glm::vec3 &color);
    void renderNumber(const glm::vec2 &pos, int num, float scale);

    void initGameData();
    static int countMines(GameData &data, int row, int col);
};
