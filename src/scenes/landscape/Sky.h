#pragma once

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <glm/glm.hpp>
#include <memory>

class Sky {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> cubeVA;

    glm::vec3 skyScale = glm::vec3(2000.0F, 400.0F, 2000.0F);
    glm::vec3 skyColor = glm::vec3(0.529F, 0.808F, 0.922F);
    float animationSpeed = 0.005F;
    float cloudBlend = 0.1F;

  public:
    void init();
    void showGui();
    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime);
};
