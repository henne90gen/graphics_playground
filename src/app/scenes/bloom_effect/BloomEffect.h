#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"
#include "opengl/Model.h"

class BloomEffect : public Scene {
  public:
    BloomEffect(SceneData &data) : Scene(data, "BloomEffect"){};

    ~BloomEffect() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange()override;

  private:
    std::shared_ptr<Shader> shader;
    std::unique_ptr<Model> model = {};

    glm::mat4 projectionMatrix;

    void
    drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
              const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation, const bool drawWireframe) const;
};
