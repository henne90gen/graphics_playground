#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"

class GraphVis : public Scene {
  public:
    explicit GraphVis(SceneData &data)
        : Scene(data, "GraphVis"){};
    ~GraphVis() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
    
  private:
    std::shared_ptr<Shader> shader;    
};
