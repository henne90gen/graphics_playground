#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>
#include <glad/glad.h>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/Shader.h"

class GammaCalculation : public Scene {
public:
    explicit GammaCalculation(SceneData data) : Scene(data, "GammaCalculation") {};

    ~GammaCalculation() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> checkerBoardTexture;

    void createCheckerBoard();

    static float calculateGammaValue(float color);
};
