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
    float fov = 45.0F, aspectRatio = 1.778F, nearClip = 0.1F, farClip = 1000.0F;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    glm::vec3 focalPoint = {0.0F, 0.0F, 0.0F};

    glm::vec2 initialMousePosition = {0.0F, 0.0F};

    float distance = 10.0F;
    float pitch = 0.0F;
    float yaw = 0.0F;

    float viewportWidth = 1280, viewportHeight = 720;

    Camera() = default;
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    void update(const InputData *input);

    // TODO make scroll to zoom possible
    // void OnEvent(Event &e);
    // void Camera::OnEvent(Event &e) {
    //    EventDispatcher dispatcher(e);
    //    dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(Camera::OnMouseScroll));
    //}
    //
    // bool Camera::OnMouseScroll(MouseScrolledEvent &e) {
    //    float delta = e.GetYOffset() * 0.1f;
    //    mouseZoom(delta);
    //    updateView();
    //    return false;
    //}

    inline void SetViewportSize(float width, float height) {
        viewportWidth = width;
        viewportHeight = height;
        updateProjection();
    }

    glm::vec3 GetUpDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetForwardDirection() const;
    glm::quat GetOrientation() const;

  private:
    void updateProjection();
    void updateView();

    void mousePan(const glm::vec2 &delta);
    void mouseRotate(const glm::vec2 &delta);
    void mouseZoom(float delta);

    glm::vec3 CalculatePosition() const;

    glm::vec2 panSpeed() const;
    float rotationSpeed() const;
    float zoomSpeed() const;
};
