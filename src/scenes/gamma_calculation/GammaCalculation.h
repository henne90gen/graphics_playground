#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

class GammaCalculation : public Scene {
  public:
    explicit GammaCalculation() : Scene("GammaCalculation"){};

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
