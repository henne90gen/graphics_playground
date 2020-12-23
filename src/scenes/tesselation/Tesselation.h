#pragma once

#include "Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

class Tesselation : public Scene {
  public:
    explicit Tesselation() : Scene("Tesselation"){};
    ~Tesselation() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> va;
};
