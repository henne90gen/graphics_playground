#pragma once

#include <Scene.h>

#include <Model.h>
#include <functional>
#include <gl/IndexBuffer.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <memory>
#include <util/Camera.h>

#include "Layers.h"
#include "ShaderToggles.h"
#include "Sky.h"
#include "Terrain.h"
#include "Trees.h"

class Landscape : public Scene {
  public:
    explicit Landscape() : Scene("Landscape"){};

    ~Landscape() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<VertexArray> textureVA;
    std::shared_ptr<VertexArray> cubeVA;

    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<Shader> flatShader;

    Camera camera = Camera();

    Sky sky = {};
    Trees trees = {};
    Terrain terrain = {};

    void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition,
                     const glm::vec3 &lightColor);

    void renderTexture(const glm::vec3 &texturePosition, float zoom, const std::shared_ptr<Texture> &texture);
};
