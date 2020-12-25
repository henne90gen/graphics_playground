#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>

#include "gl/IndexBuffer.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"

class Cube : public Scene {
  public:
    explicit Cube() : Scene("Cube"){};

    ~Cube() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<IndexBuffer> indexBuffer;
};
