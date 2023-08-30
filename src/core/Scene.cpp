#include "Scene.h"

#include <iostream>

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 10000.0F;

void Scene::renderMetrics() {
    ImGui::Begin("Metrics");
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);

    for (auto &dataPoint : performanceCounter.dataPoints) {
        ImGui::Text("%.3fms - Last Time - %s", dataPoint.second.lastValue, dataPoint.first.c_str());
        ImGui::Text("%.3fms - Average Time - %s", dataPoint.second.average, dataPoint.first.c_str());
        ImGui::Text("%.3fms - Standard Deviation - %s", dataPoint.second.standardDeviation, dataPoint.first.c_str());
    }

    if (ImGui::Button("Reset Performance Counter")) {
        performanceCounter.reset();
    }

    ImGui::End();
}

void Scene::setup(GLFWwindow *w) {
    RECORD_SCOPE();

    window = w;

    camera = Camera(FIELD_OF_VIEW, getAspectRatio(), Z_NEAR, Z_FAR);

    setup();
}

void Scene::internalTick() {
    RECORD_SCOPE();

    auto currentTime = std::chrono::high_resolution_clock::now();
    int64_t currentTimeNs =
          std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime).time_since_epoch().count();
    timeDelta = static_cast<double>(currentTimeNs - lastTimeNs) / 1000000000.0;
    lastTimeNs = currentTimeNs;

    camera.update(input);
    tick();

    renderMetrics();
}

void Scene::onWindowResize(const int w, const int h) {
    width = w;
    height = h;
    glViewport(0, 0, width, height);

    auto widthF = static_cast<float>(width);
    auto heightF = static_cast<float>(height);
    camera.setViewportSize(widthF, heightF);

    aspectRatio = widthF / heightF;
    onAspectRatioChange();
}

void Scene::onKey(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
#ifdef WITH_SCREEN_RECORDER
        auto *recorder = static_cast<ScreenRecorder *>(glfwGetWindowUserPointer(window));
        recorder->takeScreenshot();
#endif
    } else {
        bool isDown = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
        input.keyboard.keys[key] = isDown;
    }
}

void Scene::onCursorPos(double xPos, double yPos) {
    input.mouse.pos.x = xPos;
    input.mouse.pos.y = yPos;
}

void Scene::onMouseButton(int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        input.mouse.left = action == GLFW_PRESS;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        input.mouse.right = action == GLFW_PRESS;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        input.mouse.middle = action == GLFW_PRESS;
    }
}

void Scene::onScroll(double xOffset, double yOffset) { camera.onScroll(yOffset); }
