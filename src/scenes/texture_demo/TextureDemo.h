#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

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
