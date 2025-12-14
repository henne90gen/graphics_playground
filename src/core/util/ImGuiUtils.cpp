#include "ImGuiUtils.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD 1

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

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

static auto vector_getter = [](void *vec, int idx, const char **out_text) {
    auto &vector = *static_cast<std::vector<std::string> *>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) {
        return false;
    }
    *out_text = vector.at(idx).c_str();
    return true;
};

void ImGui::ListBox(const std::string &label, unsigned int &currentItem, const std::vector<std::string> &items,
                    const std::string &prefix) {
    unsigned long count = items.size();
    std::vector<std::string> fixedItems = {};
    for (unsigned long i = 0; i < count; i++) {
        fixedItems.push_back(items[i].substr(prefix.size()));
    }

    int *current = reinterpret_cast<int *>(&currentItem); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    // NOLINTNEXTLINE(google-readability-casting,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
    ImGui::ListBox(label.c_str(), current, vector_getter, (void *)&fixedItems, count);
}

void ImGui::FileSelector(const std::string &label, const std::string &path, unsigned int &currentItem,
                         std::vector<std::string> &filePaths) {
    getFilesInDirectory(
          path, [](const std::string &fileName) { return true; }, filePaths);
    ImGui::ListBox(label, currentItem, filePaths, path);
}
