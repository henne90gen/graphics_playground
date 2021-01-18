#pragma once

#include <Scene.h>

#include <functional>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

class AtmosphericScattering : public Scene {
  public:
    explicit AtmosphericScattering() : Scene("AtmosphericScattering"){};
    ~AtmosphericScattering() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> quadVA;
};
