#include "ImGuiUtils.h"

#include <array>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "examples/imgui_impl_glfw.cpp"
#include "examples/imgui_impl_opengl3.cpp"

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // ImGui::StyleColorsDark();

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

    const float dragSpeed = 0.01F;
    ImGui::DragFloat2("Vertex 1", vertices, dragSpeed);
    ImGui::DragFloat2("Vertex 2", vertices + 2, dragSpeed); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ImGui::DragFloat2("Vertex 3", vertices + 4, dragSpeed); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ImGui::End();
}

void ImGui::NoiseTypeSelector(FastNoise::NoiseType *pType) {
    static const std::array<const char *, 10> items = {"Value", "ValueFractal", "Perlin", "PerlinFractal", "Simplex",
                                                       "SimplexFractal", "Cellular", "WhiteNoise", "Cubic",
                                                       "CubicFractal"};
    ImGui::Combo("Noise Algorithm", (int *) pType, items.data(), items.size());
}
