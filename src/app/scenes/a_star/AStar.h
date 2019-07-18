#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"

#include <functional>
#include <memory>

class AStar : public Scene {
public:
    AStar(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "AStar") {};

    ~AStar() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;

    std::vector<glm::vec3> board;
    unsigned int boardWidth = 128;
    unsigned int boardHeight = 128;

    void
    checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                       const glm::mat4 &viewMatrix, bool &isStartSelection, glm::vec2 &start, bool &isFinishSelection,
                       glm::vec2 &finish);

    void setupDefaultProblem();

    void resetBoard();
};
