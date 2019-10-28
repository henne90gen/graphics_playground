#pragma once

#include <FastNoise.h>
#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

#include "util/FileUtils.h"
#include "util/ScreenRecorder.h"
#include "util/TimeUtils.h"

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

void Metrics(const std::shared_ptr<PerformanceCounter> &performanceCounter);

} // namespace ImGui
