#include "MainMenu.h"

#include <imgui.h>

void MainMenu::render() {
    ImGui::Begin("Main Menu");

    for (unsigned int i = 0; i < scenes.size(); i++) {
        const char *btnName = scenes[i]->getName().c_str();
        if (ImGui::Button(btnName)) {
            *currentSceneIndex = i;
            active = false;
        }
    }

    if (ImGui::Button("Exit")) {
        glfwSetWindowShouldClose(window, 1);
    }

    ImGui::End();
}
