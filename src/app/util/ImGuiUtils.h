#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <FastNoise.h>
#include <array>
#include <vector>
#include <string>

#include "util/FileUtils.h"

void initImGui(GLFWwindow *window);

void startImGuiFrame();

void finishImGuiFrame();

void pickColor(float *color);

void pickColor(glm::vec3 &color);

void pickPosition(glm::vec3 &position);

void pickColorAndVertices(float *color, float *vertices);

namespace ImGui {
    void NoiseTypeSelector(FastNoise::NoiseType *pType);

    void ListBox(const std::string &label, unsigned int &currentItem, const std::vector<std::string> &items,
                 const std::string &prefix = "");

    void FileSelector(const std::string &label, const std::string &path, unsigned int &currentItem,
                             std::vector<std::string> &filePaths);
}
