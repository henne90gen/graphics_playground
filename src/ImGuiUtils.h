#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

void initImGui(GLFWwindow *window);

void startImGuiFrame();

void finishImGuiFrame();

void pickColorAndVertices(float *color, float *vertices);
