#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <glad/glad.h>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/Shader.h"

class GammaCalculation : public Scene {
public:
    GammaCalculation(GLFWwindow *window, std::function<void(void)> &backToMainMenu) :
            Scene(window, backToMainMenu, "GammaCalculation") {};

    ~GammaCalculation() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray;
    Texture *checkerBoardTexture;
    Shader *shader;

    void createCheckerBoard();

    static float calculateGammaValue(float color);
};
