#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>
#include <memory>

#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"

class TextureDemo : public Scene {
  public:
    explicit TextureDemo() : Scene("TextureDemo"){};

    ~TextureDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;

    void updateTexture(std::array<float, 3> color, bool checkerBoard);
};
