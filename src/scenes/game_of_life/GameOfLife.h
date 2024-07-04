#pragma once

#include "Scene.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"

#include <functional>

struct Board {
    unsigned int width = 200;
    unsigned int height = 200;

    glm::vec3 *pixels_front = nullptr;
    glm::vec3 *pixels_back = nullptr;
};

class GameOfLife : public Scene {
  public:
    explicit GameOfLife() : Scene("GameOfLife"){};
    ~GameOfLife() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    void renderCanvas(const glm::mat4 &viewMatrix);
    void checkForMouseClick(unsigned int canvasWidth, unsigned int canvasHeight, const glm::mat4 &viewMatrix);
    void setupDefaultGame();
    void stepGame();

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;

    Board board = {128, 128, {}};
};
