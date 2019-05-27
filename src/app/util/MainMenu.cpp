#include "MainMenu.h"

#include <imgui.h>

void MainMenu::goToScene(unsigned long sceneIndex) {
    *currentSceneIndex = sceneIndex;
    active = false;
    scenes[*currentSceneIndex]->setup();
}

void MainMenu::render() {
    ImGui::Begin("Main Menu");

    for (unsigned long i = 0; i < scenes.size(); i++) {
        const char *btnName = scenes[i]->getName();
        if (ImGui::Button(btnName)) {
            goToScene(i);
        }
    }

    if (ImGui::Button("Exit")) {
        glfwSetWindowShouldClose(window, 1);
    }

    ImGui::End();
}
