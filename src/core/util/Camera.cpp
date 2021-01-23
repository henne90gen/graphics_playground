#include "Camera.h"

#include <GLFW/glfw3.h>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
    : fov(fov), aspectRatio(aspectRatio), nearClip(nearClip), farClip(farClip),
      projectionMatrix(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)) {
    updateView();
}

void Camera::updateProjection() {
    aspectRatio = viewportWidth / viewportHeight;
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
}

void Camera::updateView() {
    // m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
    position = CalculatePosition();

    glm::quat orientation = GetOrientation();
    viewMatrix = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
    viewMatrix = glm::inverse(viewMatrix);
}

glm::vec2 Camera::panSpeed() const {
    float x = std::min(viewportWidth / 1000.0F, 2.4F); // max = 2.4f
    float xFactor = 0.0366F * (x * x) - 0.1778F * x + 0.3021F;

    float y = std::min(viewportHeight / 1000.0F, 2.4f); // max = 2.4f
    float yFactor = 0.0366F * (y * y) - 0.1778F * y + 0.3021F;

    return glm::vec2(xFactor, yFactor);
}

float Camera::rotationSpeed() const { return 0.8F; }

float Camera::zoomSpeed() const {
    float d = distance * 0.2F;
    d = std::max(d, 0.0F);
    float speed = d * d;
    speed = std::min(speed, 100.0F); // max speed = 100
    return speed;
}

void Camera::update(const InputData *input) {
    if (!input->keyboard.isKeyDown(GLFW_KEY_LEFT_ALT)) {
        updateView();
        return;
    }

    const glm::vec2 &mouse = input->mouse.pos;
    glm::vec2 delta = (mouse - initialMousePosition) * 0.003F;
    initialMousePosition = mouse;

    if (input->mouse.middle) {
        mousePan(delta);
    } else if (input->mouse.left) {
        mouseRotate(delta);
    } else if (input->mouse.right) {
        mouseZoom(delta.y);
    }

    updateView();
}

void Camera::mousePan(const glm::vec2 &delta) {
    auto speed = panSpeed();
    focalPoint += -GetRightDirection() * delta.x * speed.x * distance;
    focalPoint += GetUpDirection() * delta.y * speed.y * distance;
}

void Camera::mouseRotate(const glm::vec2 &delta) {
    float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
    yaw += yawSign * delta.x * rotationSpeed();
    pitch += delta.y * rotationSpeed();
}

void Camera::mouseZoom(float delta) {
    distance -= delta * zoomSpeed();
    if (distance < 1.0f) {
        focalPoint += GetForwardDirection();
        distance = 1.0f;
    }
}

glm::vec3 Camera::GetUpDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }

glm::vec3 Camera::GetRightDirection() const { return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }

glm::vec3 Camera::GetForwardDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

glm::vec3 Camera::CalculatePosition() const { return focalPoint - GetForwardDirection() * distance; }

glm::quat Camera::GetOrientation() const { return glm::quat(glm::vec3(-pitch, -yaw, 0.0f)); }
