#include "ImGuiUtils.h"

#include <imgui.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "examples/imgui_impl_glfw.cpp"
#include "examples/imgui_impl_opengl3.cpp"

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void startImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void finishImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void pickColor(float *color) {
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", color);
    ImGui::End();
}

void pickColorAndVertices(float *color, float *vertices) {
    ImGui::Begin("Settings");
    pickColor(color);

    ImGui::DragFloat2("Vertex 1", vertices, 0.01);
    ImGui::DragFloat2("Vertex 2", vertices + 2, 0.01);
    ImGui::DragFloat2("Vertex 3", vertices + 4, 0.01);
    ImGui::End();
}
