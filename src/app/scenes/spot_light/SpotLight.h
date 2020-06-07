#pragma once

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "scenes/Scene.h"

#include <functional>

class SpotLight : public Scene {
  public:
    explicit SpotLight(SceneData &data) : Scene(data, "SpotLight"){};
    ~SpotLight() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> quadVA;
    glm::mat4 projectionMatrix;
    void moveKeyboardOnly(glm::vec3 &position, glm::vec3 &rotation, float walkSpeed, float rotationSpeed);
    std::shared_ptr<VertexArray> createWalls(const std::shared_ptr<Shader>& shared);
};

glm::vec3 calculateDirectionFromRotation(const glm::vec3 &rotation);
