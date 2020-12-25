#pragma once

#include "Scene.h"

#include <glad/glad.h>
#include <memory>

#include "gl/Shader.h"
#include "gl/VertexBuffer.h"

class Triangle : public Scene {
  public:
    explicit Triangle() : Scene("Triangle") {}

    ~Triangle() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;

    VertexBuffer *positionBuffer;
    VertexBuffer *colorBuffer;

    GLuint positionLocation;
    GLuint colorLocation;
};