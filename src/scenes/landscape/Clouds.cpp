#include "Clouds.h"

#include <imgui.h>

#include <util/RenderUtils.h>

DEFINE_DEFAULT_SHADERS(landscape_Clouds)
DEFINE_DEFAULT_SHADERS(landscape_FlatColor)

void Clouds::init() {
    shader = CREATE_DEFAULT_SHADER(landscape_Clouds);
    quadVA = createQuadVA(shader);
}

void Clouds::showGui() {
    ImGui::Checkbox("Clouds Enabled", &cloudsEnabled);
    ImGui::DragFloat3("Clouds Position", reinterpret_cast<float *>(&skyPosition));
    ImGui::DragFloat3("Clouds Rotation", reinterpret_cast<float *>(&skyRotation));
    ImGui::DragFloat3("Clouds Scale", reinterpret_cast<float *>(&skyScale));
    ImGui::ColorEdit3("Clouds Color", reinterpret_cast<float *>(&skyColor));
    ImGui::DragFloat("Animation Speed Clouds", &animationSpeed, 0.001F);
    ImGui::DragFloat("Cloud Blend", &cloudBlend, 0.001F);
}

void Clouds::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime) {
    //    renderSkyBox(projectionMatrix, viewMatrix);
    renderClouds(projectionMatrix, viewMatrix, animationTime);
}

void Clouds::renderClouds(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime) {
    if (!cloudsEnabled) {
        return;
    }

    quadVA->bind();
    shader->bind();
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = createModelMatrix(skyPosition, skyRotation, skyScale);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", normalMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("flatColor", skyColor);
    shader->setUniform("animationTime", animationTime);
    shader->setUniform("animationSpeed", animationSpeed);
    shader->setUniform("cloudBlend", cloudBlend);
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
