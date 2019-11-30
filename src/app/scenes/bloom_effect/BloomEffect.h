#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"

class BloomEffect : public Scene {
  public:
    BloomEffect(SceneData &data) : Scene(data, "BloomEffect"){};

    ~BloomEffect() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;
};
