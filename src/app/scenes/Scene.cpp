#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name);
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 200;
    const unsigned int windowHeight = 60;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Back")) {
        backToMainMenu();
    }
    ImGui::End();
}
