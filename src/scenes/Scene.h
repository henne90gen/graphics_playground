#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>

class Scene {
  public:
    Scene(GLFWwindow *window, std::function<void(void)> &backToMainMenu, const char* name)
        : window(window), backToMainMenu(backToMainMenu), name(name){};
    virtual ~Scene(){};

    void renderBackMenu() {
        ImGui::Begin(name);
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(100, 60));

        if (ImGui::Button("Back")) {
            backToMainMenu();
        }
        ImGui::End();
    }

    virtual void tick() = 0;

    const char* getName() { return name; }

  protected:
    GLFWwindow *window;

  private:
    const char* name;
    std::function<void(void)> &backToMainMenu;
};
