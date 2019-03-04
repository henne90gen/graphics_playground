#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include <string>

class Scene {
  public:
    Scene(GLFWwindow *window, std::function<void(void)> &backToMainMenu, std::string name)
        : window(window), backToMainMenu(backToMainMenu), name(name){};
    virtual ~Scene(){};

    void renderBackMenu() {
        ImGui::Begin("Go back");
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(100, 60));

        if (ImGui::Button("Back")) {
            backToMainMenu();
        }
        ImGui::End();
    }

    virtual void tick() = 0;

    std::string getName() { return name; }

  protected:
    GLFWwindow *window;

  private:
    std::string name;
    std::function<void(void)> &backToMainMenu;
};
