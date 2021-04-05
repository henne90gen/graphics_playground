#pragma once

#include <gl/Shader.h>
#include <gl/VertexArray.h>
#include <glm/glm.hpp>
#include <memory>

class Clouds {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> quadVA;

    bool cloudsEnabled = false;
    glm::vec3 skyPosition = glm::vec3(0.0F, 200.0F, 0.0F);
    glm::vec3 skyRotation = glm::vec3(glm::pi<float>() / 2.0F, 0.0F, 0.0F);
    glm::vec3 skyScale = glm::vec3(2000.0F, 2000.0F, 1.0F);
    glm::vec3 skyColor = glm::vec3(0.529F, 0.808F, 0.922F);
    float animationSpeed = 0.005F;
    float cloudBlend = 0.1F;

  public:
    void init();
    void showGui();
    void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime);
    void renderClouds(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime);
};
