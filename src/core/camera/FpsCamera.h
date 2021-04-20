#pragma once

#include <glm/glm.hpp>

#include "util/InputData.h"

class FpsCamera {
  public:
    void update(const InputData &input);

    void onScroll(double yOffset);

    void setViewportSize(float width, float height);
    void setPosition(const glm::vec3 &point) { position = point; }

    [[nodiscard]] glm::vec3 getUpDirection() const;
    [[nodiscard]] glm::vec3 getRightDirection() const;
    [[nodiscard]] glm::vec3 getForwardDirection() const;
    [[nodiscard]] glm::quat getOrientation() const;
    [[nodiscard]] glm::vec3 getPosition() const;
    [[nodiscard]] const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }
    [[nodiscard]] const glm::mat4 &getViewMatrix() const { return viewMatrix; }

    [[nodiscard]] float getYaw() const { return yaw; }
    [[nodiscard]] float getPitch() const { return pitch; }

  private:
    float fov = 45.0F, aspectRatio = 1.778F, nearClip = 0.1F, farClip = 10000.0F;

    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    float pitch = 0.0F;
    float yaw = 0.0F;

    glm::vec2 initialMousePosition = {0.0F, 0.0F};

    glm::mat4 projectionMatrix = glm::mat4(1.0F);
    glm::mat4 viewMatrix = glm::mat4(1.0F);

    float viewportWidth = 1280, viewportHeight = 720;
};
