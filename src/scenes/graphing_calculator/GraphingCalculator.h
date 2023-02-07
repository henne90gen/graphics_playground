#pragma once

#include <Scene.h>

#include <functional>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

class GraphingCalculator : public Scene {
  public:
    explicit GraphingCalculator() : Scene("GraphingCalculator"){};
    ~GraphingCalculator() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> va;
};
