#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"

class LSystems : public Scene {
  public:
    explicit LSystems(SceneData &data) : Scene(data, "LSystems"){};
    ~LSystems() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
};
