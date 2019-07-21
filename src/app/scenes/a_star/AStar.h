#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"
#include "a_star/AStarSolver.h"

#include <functional>
#include <memory>

class AStar : public Scene {
public:
    explicit AStar(SceneData data) : Scene(data, "AStar") {};

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

    void
    checkForMouseClick(const unsigned int canvasWidth, const unsigned int canvasHeight,
                       const glm::mat4 &viewMatrix, bool &isStartSelection, glm::vec2 &start, bool &isFinishSelection,
                       glm::vec2 &finish);

    void setupDefaultProblem();

    void reset();
};
