#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>

#include "opengl/IndexBuffer.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

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
