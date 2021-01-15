#include "Sky.h"

#include <imgui.h>

#include <util/RenderUtils.h>

DEFINE_DEFAULT_SHADERS(landscape_Sky)

void Sky::init() {
    shader = CREATE_DEFAULT_SHADER(landscape_Sky);
    cubeVA = createCubeVA(shader);
}

void Sky::showGui() {
    ImGui::DragFloat3("Sky Scale", reinterpret_cast<float *>(&skyScale));
    ImGui::ColorEdit3("Sky Color", reinterpret_cast<float *>(&skyColor));
    ImGui::DragFloat("Animation Speed Clouds", &animationSpeed, 0.001F);
    ImGui::DragFloat("Cloud Blend", &cloudBlend, 0.001F);
}

void Sky::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, float animationTime) {
    cubeVA->bind();
    shader->bind();
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, skyScale);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("flatColor", skyColor);
    shader->setUniform("animationTime", animationTime);
    shader->setUniform("animationSpeed", animationSpeed);
    shader->setUniform("cloudBlend", cloudBlend);
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
