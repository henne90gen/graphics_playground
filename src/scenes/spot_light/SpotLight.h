#pragma once

#include "Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

#include <functional>

class SpotLight : public Scene {
  public:
    explicit SpotLight() : Scene("SpotLight"){};
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
    std::shared_ptr<VertexArray> createWalls();
};

glm::vec3 calculateDirectionFromRotation(const glm::vec3 &rotation);
