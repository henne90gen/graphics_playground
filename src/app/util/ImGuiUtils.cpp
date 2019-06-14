#include "ImGuiUtils.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "examples/imgui_impl_glfw.cpp"
#include "examples/imgui_impl_opengl3.cpp"

#include <FastNoise.h>

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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::Combo("Noise Algorithm", reinterpret_cast<int *>(pType), items.data(), items.size());
}

void ImGui::ListBox(const std::string &label, unsigned int &currentItem, const std::vector<std::string> &items,
                    const std::string &prefix) {
    unsigned long count = items.size();
    ImGui::ListBoxHeader(label.c_str(), count);
    for (unsigned long i = 0; i < count; i++) {
        auto fileName = items[i].substr(prefix.size());
        if (ImGui::Selectable(fileName.c_str(), i == currentItem)) {
            currentItem = i;
        }
    }
    ImGui::ListBoxFooter();
}

void ImGui::FileSelector(const std::string &label, const std::string &path, unsigned int &currentItem, std::vector<std::string> &filePaths) {
    getFilesInDirectory(path, filePaths);
    ImGui::ListBox(label,currentItem, filePaths, path);
}
