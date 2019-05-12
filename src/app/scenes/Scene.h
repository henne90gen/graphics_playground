#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>

class Scene {
public:
    Scene(GLFWwindow *window, std::function<void(void)> &backToMainMenu, const char *name)
            : window(window), name(name), backToMainMenu(backToMainMenu) {};

    virtual ~Scene() {};

    void renderBackMenu() {
        ImGui::Begin(name);
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(100, 60));

        if (ImGui::Button("Back")) {
            backToMainMenu();
        }
        ImGui::End();
    }

    virtual void setup() = 0;

    virtual void tick() = 0;

    virtual void destroy() = 0;

    const char *getName() { return name; }

    void setAspectRatio(const float ratio) { this->aspectRatio = ratio; }

protected:
    GLFWwindow *window;

    inline float getAspectRatio() { return aspectRatio; }

private:
    const char *name;
    float aspectRatio = 16.0f / 9.0f;
    std::function<void(void)> &backToMainMenu;
};
