#include "MainMenu.h"

#include <imgui.h>

void MainMenu::goToScene(unsigned long sceneIndex) {
    *currentSceneIndex = sceneIndex;
    active = false;
    unsigned int width, height;
    glfwGetFramebufferSize(window, reinterpret_cast<int *>(&width), reinterpret_cast<int *>(&height));
    scenes[*currentSceneIndex]->setup(width, height);
}

void MainMenu::render() {
    ImGui::Begin("Main Menu");

    for (unsigned long i = 0; i < scenes.size(); i++) {
        const std::string &btnName = scenes[i]->getName();
        if (ImGui::Button(btnName.c_str())) {
            goToScene(i);
        }
    }

    if (ImGui::Button("Exit")) {
        glfwSetWindowShouldClose(window, 1);
    }

    ImGui::End();
}
