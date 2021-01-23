#pragma once

#include <glm/glm.hpp>

#include "InputData.h"

/**
 * This camera class is copied from the Hazel project (https://github.com/TheCherno/Hazel)
 * https://github.com/TheCherno/Hazel/blob/96c57dadf5c3616ba3613034dfb63d3918590359/Hazel/src/Hazel/Renderer/EditorCamera.h
 * https://github.com/TheCherno/Hazel/blob/96c57dadf5c3616ba3613034dfb63d3918590359/Hazel/src/Hazel/Renderer/EditorCamera.cpp
 * https://github.com/TheCherno/Hazel/blob/96c57dadf5c3616ba3613034dfb63d3918590359/Hazel/src/Hazel/Renderer/Camera.h
 */
class Camera {
  public:
    Camera() = default;
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    void update(const InputData &input);

    void onScroll(double yOffset);

    void setViewportSize(float width, float height);
    inline void setFocalPoint(const glm::vec3 &point) { focalPoint = point; }

    glm::vec3 getUpDirection() const;
    glm::vec3 getRightDirection() const;
    glm::vec3 getForwardDirection() const;
    glm::quat getOrientation() const;
    glm::vec3 getPosition() const;
    const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }
    const glm::mat4 &getViewMatrix() const { return viewMatrix; }

  private:
    void updateProjection();
    void updateView();

    void mousePan(const glm::vec2 &delta);
    void mouseRotate(const glm::vec2 &delta);
    void mouseZoom(float delta);

    glm::vec2 panSpeed() const;
    float rotationSpeed() const;
    float zoomSpeed() const;

    float fov = 45.0F, aspectRatio = 1.778F, nearClip = 0.1F, farClip = 10000.0F;

    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    glm::vec3 focalPoint = {0.0F, 0.0F, 0.0F};

    glm::vec2 initialMousePosition = {0.0F, 0.0F};

    float distance = 10.0F;
    float pitch = 0.0F;
    float yaw = 0.0F;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    float viewportWidth = 1280, viewportHeight = 720;
};
