#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <FastNoise.h>

void initImGui(GLFWwindow *window);

void startImGuiFrame();

void finishImGuiFrame();

void pickColor(float *color);

void pickColor(glm::vec3 &color);

void pickPosition(glm::vec3 &position);

void pickColorAndVertices(float *color, float *vertices);

namespace ImGui {
    void NoiseTypeSelector(FastNoise::NoiseType *pType);
}
