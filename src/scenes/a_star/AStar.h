#pragma once

#include "AStarSolver.h"
#include "Scene.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"

#include <functional>
#include <memory>

class AStar : public Scene {
  public:
    explicit AStar() : Scene("AStar"){};

    ~AStar() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;
    std::unique_ptr<AStarSolver> solver;

    Board board = {128, 128, {}};

    void checkForMouseClick(unsigned int canvasWidth, unsigned int canvasHeight, const glm::mat4 &viewMatrix);

    void setupDefaultProblem();
    void resetSolver();
    void setStartAndFinish();

    void renderCanvas(const glm::mat4 &viewMatrix);

    void visualizeNodeSet(const std::vector<Node *> &nodes, const glm::mat4 &viewMatrix);
};
